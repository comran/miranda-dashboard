#include <memory>
#include <vector>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <poll.h>
#include <functional>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>
#include "gflags/gflags.h"

#include "aos/vision/image/image_stream_legacy.h"
#include "aos/vision/image/jpeg_routines.h"
#include "aos/vision/image/events.h"
#include "aos/vision/image/reader.h"

namespace aos {
namespace vision {

#include "aos/vision/debug/overlay.h"
  
class CairoRender : public RenderInterface {
 public:
  CairoRender(cairo_t *cr) : cr_(cr) {}
  virtual ~CairoRender() {}

  void Translate(double x, double y) { cairo_translate(cr_, x, y); }

  void SetSourceRGB(double r, double g, double b) {
    cairo_set_source_rgb(cr_, r, g, b);
  }

  void MoveTo(double x, double y) { cairo_move_to(cr_, x, y); }

  void LineTo(double x, double y) { cairo_line_to(cr_, x, y); }

  void Circle(double x, double y, double r) {
    cairo_arc(cr_, x, y, r, 0.0, 2 * M_PI);
  }

  void Stroke() { cairo_stroke(cr_); }

  cairo_t *cr_;
};

DEFINE_string(record, "", "File to record playback to. Empty string means no recording.");
DEFINE_string(replay, "", "File to replay playback from. Empty string means no replay.");

void ImageStreamConfig::ParseFromArgv(int* argc, const char*** argv) {
  if (*argc == 2) {
    filename = (*argv)[1];
  } else {
    filename = "/dev/video0";
  }

  replay = FLAGS_replay;

  record = FLAGS_record;
};

void ImageStreamConfig::LogConfig() {
  printf("config:\n");
  if (filename.size() > 0) {
    printf("  filename: %s\n", filename.c_str());
  }
  if (record.size() > 0) {
    printf("  record: %s\n", record.c_str());
  }
}

class WebcamReader : public ImageReader {
 public:
  WebcamReader(::camera::Reader* reader) : reader_(reader) {}
  LoopEvent* get_read_event() override {
    return new ReadEvent(reader_->getFd(), [this]() { reader_->handleFrame(); });
  }
  aos::vision::ImageFormat get_format() override {
    return reader_->get_format();
  }
  std::unique_ptr<::camera::Reader> reader_;
};

void ImageStream::run(std::vector<LoopEvent*> events) {
  for (auto& ev : events_) {
    events.emplace_back(ev.get());
  }
  run_main(events);
}
  
ImageStream::ImageStream(const ImageStreamConfig& cfg) {
  using namespace std::placeholders;

  if (cfg.make_reader) {
    rdr = cfg.make_reader(this);
  } else {
    auto camread = new ::camera::Reader(
        cfg.filename.c_str(),
        std::bind(&ImageStream::process, this, _1, _2, _3),
        cfg.Width(), cfg.Height());
    camread->startAsync();
    rdr.reset(new WebcamReader(camread));
  }

  LoopEvent* ev = rdr->get_read_event();
  if (ev != NULL) events_.emplace_back(ev);

  fmt = rdr->get_format();

  outbuf.reset(new PixelRef[fmt.img_size()]);
  ptr = ImagePtr { fmt, outbuf.get() };
}

void ImageStream::process(void* buf, size_t len, uint32_t seqnum) {
  if (len < 300) {
    LOG(DEBUG, "got bad img: %d of size(%lu)\n", seqnum, len);
    return;
  }
  // LOG(DEBUG, "got aimgg: %d of size(%lu)\n", seqnum, len);
  if (seqnum % refresh_rate == 0) {
    DataRef data{(const char*)buf, (int)len};
    if (raw_cb) {
      raw_cb(data);
    }
    if (vision_cb || redraw) {
      process_jpeg((unsigned char*)outbuf.get(), (unsigned char*)buf, len);
      if (vision_cb) vision_cb(ptr);
      if (redraw) redraw();
    }
    if (write_flag) {
      write_to_file(data);
    }
  }
}

template <typename T, gboolean (T::*DrawMethod)(cairo_t* cr)>
gboolean draw_callback(GtkWidget*, cairo_t* cr, gpointer data) {
  return ((*((T*)data)).*DrawMethod)(cr);
}

template <typename T, gboolean (T::*DrawMethod)(cairo_t* cr)>
void g_draw_signal_connect(GtkWidget* widget, T* obj) {
  gboolean (*fnptr)(GtkWidget*, cairo_t*, gpointer) = draw_callback<T, DrawMethod>;
  g_signal_connect(widget, "draw", G_CALLBACK(fnptr), obj);
}

class StreamViewer : public Viewer {
 public:
  explicit StreamViewer(ImageStream* strm);
  ~StreamViewer() override {}
  gboolean handle_key_press(GdkEventKey *event) {
    if (stream->key_press) {
      stream->key_press(event->keyval);
    }
    return FALSE;
  }
 protected:
  gboolean draw(cairo_t *cr);
  void redraw();
 private:
  bool needs_draw = true;
  ImageStream* stream;
  GdkPixbuf* pixbuf;
  GtkWidget* drawing_area;
};

std::unique_ptr<Viewer> GtkView(ImageStream* strm) {
  return std::unique_ptr<Viewer>(new StreamViewer(strm));
}

gboolean stream_key_press_event(GtkWidget *widget, GdkEventKey *event, StreamViewer* strm) {
  (void)widget;
  return strm->handle_key_press(event);
}

StreamViewer::StreamViewer(ImageStream* strm) {
  stream = strm;
  strm->redraw = std::bind(&StreamViewer::redraw, this);

  gtk_init(NULL, NULL);
  GtkWidget* window;
  drawing_area = gtk_drawing_area_new();
  window_width_ = strm->get().fmt.w;
  window_height_ = strm->get().fmt.h;
  gtk_widget_set_size_request(drawing_area, window_width_ * scale_factor,
                              window_height_ * scale_factor);
  gtk_widget_add_events(drawing_area, GDK_KEY_PRESS_MASK);


  pixbuf = gdk_pixbuf_new_from_data(
      (uint8_t*)strm->get().data, GDK_COLORSPACE_RGB, FALSE, 8, window_width_,
      window_height_, 3 * window_width_, NULL, NULL);

  g_draw_signal_connect<StreamViewer, &StreamViewer::draw>(drawing_area, this);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "key-press-event", G_CALLBACK(stream_key_press_event), this);
  g_signal_connect(window, "destroy", G_CALLBACK(main_loop_quit), NULL);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width_ * scale_factor,
                              window_height_ * scale_factor);

  gtk_container_add(GTK_CONTAINER(window), drawing_area);
  gtk_widget_show_all(window);
}

gboolean StreamViewer::draw(cairo_t *cr) {
  cairo_save(cr);
  cairo_scale(cr, scale_factor, scale_factor);
  gdk_cairo_set_source_pixbuf(cr, pixbuf, 0.0, 0.0);
  cairo_paint (cr);
  cairo_restore(cr);

  for (const auto& ov : overlays) {
    cairo_save(cr);
    // move the drawing to match the window size
    cairo_scale(cr, scale_factor, scale_factor);
    cairo_translate(cr, window_width_ / 2.0, window_height_ / 2.0);
    CairoRender render(cr);
    ov->draw(render, stream->fmt.w, stream->fmt.h);
    cairo_restore(cr);
  }

  for (const auto& ov : pixel_overlays) {
    cairo_save(cr);
    cairo_scale(cr, scale_factor, scale_factor);
    CairoRender render(cr);
    ov->draw(render, stream->fmt.w, stream->fmt.h);
    cairo_restore(cr);
  }

  needs_draw = false;
  return FALSE;
}

void StreamViewer::redraw() {
  if (!needs_draw) {
    gtk_widget_queue_draw(drawing_area);
    needs_draw = true;
  }
}

}  // vision
}  // aos
