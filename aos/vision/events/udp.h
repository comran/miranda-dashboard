#ifndef _AOS_VISION_IMAGE_UDP_H_
#define _AOS_VISION_IMAGE_UDP_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>
#include <unistd.h>
#include <vector>

namespace aos {
namespace vision {

class TXUdpSocket {
 public:
  TXUdpSocket() : s_(-1) {}
  TXUdpSocket(const char* ip_addr, int port);
  ~TXUdpSocket();

  void Connect(const char* ip_addr, int port);

  int Send(const char* data, int size);
 private:
  struct sockaddr_in si_other_;
  int s_;
};

class RXUdpSocket {
 public:
  RXUdpSocket() : s_(-1) {}
  RXUdpSocket(int port);
  ~RXUdpSocket();

  void Bind(int port);

  int Recv(char* data, int size);

  int get_fd() { return s_; }
 private:
  int s_;
};

}  // vision
}  // aos

#endif  // _AOS_VISION_IMAGE_UDP_H_
