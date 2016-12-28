#ifndef _AOS_VISION_DEBUG_EVENTS_H_
#define _AOS_VISION_DEBUG_EVENTS_H_

#include <stdint.h>

#include <memory>

namespace aos {
namespace events {

class EpollEvent;
// use timeout sparingly.
class EpollTimeout;

class EpollLoop {
 public:
  class Impl;
  EpollLoop();
  ~EpollLoop();
  void AddTimeout(EpollTimeout *timeout);
  int Add(EpollEvent* evt);
  int Stop(EpollEvent* evt);
  void Run();
 private:
  std::unique_ptr<Impl> impl_;
};

class EpollTimeout {
 public:
  virtual ~EpollTimeout(){}
  // Every Epoll event.
  virtual void Tick() {}
  // Epoll Timeout event.
  virtual void Hit() {}
  void SetTimeout(int t) {
    timeout = t;
    start_time = -1;
  }
  int GetTimeout(int64_t cur_time) {
    if (timeout < 0) return timeout;
    if (start_time < 0) start_time = cur_time;
    int64_t timeout_adj = (int64_t)timeout - (cur_time - start_time);
    if (timeout_adj < 0) return 0;
    return (int)timeout_adj;
  }
 private:
  int64_t start_time; // for bookkeeping
  int timeout;
};

class EpollEvent {
 public:
  struct Context {
    EpollLoop* loop;
    uint32_t events;
  };
  EpollEvent(int fd) : fd_(fd) {}
  virtual ~EpollEvent() {}
  virtual void ReadEvent(Context) {}
  virtual void Event(Context ctx);
  int fd() { return fd_; }
 private:
  int fd_;
};

class ReadEpollEvent : public EpollEvent {
 public:
  class ReadContext {
   public:
    ssize_t Read(void *buf, size_t count);
   private:
    friend class ReadEpollEvent;
    int fd_;
    bool has_error_ = false;
  };
  ReadEpollEvent(int fd) : EpollEvent(fd) {}
  void ReadEvent(Context ctx) override;
  virtual void ReadEvent(ReadContext* ctx) = 0;
};

// Hey! Maybe you want a doubly linked list that frees things for you!
template <class T>
class intrusive_free_list {
 public:
  class elem {
   public:
    elem(intrusive_free_list<T>* list, T* t) : list_(list), prev_(nullptr) {
      next_ = list_->begin_;
      if (next_) next_->prev_ = t;
      list_->begin_ = t;
    }
    ~elem() {
      if (next_) next_->prev_ = prev_;
      ((prev_) ? prev_->next_ : list_->begin_) = next_;
    }
    T* next() { return next_; }
   private:
    friend class intrusive_free_list<T>;
    intrusive_free_list<T>* list_;
    T* next_;
    T* prev_;
  };
  intrusive_free_list() : begin_(nullptr) {}
  ~intrusive_free_list() {
    while (begin_) delete begin_;
  }
  T* begin() { return begin_; }
 private:
  friend class elem;
  T* begin_;
};

}  // namespace events
}  // namespace aos

#endif  // _AOS_VISION_DEBUG_EVENTS_H_
