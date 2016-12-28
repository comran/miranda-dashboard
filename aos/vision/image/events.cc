#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <poll.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#include "aos/vision/image/events.h"

namespace aos {
namespace vision {

int64_t GetTimeStamp() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(int64_t)1000 + tv.tv_usec / 1000;
}

static GMainLoop* main_loop;

void main_loop_quit() {
  g_main_loop_quit(main_loop);
}

class EventList {
 public:
  std::vector<struct pollfd>* vec;
  gint* timeout;
};

class EventLoop {
 public:
  std::vector<LoopEvent*> events;
  EventLoop() {
    main_loop = g_main_loop_new(NULL, TRUE);
  }
  ~EventLoop() {
    g_main_loop_unref(main_loop);
    main_loop = NULL;
  }
  void run() {
    while (g_main_loop_is_running(main_loop)) {
      run_timeout(1000000);
    }
  }
  void run_timeout(int ms) {
    int ms_left = ms;
    int64_t start_t = GetTimeStamp();

    while (g_main_loop_is_running(main_loop) && ms_left > 0) {
      auto* context = g_main_loop_get_context(main_loop);
      if (g_main_context_acquire(context) != TRUE) {
        fprintf(stderr, "incompatable main lib usage\n");
        exit(-1);
      }

      gint max_priority;
      g_main_context_prepare (context, &max_priority); 

      gint nfds, alloc_size;
      gint timeout;
      std::vector<struct pollfd> fds(10);

      do {
        alloc_size = fds.size();
        nfds = g_main_context_query(context, max_priority, &timeout, 
                                    reinterpret_cast<GPollFD*>(fds.data()), alloc_size);
        fds.resize(nfds);
      } while (nfds > alloc_size);

      sorted_insert(&fds, &timeout);

      if (ms_left < timeout || timeout == -1) timeout = ms_left;
//      printf("will timeout: %d\n", timeout);
      poll(fds.data(), fds.size(), timeout);
      handle_events(&fds);

      g_main_context_check(context, max_priority, reinterpret_cast<GPollFD*>(fds.data()), fds.size());

      g_main_context_dispatch (context);

      g_main_context_release(context);
    
      ms_left = ms - (GetTimeStamp() - start_t);
 //     printf("going back for %d\n", ms_left);
    }
  }
 private:
  void sorted_insert(std::vector<struct pollfd>* vec, gint* timeout) {
    EventList ev_list;
    ev_list.vec = vec;
    ev_list.timeout = timeout;
    for (auto event : events) {
      event->Report(&ev_list);
    }
  }
  void handle_events(std::vector<struct pollfd>* vec) {
    EventList ev_list;
    ev_list.vec = vec;
    for (auto event : events) {
      event->Handle(&ev_list);
    }
  }
};

void ReadEvent::Report(EventList* ev) {
  struct pollfd record;
  record.fd = fd;
  record.events = POLLIN;
  record.revents = 0;

  auto iter = ev->vec->begin();
  while (iter < ev->vec->end() && iter->fd < record.fd) {
    ++iter;
  }
  ev->vec->insert(iter, record);
}

void ReadEvent::Handle(EventList* ev) {
  for (auto iter = ev->vec->begin(); iter < ev->vec->end(); ++iter) {
    if (iter->fd == fd) {
      if (iter->revents != 0) {
        cb();
      }
      ev->vec->erase(iter);
    }
  }
}

void TimeoutEvent::Report(EventList* ev) {
  int64_t ms_left = timeout - (GetTimeStamp() - start_t);
  while (ms_left <= 0) {
    start_t = GetTimeStamp();
    ms_left = timeout;
    cb();
  }
  if (ms_left < *ev->timeout || *ev->timeout == -1) *ev->timeout = ms_left;
}

void TimeoutEvent::Handle(EventList* /*ev*/) {
  int64_t ms_left = timeout - (GetTimeStamp() - start_t);
  if (ms_left <= 0) {
    start_t = GetTimeStamp();
    cb();
  }
}


void run_main(std::vector<LoopEvent*> ev) {
  EventLoop evnt;
  evnt.events = ev;
  evnt.run();
}

void run_main_timeout(int ms) {
  EventLoop evnt;
  evnt.run_timeout(ms);
}

}  // vision
}  // aos
