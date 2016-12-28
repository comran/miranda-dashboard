#ifndef _AOS_VISION_BLOB_RANGE_IMAGE_H_
#define _AOS_VISION_BLOB_RANGE_IMAGE_H_

#include <vector>

#include "aos/vision/comp_geo/vector.h"
#include "aos/vision/image/image_types.h"
#include "aos/vision/debug/overlay.h"

namespace aos {
namespace vision {
 
class Pt {
 public:
  int x;
  int y;
};

class ImageRange {
 public:
  int st;
  int ed;
  int last() const { return ed - 1; }
  int calc_width() const { return ed - st; }
  ImageRange(int a, int b) : st(a), ed(b) {}
  
  bool operator<(const ImageRange& o) const { return st < o.st; }
};

class RangeImage {
 public:
//  using namespace comp_geo;
  RangeImage(int l) { ranges.reserve(l); }
  RangeImage() {}

  int size() const { return ranges.size(); }

  int npixels() {
    if (npixelsc > 0) {
      return npixelsc;
    }
    npixelsc = 0;
    for (int i = 0; i < (int)ranges.size(); ++i) {
      for (ImageRange rng : ranges[i]) {
        npixelsc += rng.ed - rng.st;
      }
    }
    return npixelsc;
  }

  int calc_area() const {
    int area = 0;
    for (auto& range : ranges) {
      for (auto& interval : range) {
        area += interval.calc_width();
      }
    }
    return area;
  }

  inline void Flip(ImageFormat fmt) const {
    Flip(fmt.w, fmt.h);
  }
  void Flip(int image_width, int image_height) const {
    std::reverse(ranges.begin(), ranges.end());
    for (std::vector<ImageRange>& range : ranges) {
      std::reverse(range.begin(), range.end());
      for (ImageRange& interval : range) {
        int tmp = image_width - interval.ed;
        interval.ed = image_width - interval.st;
        interval.st = tmp;
      }
    }

    mini = image_height - (int)ranges.size() - mini;

    // flip bounding box names
    comp_geo::Vector<2> tmp = lower_right;
    lower_right = upper_left;
    upper_left = tmp;
    tmp = lower_left;
    lower_left = upper_right;
    upper_right = tmp;

    // now flip the box
    lower_right = comp_geo::Vector<2>(image_width - lower_right.x(),
                                      image_height - lower_right.y());
    upper_right = comp_geo::Vector<2>(image_width - upper_right.x(),
                                      image_height - upper_right.y());
    lower_left = comp_geo::Vector<2>(image_width - lower_left.x(),
                                     image_height - lower_left.y());
    upper_left = comp_geo::Vector<2>(image_width - upper_left.x(),
                                     image_height - upper_left.y());
  }

  void CalcBoundingBox(comp_geo::Vector<2> &ul, comp_geo::Vector<2> &ur, comp_geo::Vector<2> &lr,
                       comp_geo::Vector<2> &ll) const {
    double x_min = 10000000000.0;
    double x_max = 0.0;
    double y_min = mini;
    double y_max = mini + ranges.size();
    for (auto &range : ranges) {
      for (auto &interval : range) {
        if (interval.st < x_min) {
          x_min = interval.st;
        }
        if (interval.ed > x_max) {
          x_max = interval.ed;
        }
      }
    }
    ul = comp_geo::Vector<2>(x_min, y_min);
    ur = comp_geo::Vector<2>(x_max, y_min);
    lr = comp_geo::Vector<2>(x_max, y_max);
    ll = comp_geo::Vector<2>(x_min, y_max);
  }

  void Draw(PixelLinesOverlay* overlay) const {
    int y = mini;
    for (auto &range : ranges) {
      for (auto &interval : range) {
        overlay->add_line(
            comp_geo::Vector<2>(interval.st, y),
            comp_geo::Vector<2>(interval.ed, y));
      }
      y++;
    }
  }

  std::vector<std::vector<ImageRange>>::const_iterator begin() const {
    return ranges.begin();
  }

  std::vector<std::vector<ImageRange>>::const_iterator end() const {
    return ranges.end();
  }

  int npixelsc = -1;

  // minimum index in y where the blob starts
  mutable int mini = 0;

  // ranges are always sorted in y then x order
  mutable std::vector<std::vector<ImageRange>> ranges;

  // used by  blob_filters.h ... Probably shouldn't though.
  mutable bool corners_set = false;
  mutable comp_geo::Vector<2> upper_left;
  mutable comp_geo::Vector<2> upper_right;
  mutable comp_geo::Vector<2> lower_right;
  mutable comp_geo::Vector<2> lower_left;
};

typedef std::vector<RangeImage> BlobList;
typedef std::vector<const RangeImage*> BlobLRef;

void draw_range_img(const RangeImage& rimg, ImagePtr outbuf, PixelRef color);

RangeImage MergeRangeImage(const BlobList& blobl);
std::string ShortDebugPrint(const BlobList& blobl);
void DebugPrint(const BlobList& blobl);

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_BLOB_RANGE_IMAGE_H_
