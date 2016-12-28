#include "aos/vision/events/tcp_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace aos {
namespace events {

namespace {
inline void error(const char *msg) {
  perror(msg);
  exit(1);
}

int make_socket_non_blocking (int sfd) {
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1) {
    perror ("fcntl");
    return -1;
  }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1) {
    perror ("fcntl");
    return -1;
  }

  return 0;
}
}  // namespace

namespace {
int open_client(const char* hostname, int portno) {
  int sockfd;
  struct sockaddr_in serveraddr;
  struct hostent *server;
  /* socket: create the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { 
    error("ERROR opening socket");
	}

  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host as %s\n", hostname);
    exit(0);
  }

  /* build the server's Internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	  		(char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portno);

  /* connect: create a connection with the server */
  if (connect(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    error("ERROR connecting");
	}
  if (make_socket_non_blocking(sockfd) == -1) error("make_socket_non_blocking");
	return sockfd;
}
}  // namespace

TcpClient::TcpClient(const char* hostname, int portno)
  : ReadEpollEvent(open_client(hostname, portno)) {} 

}  // namespace events
}  // namespace aos
