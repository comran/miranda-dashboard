#include "aos/vision/utils/bresenham.h"

namespace aos {
namespace vision {

inline void img_write_char(ImagePtr outbuf, int x, int y, PixelRef color) {
  if (0 <= y && y < outbuf.fmt.h && 0 <= x && x < outbuf.fmt.w) {
    outbuf.get_px(x, y) = color;
  }
}

void draw_bresham_line(Pt a, Pt b, ImagePtr outbuf, PixelRef color) {
  if (abs(b.x - a.x) > abs(b.y - a.y) ? a.x > b.x : a.y > b.y) {
    std::swap(a, b);
  }

  int dx = b.x - a.x;
  int dy = b.y - a.y;
  if (abs(dx) > abs(dy)) {
    int dup = 1;
    if (dy < 0) {
      dup = -1;
      dy = -dy;
    }
    int error_o = -abs(dx);
    int py = a.y;
    for (int px = a.x; px <= b.x; px++) {
      img_write_char(outbuf, px, py, color);
      error_o += dy * 2;
      if (error_o > 0) {
        error_o -= dx * 2;
        py += dup;
      }
    }
  } else {
    int dup = 1;
    if (dx < 0) {
      dup = -1;
      dx = -dx;
    }
    int error_o = -abs(dy);
    int px = a.x;
    for (int py = a.y; py <= b.y; py++) {
      img_write_char(outbuf, px, py, color);
      error_o += dx * 2;
      if (error_o > 0) {
        error_o -= dy * 2;
        px += dup;
      }
    }
  }
}

}  // namespace vision
}  // namespace aos
