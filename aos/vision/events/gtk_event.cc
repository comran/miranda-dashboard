#include "aos/vision/events/gtk_event.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <poll.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#include <map>

using namespace aos::events;
using namespace aos::vision;

namespace aos {
namespace vision {

static GMainLoop* main_loop;

namespace {

/*
std::vector<struct pollfd> get_fds() {
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
  return fds;
}

void handleEvents(std::vector<struct pollfd>* fds) {
  g_main_context_check(context, max_priority,
    reinterpret_cast<GPollFD*>(fds->data()), fds->size());

  g_main_context_dispatch (context);

  g_main_context_release(context);

}
*/


class AdaptGtkMainLoop {
 public:
  class GtkSocketEventHandler : public events::EpollEvent {
   public:
    GtkSocketEventHandler(int fd, AdaptGtkMainLoop* loop) : events::EpollEvent(fd), loop_(loop) {}

    void Event(Context /*ctx*/) override {
      loop_->Tick();
    }

    AdaptGtkMainLoop* loop_;
   private:
  };
  class Timeout : public events::EpollTimeout {
   public:
    Timeout(AdaptGtkMainLoop* loop) : loop_(loop) {}
    void Tick() {
      loop_->TimeoutTick();
    }
    void Hit() {
      //loop_->TimeoutTick();
    }
    AdaptGtkMainLoop* loop_;
  };
  Timeout epoll_timeout;
  AdaptGtkMainLoop(events::EpollLoop* loop) : epoll_timeout(this), loop_(loop) {
    Populate_fds();
    while (timeout == 0) {
      handle_fds();
      Populate_fds();
    }
    refresh_events();
    loop_->AddTimeout(&epoll_timeout);
    epoll_timeout.SetTimeout(timeout);
  }

  void Populate_fds() {
    context_ = g_main_loop_get_context(main_loop);
    if (g_main_context_acquire(context_) != TRUE) {
      fprintf(stderr, "incompatable main lib usage\n");
      exit(-1);
    }

    g_main_context_prepare(context_, &max_priority_); 

    gint nfds, alloc_size;

    do {
      if (fds_.size() == 0) {
        fds_.resize(10);
      }
      alloc_size = fds_.size();
      nfds = g_main_context_query(context_, max_priority_, &timeout, 
                                  reinterpret_cast<GPollFD*>(fds_.data()), alloc_size);
      fds_.resize(nfds);
    } while (nfds > alloc_size);
//    printf("found events: %d timeout %d\n", (int)fds_.size(), timeout);
    //for (auto& fd_i : fds_) {
   //   printf("fd: %d events: %d\n", fd_i.fd, fd_i.events);
    //}
  }

  void TimeoutTick() {
    timeout = 0;
    while (timeout == 0) {
      poll(fds_.data(), fds_.size(), timeout);
      handle_fds();
      Populate_fds();
    }
    epoll_timeout.SetTimeout(timeout);
  }

  void Tick() {
    timeout = 0;
    while (timeout == 0) {
//      printf("stuck in poll: %d\n", timeout);
      poll(fds_.data(), fds_.size(), timeout);
//printf("- exit poll");
      handle_fds();
      Populate_fds();
    }
    epoll_timeout.SetTimeout(timeout);
  }

  void handle_fds() { 
    g_main_context_check(context_, max_priority_,
                         reinterpret_cast<GPollFD*>(fds_.data()), fds_.size());

    g_main_context_dispatch(context_);

    g_main_context_release(context_);
  }

  void refresh_events() {
    for (auto& fd_i : fds_) {
      if (!eventsh_[fd_i.fd]) {
        if (fd_i.events != 1) {
          printf("error only support read event integration\n");
          exit(-1);
        }
        RegisterEvent(fd_i.fd);
      }
    }
  }

  void RegisterEvent(int fd) {
    auto sock_handle = new GtkSocketEventHandler(fd, this);
    loop_->Add(sock_handle);
    eventsh_[fd].reset(sock_handle);
  }

  gint max_priority_;
  gint timeout;

  GMainContext* context_;
  std::vector<struct pollfd> fds_;
  std::map<int, std::unique_ptr<GtkSocketEventHandler>> eventsh_;
  events::EpollLoop* loop_;
};

}  // namespace

AdaptGtkMainLoop* adaptor;

void add_gtk_main(events::EpollLoop* loop, int* argc, char **argv[]) {
  gtk_init(argc, argv); //NULL);

  (void)loop;
  main_loop = g_main_loop_new(NULL, TRUE);
  adaptor = new AdaptGtkMainLoop(loop);
}

}  // namespace vision
}  // namespace aos

