#ifndef AOS_VISION_IMAGE_READER_H_
#define AOS_VISION_IMAGE_READER_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <string>
#include <inttypes.h>
#include <functional>

#include "aos/vision/image/V4L2.h"
#include "aos/vision/image/image_types.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

namespace camera {

struct CameraParams {
  int32_t width;
  int32_t height;
  int32_t exposure;
  int32_t brightness;
  int32_t gain;
  int32_t fps;
};

typedef std::function<void(void *buf, size_t len, uint64_t timestamp)> process_cb;
typedef std::function<void(aos::vision::DataRef data, uint64_t timestamp)> process_cb_2;

class Reader {
 public:
  // deprecated
  Reader(const char *dev_name, process_cb process, CameraParams params);
  Reader(const char *dev_name, process_cb_2 process, CameraParams params)
      : Reader(dev_name,
               [process](void *buf, size_t len, uint64_t timestamp) {
                 process(aos::vision::DataRef{(const char *)buf, (int)len},
                         timestamp);
               },
               params) {}
  aos::vision::ImageFormat get_format();
 private:
  void QueueBuffer(v4l2_buffer *buf);
  void ReadFrame();
  void init_mmap();
  bool SetCameraControl(uint32_t id, const char *name, int value);
  void Init();
  void Start();
  void mmap_buffs();
 public:
  void Run();
  void handleFrame() {
    ReadFrame();
  }
  void startAsync() {
    Start();
    mmap_buffs();
  }
  int getFd() {
    return fd_;
  }
#if 0
  // if we ever do want to do any of these things, this is how
  void Stop() {
    const v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd_, VIDIOC_STREAMOFF, &type) == -1) {
      errno_exit("VIDIOC_STREAMOFF");
    }
  }
  void Close() {
    if (close(fd_) == -1)
      errno_exit("close");
    fd_ = -1;
  }
#endif
  // of the camera
  int fd_;
  // the bound socket listening for fd requests
  const char *dev_name_;

  process_cb process_;

  // the number of buffers currently queued in v4l2
  uint32_t queued_;
  struct Buffer;
  Buffer *buffers_;

  static const unsigned int kNumBuffers = 10;

  // set only at initialize
  CameraParams params_;

};

} // namespace camera

#endif  // AOS_VISION_IMAGE_READER_H_
