#ifndef _AOS_VISION_DEBUG_TCP_SERVER_H_
#define _AOS_VISION_DEBUG_TCP_SERVER_H_

#include "aos/vision/events/epoll_events.h"

#include <memory>

namespace aos {
namespace events {

class TcpClient : public ReadEpollEvent {
 public:
  TcpClient(const char* hostname, int portno);
};

}  // namespace events
}  // namespace aos

#endif  // _AOS_VISION_DEBUG_TCP_SERVER_H_
