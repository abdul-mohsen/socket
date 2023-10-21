#include "util.h"
#include "../dataStructure/Array.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int get_listener_socket(void) {
  int listener, yes = 1, rv;
  struct addrinfo hints, *ai, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;


  if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return -1;
  }

  for (p = ai; p!=NULL; p = p->ai_next) {

    if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      return -1;
    }

    if ((bind(listener, p->ai_addr, p->ai_addrlen) == -1)) {
      close(listener);
      perror("server: bind");
      continue;
    }
    break;
  }

  freeaddrinfo(ai);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return -1;
  }

  if (listen(listener, BACKLOG) == -1) {
    perror("listen");
    return -1;
  }

  return listener;
}

void add_to_pdfs(Array *pfds, int newfd) {
  struct pollfd pfd = {0};
  pfd.events = POLLIN;
  pfd.fd = newfd;
  addItem(pfds, &pfd);
}

int main(int argc, char *argv[]) {
  int listener, new_fd, fd_count = 0, fd_size = 5;
  char buf[256], remoteIP[INET6_ADDRSTRLEN];
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  Array *pfds = initArray(sizeof(struct pollfd));

  listener = get_listener_socket();

  if (listener == -1) {
    fprintf(stderr, "error getting listening socket\n");
    exit(1);
  }

  add_to_pdfs(pfds, listener);
  
  while (1) {
    int poll_count = poll(pfds->data, pfds->size, -1);

    if (poll_count == -1) {
      perror("poll");
      exit(1);
    }

    for (int i = 0; i < poll_count; i++) {
      struct pollfd *pfd = ((struct pollfd **) pfds->data)[i];

      if (!(pfd->revents & POLLIN)) {
        continue;
      }

      if (pfd->fd == listener) {
        addrlen = sizeof(remoteaddr);
        new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
        if (new_fd == -1) {
          perror("accept");
          continue;
        }

        add_to_pdfs(pfds, new_fd);
        void *address =  get_in_addr((struct sockaddr*)&remoteaddr);
        const char *con = inet_ntop(remoteaddr.ss_family, address, remoteIP, INET6_ADDRSTRLEN);
        printf("pollserver: new connection from %s on socket %d\n", con, new_fd);
      } else {
        int nbytes = recv(pfd->fd, buf, sizeof(buf), 0);
        int sender_fd = pfd->fd;

        if (nbytes == 0) {
          printf("pollserver: socket %d hung up\n", sender_fd);
        } else if (nbytes < 0) {
          perror("recv");
        } else {

          for (int j = 0; j < fd_count; j++) {
            struct pollfd *pfdj = ((struct pollfd **) pfds->data)[j];
            int dest_fd = pfdj->fd;
            if (dest_fd != listener && dest_fd != sender_fd) {
              if (send(dest_fd, buf, nbytes, 0) == -1) {
                perror("send");
              }
            }
          }
        }

        if (nbytes <= 0) {
          close(pfd->fd);
          deleteItem(pfds, i);
        }
      }
    }
  }

  return EXIT_SUCCESS;
}
