#include <sys/socket.h>

#define PORT "8000"
#define BACKLOG 10
#define MAXDATASIZE 100

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
