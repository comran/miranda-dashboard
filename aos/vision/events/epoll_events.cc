#include "aos/vision/events/epoll_events.h"

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

#include <vector>
#include <memory>

#define MAXEVENTS 64

namespace aos {
namespace events {

namespace {
#include <sys/time.h>
int64_t GetTimeStamp() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(int64_t)1000 + tv.tv_usec / 1000;
}
}  // namespace

class EpollLoop::Impl {
 public:
  Impl() {
    efd_ = epoll_create1(0);
    if (efd_ == -1) {
      perror ("epoll_create");
      abort ();
    }

    /* Buffer where events are returned */
    epoll_events_.reset(new struct epoll_event[MAXEVENTS]);
  }
  int Add(EpollEvent* evt) {
    struct epoll_event event;
    event.data.ptr = (void*)evt;
    event.events = EPOLLIN; // | EPOLLET;
    return epoll_ctl(efd_, EPOLL_CTL_ADD, evt->fd(), &event);
  }
  int Stop(EpollEvent* evt) {
    return epoll_ctl(efd_, EPOLL_CTL_DEL, evt->fd(), NULL);
  }
  void Run(EpollLoop* loop) {
    while (1) {
      int n, i;

      int timeout = CalcTimeout();
//printf("epoll timeout: %d\n", timeout);
      n = epoll_wait(efd_, epoll_events_.get(), MAXEVENTS, timeout);
      //printf("epoll done: %d\n", n);
      HandleTimeouts();

      for (i = 0; i < n; i++) {
        EpollEvent* evt = static_cast<EpollEvent*>(epoll_events_[i].data.ptr);
        EpollEvent::Context ctx = {loop, epoll_events_[i].events};
        evt->Event(ctx);
      }
    }
  }
  void HandleTimeouts() {
    int64_t cur_time = GetTimeStamp();
    for (EpollTimeout* timeout : timeouts_) {
      timeout->Tick();
      int new_timeout = timeout->GetTimeout(cur_time);
      if (new_timeout == 0) {
        timeout->Hit();
      }
    }
  }
  int CalcTimeout() {
    int64_t cur_time = GetTimeStamp();
    int timeout_val = -1;
    for (EpollTimeout* timeout : timeouts_) {
      int new_timeout = timeout->GetTimeout(cur_time);
      if (new_timeout >= 0) {
        if (timeout_val < 0) {
          timeout_val = new_timeout;
        } else if (new_timeout < timeout_val) {
          timeout_val = new_timeout;
        }
      }
    }
    return timeout_val;
  }
  void AddTimeout(EpollTimeout *timeout) {
    timeouts_.push_back(timeout);
  }
  int efd() { return efd_; }
 private:
  std::unique_ptr<struct epoll_event[]> epoll_events_;
  int efd_;
  std::vector<EpollTimeout*> timeouts_;
};

EpollLoop::EpollLoop() : impl_(new Impl()) {}
EpollLoop::~EpollLoop() {}
void EpollLoop::Run() { impl_->Run(this); }
int EpollLoop::Add(EpollEvent* evt) { return impl_->Add(evt); }
int EpollLoop::Stop(EpollEvent* evt) { return impl_->Stop(evt); }
void EpollLoop::AddTimeout(EpollTimeout *timeout) { impl_->AddTimeout(timeout); }

void EpollEvent::Event(Context ctx) {
  if ((ctx.events & EPOLLERR) || (ctx.events & EPOLLHUP) || (!(ctx.events & EPOLLIN))) {
    fprintf (stderr, "epoll error\n");
    close (fd_);
  } else { 
    ReadEvent(ctx);
  }
}

ssize_t ReadEpollEvent::ReadContext::Read(void *buf, size_t len) {
  ssize_t count;
  count = read(fd_, buf, len);
  if (count > 0) return count;
  if (count == -1) {
    if (errno == EAGAIN) return count;
    perror ("read");
  }
  has_error_ = true;
  return count;
}

void ReadEpollEvent::ReadEvent(Context ctx) {
  ReadContext r_ctx;
  r_ctx.fd_ = fd();
  ReadEvent(&r_ctx);
  if (r_ctx.has_error_) {
    ctx.loop->Stop(this);
    close(fd());
    delete this; return;
    return;
  }
}

}  // namespace events
}  // namespace aos
