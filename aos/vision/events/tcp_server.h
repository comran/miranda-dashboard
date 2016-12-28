#ifndef _AOS_VISION_EVENTS_TCP_SERVER_H_
#define _AOS_VISION_EVENTS_TCP_SERVER_H_

#include "aos/vision/events/epoll_events.h"

#include <memory>
#include <vector>

namespace aos {
namespace events {

template <class T>
class TCPServer;
class SocketConn;
class TCPServerBase : public EpollEvent {
 public:
  TCPServerBase(int fd) : EpollEvent(fd) {}
  ~TCPServerBase();
 protected:
  static int ConsPort(int id);
 private:
  virtual SocketConn* Construct(int child_fd) = 0;
  void ReadEvent(Context ctx) override;
  friend class SocketConn;
  template <class T>
  friend class TCPServer;
  intrusive_free_list<SocketConn> free_list;
};

class SocketConn : public ReadEpollEvent, public intrusive_free_list<SocketConn>::elem {
 public:
  SocketConn(TCPServerBase* server, int fd) : ReadEpollEvent(fd),
  intrusive_free_list<SocketConn>::elem(&server->free_list, this) {}
};

template <class T>
class TCPServer : public TCPServerBase {
 public:
  TCPServer(int port) : TCPServerBase(ConsPort(port)) {}
  SocketConn* Construct(int child_fd) override { return new T(this, child_fd); }

  static std::unique_ptr<TCPServer<T>> Make(int port) {
    return std::unique_ptr<TCPServer<T>>(new TCPServer<T>(port));
  }

  template <typename EachBlock>
  void broadcast(EachBlock blk) {
    auto a = free_list.begin();
    while (a) {
      auto client = static_cast<T*>(a);
      if (client) blk(client);
      else printf("error downcasting!\n");
      a = a->next();
    }
  }
};

}  // namespace events
}  // namespace aos

#endif  // _AOS_VISION_EVENTS_TCP_SERVER_H_
