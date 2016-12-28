#ifndef _AOS_VISION_EVENTS_GTK_EVENT_H_
#define _AOS_VISION_EVENTS_GTK_EVENT_H_

#include "aos/vision/events/epoll_events.h"

namespace aos {
namespace vision {

// void main_loop_quit();

void add_gtk_main(events::EpollLoop* loop, int* argc, char **argv[]);

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_DEBUG_GTK_EVENT_H_
