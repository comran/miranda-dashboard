#ifndef _AOS_VISION_IMAGE_IMAGE_STREAM_LEGACY_H_
#define _AOS_VISION_IMAGE_IMAGE_STREAM_LEGACY_H_

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "aos/vision/image/image_types.h"
#include "aos/vision/debug/overlay.h"
#include "aos/vision/comp_geo/segment.h"
#include "aos/vision/image/events.h"
#include "aos/vision/image/file_stream.h"

namespace aos {
namespace vision {

class ImageReader;
class ImageStream;

class ImageStreamConfig {
 public:
  ImageStreamConfig() : ImageStreamConfig(640, 480) {}
  ImageStreamConfig(int32_t width, int32_t height) : img_width_(width), img_height_(height) {}
  void ParseFromArgv(int* argc, const char*** argv);
  void LogConfig();
  int32_t Width() const { return img_width_; }
  int32_t Height() const { return img_height_; }

  std::function<std::unique_ptr<ImageReader>(ImageStream*)> make_reader;
  std::string filename;
  std::string record;
  std::string replay;
  int32_t img_width_;
  int32_t img_height_;
};

class ImageReader { 
 public:
  virtual ~ImageReader() {}
  virtual ImageFormat get_format() = 0;
  virtual LoopEvent* get_read_event() = 0;
};

class ImageStream {
 public:
  explicit ImageStream(const ImageStreamConfig& cfg);
  void process(void *buf, size_t len, uint32_t seqnum);
  void handleFrame();
  const ImagePtr& get() { return ptr; }
  void run(std::vector<LoopEvent*> events);

  std::function<void()> redraw;
  std::function<void(unsigned int)> key_press;
  std::function<void(const ImagePtr&)> vision_cb;
  std::function<void(const DataRef&)> raw_cb;
  int refresh_rate = 1;
  aos::vision::ImageFormat fmt;

  std::function<void()>& add_fd_cb(int fd) {
    ReadEvent* rev = new ReadEvent(fd);
    events_.emplace_back(rev);
    return rev->cb;
  }

  void flag_write() { write_flag = true; }

  std::vector<LoopEvent*> events(std::vector<LoopEvent*> events) {
  for (auto& ev : events_) {
    events.emplace_back(ev.get());
  }
  return events;
  }
  std::vector<LoopEvent*> events() {
    return std::vector<LoopEvent*>();
  }
  void run() {
    run(events(std::vector<LoopEvent*>()));
  }
 private:
  // write output image
  void write_to_file(DataRef data) {
    printf("writing file (%s)\n", output_path.c_str());
    JpegStreamWriter outfile(output_path);
    outfile.WriteImage(data, GetTimeStamp());
    write_flag = false;
  }

  // flag will write a single image when set
  bool write_flag = false;
  // path to save files for written images
  std::string output_path = "./default.img";
  std::vector<std::unique_ptr<LoopEvent>> events_;
  std::unique_ptr<PixelRef[]> outbuf;
  ImagePtr ptr;
  std::unique_ptr<ImageReader> rdr;
};

class Viewer {
 public:
  virtual ~Viewer() {}

  // queues up a collection of lines to be drawn
  LinesOverlay* add_line_overlay() {
    LinesOverlay* ln = new LinesOverlay();
    overlays.emplace_back(ln);
    return ln;
  }

  // queues up a collection of lines to be drawn
  PixelLinesOverlay* add_pixel_line_overlay() {
    PixelLinesOverlay* ln = new PixelLinesOverlay();
    pixel_overlays.emplace_back(ln);
    return ln;
  }

  // queues up a collection of circles to be drawn
  CircleOverlay* add_circle_overlay() {
    CircleOverlay* ln = new CircleOverlay();
    overlays.emplace_back(ln);
    return ln;
  }
  double scale_factor = 0.75;
  int window_width_ = 100;
  int window_height_ = 100;
 protected:
  Viewer() {}
  std::vector<std::unique_ptr<OverlayBase>> overlays;
  std::vector<std::unique_ptr<OverlayBase>> pixel_overlays;
};

std::unique_ptr<Viewer> GtkView(ImageStream* strm);

}  // vision
}  // aos

#endif  // _AOS_VISION_IMAGE_IMAGE_STREAM_H_
