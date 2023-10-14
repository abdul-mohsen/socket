#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
  int status;
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
  
  
  freeaddrinfo(res);
  return 0;
}
