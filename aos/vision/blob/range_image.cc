#include "aos/vision/blob/range_image.h"

namespace aos {
namespace vision {

void draw_range_img(const RangeImage &rimg, ImagePtr outbuf, PixelRef color) {
  for (int i = 0; i < (int)rimg.ranges.size(); ++i) {
    int y = rimg.mini + i;
    for (ImageRange rng : rimg.ranges[i]) {
      for (int x = rng.st; x < rng.ed; ++x) {
        outbuf.get_px(x, y) = color;
      }
      outbuf.get_px(rng.st, y) = color;
      outbuf.get_px(rng.ed - 1, y) = color;
    }
  }
}

RangeImage MergeRangeImage(const BlobList& blobl) {
  if (blobl.size() == 1) return blobl[0];

  int i = blobl[0].mini;
  for (const RangeImage& subrimg : blobl) {
    if (i > subrimg.mini) i = subrimg.mini;
  }
  RangeImage rimg;
  rimg.mini = i;
  while (true) {
    std::vector<ImageRange> range_lst;
    int n_missing = 0;
    for (const RangeImage& subrimg : blobl) {
      if (subrimg.mini > i) continue;
      int ri = i - subrimg.mini;
      if (ri < (int)subrimg.ranges.size()) {
        for (const auto& span : subrimg.ranges[ri]) {
          range_lst.emplace_back(span);
        }
      } else {
        ++n_missing;
      }
    }
    std::sort(range_lst.begin(), range_lst.end());
    rimg.ranges.emplace_back(std::move(range_lst));
    if (n_missing == (int)blobl.size()) return rimg;
    ++i;
  }
}

std::string ShortDebugPrint(const BlobList& blobl) {
  RangeImage rimg = MergeRangeImage(blobl);
  std::string out;
  out += "{";
  out += "mini: " + std::to_string(rimg.mini);
  for (const auto& line : rimg) {
    out += "{";
    for (const auto& span : line) {
      out += "{" + std::to_string(span.st) + ", " + std::to_string(span.ed) + "},";
    }
    out += "},";
  }
  out += "}";
  return out;
}

void DebugPrint(const BlobList& blobl) {
  RangeImage rimg = MergeRangeImage(blobl);
  int minx = rimg.ranges[0][0].st;
  int maxx = 0;
  for (const auto& range : rimg.ranges) {
    for (const auto& span : range) {
      if (span.st < minx) minx = span.st;
      if (span.ed > maxx) maxx = span.ed;
    }
  }
  fprintf(stderr, "maxx: %d minx: %d\n", maxx, minx);
  char buf[maxx - minx];
  for (const auto& range : rimg.ranges) {
    int i = minx;
    for (const auto& span : range) {
      for (; i < span.st; ++i) buf[i - minx] = ' ';
      for (; i < span.ed; ++i) buf[i - minx] = '#';
    }
    for (; i < maxx; ++i) buf[i - minx] = ' ';
    fprintf(stderr, "%.*s\n", maxx - minx, buf);
  }
}

}  // namespace vision
}  // namespace aos
