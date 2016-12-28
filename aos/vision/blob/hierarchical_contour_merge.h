#ifndef _AOS_VISION_BLOB_HIERARCHICAL_CONTOUR_MERGE_H_
#define _AOS_VISION_BLOB_HIERARCHICAL_CONTOUR_MERGE_H_

#include "aos/vision/blob/range_image.h"
#include "aos/vision/blob/contour.h"

namespace aos {
namespace vision {

class FittedLine {
 public:
  Pt st;
  Pt ed;
};

void hier_merge(ContourNode *stval,
                std::vector<FittedLine> *fit_lines, float merge_rate = 4.0,
                int min_len = 15);
     
inline void hier_merge(ContourNode *stval, ImagePtr /* outbuf */,
                std::vector<FittedLine> *fit_lines, float merge_rate = 4.0,
                int min_len = 15) {
  hier_merge(stval, fit_lines, merge_rate, min_len);
}

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_BLOB_HIERARCHICAL_CONTOUR_MERGE_H_
