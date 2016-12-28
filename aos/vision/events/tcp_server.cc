#include "aos/vision/events/tcp_server.h"

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

int TCPServerBase::ConsPort(int portno) {
  int parentfd; /* parent socket */
  struct sockaddr_in serveraddr; /* server's addr */
  int optval; /* flag value for setsockopt */

  /* 
   * socket: create the parent socket 
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) error("ERROR on binding");

  if (listen (parentfd, SOMAXCONN) == -1) error("listen");

  //if (make_socket_non_blocking(parentfd) == -1) error("make_socket_non_blocking");

  printf("connected to port: %d on fd: %d\n", portno, parentfd);
  return parentfd; 
}

TCPServerBase::~TCPServerBase() { close(fd()); }

void TCPServerBase::ReadEvent(Context ctx) {
  int s;
  /* We have a notification on the listening socket, which
   means one or more incoming connections. */
  struct sockaddr in_addr;
  socklen_t in_len;
  int infd;
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

  in_len = sizeof in_addr;
  infd = accept(fd(), &in_addr, &in_len);
  if (infd == -1) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
      /* We have processed all incoming
         connections. */
      return;
    } else {
      perror ("accept");
      return;
    }
  }

  s = getnameinfo (&in_addr, in_len,
                   hbuf, sizeof hbuf,
                   sbuf, sizeof sbuf,
                   NI_NUMERICHOST | NI_NUMERICSERV);
  if (s == 0) {
    printf("Accepted connection on descriptor %d "
           "(host=%s, port=%s)\n", infd, hbuf, sbuf);
  }

  /* Make the incoming socket non-blocking and add it to the
     list of fds to monitor. */
  s = make_socket_non_blocking (infd);
  if (s == -1) abort ();

  s = ctx.loop->Add(Construct(infd));
  if (s == -1) {
    perror ("epoll_ctl");
    abort ();
  }
}

}  // namespace events
}  // namespace aos
