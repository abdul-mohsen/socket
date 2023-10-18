#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
  int status;
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hint, *res, *p;
  char ipstr[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr,"usage: showip hostname\n");
    return 1;
  }

  memset(&hint, 0, sizeof(struct addrinfo));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(argv[1],NULL , &hint, &res))) {
    fprintf(stderr,"getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }

  printf("IP address for %s\n\n", argv[1]);

  for (p = res; p != NULL; p = p->ai_next) {
    void *addr;
    char *ipver;

    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct  sockaddr_in *) p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPV4";
    } else {
      struct sockaddr_in *ipv6 = (struct  sockaddr_in *) p->ai_addr;
      addr = &(ipv6->sin_addr);
      ipver = "IPV6";
    }

    inet_ntop(p->ai_family, addr, ipstr, INET6_ADDRSTRLEN);
    printf(" %s: %s\n", ipver, ipstr);
  
  }

  int sockfd, new_fd;
  
  if (!(sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))) {
    fprintf(stderr,"socket error: ");
    return 1;
  };
  


  if ((bind(sockfd, res->ai_addr, res->ai_addrlen))) {
    fprintf(stderr,"bind error: ");
    return 1;
  }
  /* if ((connect(sockfd, res->ai_addr, res->ai_addrlen))) {
    fprintf(stderr,"connect error: ");
    return 1;
  } */
  
  if (listen(sockfd, BACKLOG)) {
    fprintf(stderr,"listen error: ");
    return 1;
  }

  addr_size = sizeof(their_addr);

  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

  if (new_fd == -1) {
    fprintf(stderr,"accept error: ");
    return 1;
  }

  char *msg = "ssda po pa";
  int len,bytes_sent;

  len = strlen(msg);
  bytes_sent = send(new_fd, msg, len, 0);
  
  freeaddrinfo(res);
  return 0;
}
