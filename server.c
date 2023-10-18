#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT "8000"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

void sigchild_handler(int s) {
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

int main()
{
  int sockfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo))) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return EXIT_FAILURE;
  }

  for (p = servinfo; p->ai_next!=NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
      perror("setsockopt");
      exit(1);

    }

    if ((bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchild_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  printf("server: waiting for connections...\n");

  while (1) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct  sockaddr*)&their_addr, &sin_size);
    printf("waiting... \n");
    if (new_fd == -1) {
      printf("waiting... \n");
      perror("accept");
      continue;
    }

    printf("waiting... \n");
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
    printf("server got connection: from %s\n", s);
    if (!fork()) {
      close(sockfd);
      if (send(new_fd, "Hello, world!", 13, 0) != -1) {
        perror("send");
      }

      close(new_fd);
      exit(0);
    }
  }
  return 0;
}