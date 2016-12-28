#include "aos/vision/events/udp.h"

#include <string.h>

namespace aos {
namespace vision {

TXUdpSocket::TXUdpSocket(const char* ip_addr, int port) {
  Connect(ip_addr, port);
}

TXUdpSocket::~TXUdpSocket() {
  if (s_ >= 0) close(s_);
  s_ = -1;
}

void TXUdpSocket::Connect(const char* ip_addr, int port) {
  s_ = -1;
  s_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s_ == -1) {
    return;
  }

  struct sockaddr_in si_me;
  memset((char *) &si_me, 0, sizeof(si_me));

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(0);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  //bind socket to port
  
  struct sockaddr arr_out;
  memcpy(&arr_out, &si_me, sizeof(si_me));
  if( bind(s_, &arr_out, sizeof(si_me) ) == -1) {
    close(s_);
    s_ = -1;
    return;
  }

  si_other_.sin_family = AF_INET;
  si_other_.sin_port = htons(port);
  inet_aton(ip_addr, &si_other_.sin_addr);
}

int TXUdpSocket::Send(const char* data, int size) {
  struct sockaddr arr_out;
  memcpy(&arr_out, &si_other_, sizeof(si_other_));
  return sendto(s_, data, size, 0, &arr_out, sizeof(si_other_));
}

RXUdpSocket::RXUdpSocket(int port) {
  Bind(port);
}
  
RXUdpSocket::~RXUdpSocket() {
  if (s_ >= 0) close(s_);
  s_ = -1;
}

void RXUdpSocket::Bind(int port) {
  s_ = -1;
  s_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s_ == -1) {
    return;
  }

  struct sockaddr_in si_me;
  memset((char *) &si_me, 0, sizeof(si_me));

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(port);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  //bind socket to port
  
  struct sockaddr arr_out;
  memcpy(&arr_out, &si_me, sizeof(si_me));
  if( bind(s_, &arr_out, sizeof(si_me) ) == -1) {
    close(s_);
    s_ = -1;
    return;
  }
}

int RXUdpSocket::Recv(char* data, int size) {
  struct sockaddr addr;
  socklen_t fromlen = sizeof(addr);
  return recvfrom(s_, data, size, 0, &addr, &fromlen);
}

}  // vision
}  // aos

