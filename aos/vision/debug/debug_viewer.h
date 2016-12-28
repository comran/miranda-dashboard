#ifndef AOS_VISION_DEBUG_DEBUG_VIEWER_H_
#define AOS_VISION_DEBUG_DEBUG_VIEWER_H_

#include "aos/vision/image/image_types.h"
#include "aos/vision/debug/overlay.h"
#include <cairo.h>
#include <functional>

namespace aos {
namespace vision {

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

  void Text(int x, int y, int text_x, int text_y, const std::string& text);

  cairo_t *cr_;
};

class DebugViewer {
 public:
  class Internals;
  DebugViewer(bool flip);
  ~DebugViewer();
  void Redraw();
  void UpdateImage(ImagePtr ptr);
  void SetOverlays(std::vector<OverlayBase*>* overlay);
  double scale_factor = 1.0;
  void SetScale(double scale_factor);
  void MoveTo(int x, int y);
  std::function<void(uint32_t)> key_press_event;
 private:
  int window_width_ = 100;
  int window_height_ = 100;
  std::unique_ptr<Internals> self;
};

}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_DEBUG_DEBUG_VIEWER_H_
