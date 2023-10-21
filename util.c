#include "util.h"
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

void sigchld_handler(int s) {
  printf("stuck here %i", s);
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
