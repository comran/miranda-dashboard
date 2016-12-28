#ifndef _AOS_VISION_IMAGE_IMAGE_STREAM_H_
#define _AOS_VISION_IMAGE_IMAGE_STREAM_H_

#include "aos/vision/image/reader.h"
#include "aos/vision/events/epoll_events.h"

#include <memory>

namespace aos {
namespace vision {

class ImageStreamEvent : public ::aos::events::EpollEvent {
 public:
  static ::camera::Reader *get_camera(const std::string &fname,
                                      ImageStreamEvent *obj, camera::CameraParams params) {
    using namespace std::placeholders;
    auto camread = new ::camera::Reader(
        fname.c_str(),
        std::bind(&ImageStreamEvent::process_helper, obj, _1, _2), params);
    camread->startAsync();
    return camread;
  }

  explicit ImageStreamEvent(::camera::Reader *reader)
      : ::aos::events::EpollEvent(reader->getFd()), reader_(reader) {
    auto fmt = reader->get_format();
    outbuf.reset(new PixelRef[fmt.img_size()]);
    ptr = ImagePtr{fmt, outbuf.get()};
  }

  explicit ImageStreamEvent(const std::string &fname, camera::CameraParams params)
      : ImageStreamEvent(
            get_camera(fname, this, params)) {}

  void process_helper(DataRef data, uint64_t timestamp) {
    if (data.size() < 300) {
      printf("got bad img: %d of size(%d)\n", (int)timestamp, data.size());
      return;
    }
    ProcessImage(data, timestamp);
  }
  virtual void ProcessImage(DataRef data, uint64_t timestamp) = 0;

  void ReadEvent(Context /*ctx*/) override { reader_->handleFrame(); }

 private:
  std::unique_ptr<::camera::Reader> reader_;
  std::unique_ptr<PixelRef[]> outbuf;
  ImagePtr ptr;
};

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_DEBUG_IMAGE_STREAM_H_
