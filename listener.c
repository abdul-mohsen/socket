#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT "8000"
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
  int sockfd, numbytes, rv;
  char buf[MAXDATASIZE], s[INET6_ADDRSTRLEN];
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t addr_len;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return EXIT_FAILURE;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  printf("client: waiting for connections...\n");

  addr_len = sizeof their_addr;
  if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
    perror("recvfrom");
    exit(1);
  }

  void * ip = get_in_addr((struct sockaddr *)&their_addr);
  const char *pocket = inet_ntop(their_addr.ss_family, ip,s, sizeof s);
  printf("listener: got packet from %s\n",pocket);
  printf("listener: packet is %d bytes long\n", numbytes);
  buf[numbytes] = '\0';
  printf("listener: packet contains \"%s\"\n", buf);

  close(sockfd);

  return EXIT_SUCCESS;
}
