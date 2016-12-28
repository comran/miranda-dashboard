#ifndef AOS_VISION_IMAGE_EVENTS_H_
#define AOS_VISION_IMAGE_EVENTS_H_

#include <functional>
#include <vector>
#include <stdint.h>

namespace aos {
namespace vision {

int64_t GetTimeStamp();

class EventList;

class LoopEvent {
 public:
  LoopEvent(std::function<void()> cb) : cb(cb) {}
  LoopEvent() {}
  virtual ~LoopEvent() {}
  
  std::function<void()> cb;

 protected:
  virtual void Report(EventList*) = 0;
  virtual void Handle(EventList*) = 0;
  friend class EventLoop;
};

class ReadEvent : public LoopEvent {
 public:
  ReadEvent(int fd, std::function<void()> cb) : LoopEvent(cb), fd(fd) {}
  explicit ReadEvent(int fd) : fd(fd) {}
  int fd;

 protected:
  void Report(EventList*) override;
  void Handle(EventList*) override;
};

class TimeoutEvent : public LoopEvent {
 public:
  TimeoutEvent(int timeout, std::function<void()> cb) : LoopEvent(cb), timeout(timeout) {}
  explicit TimeoutEvent(int timeout) : timeout(timeout) {}
  int timeout;

  //GetTimeStamp();
 protected:
  int64_t start_t = 0;
  void Report(EventList*) override;
  void Handle(EventList*) override;
};

void run_main(std::vector<LoopEvent*> ev);
void run_main_no_gtk(std::vector<LoopEvent*> ev);
void run_main_timeout(int ms);

void main_loop_quit();

}  // vision
}  // aos

#endif   // AOS_VISION_IMAGE_EVENTS_H_
