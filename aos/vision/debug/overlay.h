#ifndef _AOS_VISION_IMAGE_OVERLAY_H_
#define _AOS_VISION_IMAGE_OVERLAY_H_

#include <vector>
#include <string>

#include "aos/vision/comp_geo/segment.h"

namespace aos {
namespace vision {

class RenderInterface {
 public:
  RenderInterface() {}
  ~RenderInterface() {}

  virtual void Translate(double x, double y) = 0;
  virtual void SetSourceRGB(double r, double g, double b) = 0;
  virtual void MoveTo(double x, double y) = 0;
  virtual void LineTo(double x, double y) = 0;
  virtual void Circle(double x, double y, double r) = 0;
  // negative in x, y, text_x, text_y measures from max in those value
  virtual void Text(int x, int y, int text_x, int text_y, const std::string& text) = 0;
  virtual void Stroke() = 0;
 private:
  RenderInterface(RenderInterface& other) = delete;
  RenderInterface(const RenderInterface& other) = delete;
  RenderInterface(RenderInterface&& other) = delete;
};

class OverlayBase {
 public:
  OverlayBase() {}
  virtual ~OverlayBase() {}

  // draw this overlay to the given canvas
  virtual void draw(RenderInterface &render, double /* width */, double /* height */) = 0;

  // clear the entire overlay
  virtual void reset() = 0;

  PixelRef color = {255, 0, 0};
  double scale = 1.0;
};

class LambdaOverlay : public OverlayBase {
 public:
  std::function<void(RenderInterface &, double, double)> draw_fn;
  void draw(RenderInterface &render, double width, double height) override {
    if (draw_fn) draw_fn(render, width, height);
  }
  void reset() override {}
};

class LinesOverlay : public OverlayBase {
 public:
  LinesOverlay() : OverlayBase() {}
  ~LinesOverlay() {}

  // build a segment for this line
  void add_line(comp_geo::Vector<2> st, comp_geo::Vector<2> ed) {
    add_line(st, ed, color);
  }

  // build a segment for this line
  void add_line(comp_geo::Vector<2> st, comp_geo::Vector<2> ed,
                PixelRef newColor) {
    lines_.emplace_back(std::pair<comp_geo::Segment<2>, PixelRef>(
        comp_geo::Segment<2>(st, ed), newColor));
  }

  void add_point(comp_geo::Vector<2> pt) { add_point(pt, color); }

  // add a new point connected to the last point in the line
  void add_point(comp_geo::Vector<2> pt, PixelRef newColor) {
    if (lines_.empty()) {
      lines_.emplace_back(std::pair<comp_geo::Segment<2>, PixelRef>(
          comp_geo::Segment<2>(pt, pt), newColor));
    } else {
      comp_geo::Vector<2> st = lines_.back().first.B();
      lines_.emplace_back(std::pair<comp_geo::Segment<2>, PixelRef>(
          comp_geo::Segment<2>(st, pt), newColor));
    }
  }

  void draw(RenderInterface &render, double w, double h) {
    render.Translate(w / 2.0, h / 2.0);
    for (const auto &ln : lines_) {
      PixelRef localColor = ln.second;
      render.SetSourceRGB(localColor.r / 255.0, localColor.g / 255.0,
                          localColor.b / 255.0);
      render.MoveTo(scale * ln.first.A().x(), -scale * ln.first.A().y());
      render.LineTo(scale * ln.first.B().x(), -scale * ln.first.B().y());
      render.Stroke();
    }
  }

  // empting the list will blank the whole overlay
  void reset() { lines_.clear(); }

 private:
  // lines in this over lay
  std::vector<std::pair<comp_geo::Segment<2>, PixelRef>> lines_;
};

class PixelLinesOverlay : public OverlayBase {
 public:
  PixelLinesOverlay() : OverlayBase() {}
  ~PixelLinesOverlay() {}

  // build a segment for this line
  void add_line(comp_geo::Vector<2> st, comp_geo::Vector<2> ed) {
    add_line(st, ed, color);
  }

  // build a segment for this line
  void add_line(comp_geo::Vector<2> st, comp_geo::Vector<2> ed,
                PixelRef newColor) {
    lines_.emplace_back(std::pair<comp_geo::Segment<2>, PixelRef>(
        comp_geo::Segment<2>(st, ed), newColor));
  }

  void start_new_profile() { start_profile = true; }

  // add a new point connected to the last point in the line
  void add_point(comp_geo::Vector<2> pt, PixelRef newColor) {
    if (lines_.empty() || start_profile) {
      lines_.emplace_back(std::pair<comp_geo::Segment<2>, PixelRef>(
          comp_geo::Segment<2>(pt, pt), newColor));
      start_profile = false;
    } else {
      comp_geo::Vector<2> st = lines_.back().first.B();
      lines_.emplace_back(std::pair<comp_geo::Segment<2>, PixelRef>(
          comp_geo::Segment<2>(st, pt), newColor));
    }
  }

  void draw(RenderInterface &render, double /*width*/, double /*hieght*/) {
    for (const auto &ln : lines_) {
      PixelRef localColor = ln.second;
      render.SetSourceRGB(localColor.r / 255.0, localColor.g / 255.0,
                          localColor.b / 255.0);
      render.MoveTo(ln.first.A().x(), ln.first.A().y());
      render.LineTo(ln.first.B().x(), ln.first.B().y());
      render.Stroke();
    }
  }

  // empting the list will blank the whole overlay
  void reset() { lines_.clear(); }

 private:
  // lines in this over lay
  std::vector<std::pair<comp_geo::Segment<2>, PixelRef>> lines_;
  bool start_profile = false;
};

class CircleOverlay : public OverlayBase {
 public:
  CircleOverlay() : OverlayBase() {}
  ~CircleOverlay() {}

  // build a circle as a point and radius
  std::pair<comp_geo::Vector<2>, double> *add_circle(comp_geo::Vector<2> center,
                                                     double radius) {
    circles_.emplace_back(
        std::pair<comp_geo::Vector<2>, double>(center, radius));
    return &(circles_.back());
  }

  void draw(RenderInterface &render, double w, double h) {
    render.Translate(w / 2.0, h / 2.0);
    render.SetSourceRGB(color.r / 255.0, color.g / 255.0, color.b / 255.0);
    for (const auto &circle : circles_) {
      render.Circle(scale * circle.first.x(), -scale * circle.first.y(),
                    scale * circle.second);
      render.Stroke();
    }
  }

  // empting the list will blank the whole overlay
  void reset() { circles_.clear(); }

 private:
  // circles in this over lay
  std::vector<std::pair<comp_geo::Vector<2>, double>> circles_;
};

}  // vision
}  // aos

#endif  // _AOS_VISION_IMAGE_OVERLAY_H_
