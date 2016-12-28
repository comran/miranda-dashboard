#ifndef _AOS_VIISON_BLOB_THRESHOLD_H_
#define _AOS_VIISON_BLOB_THRESHOLD_H_

#include "aos/vision/blob/range_image.h"
#include "aos/vision/image/image_types.h"

namespace aos {
namespace vision {

template<typename ThresholdFn>
RangeImage do_threshold(const ImagePtr& img, ThresholdFn fn) {
  RangeImage rimg(img.fmt.h);
  for (auto& row : img.by_row()) {
    bool p_score = false;
    int pstart = -1;
    std::vector<ImageRange> rngs;
    int x = 0;
    for (auto& px : row) {
      if (fn(px) != p_score) {
        if (p_score) {
          rngs.emplace_back(ImageRange(pstart, x));
        } else {
          pstart = x;
        }
        p_score = !p_score;
      }
      x++;
    }
    if (p_score) {
      rngs.emplace_back(ImageRange(pstart, img.fmt.w));
    }
    rimg.ranges.push_back(rngs);
  }
  return rimg;
}

}  // namespace vision
}  // namespace aos

#endif  //  _AOS_VIISON_BLOB_THRESHOLD_H_
