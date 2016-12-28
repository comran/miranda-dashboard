#ifndef _AOS_VIISON_BLOB_CONTOUR_H_
#define _AOS_VIISON_BLOB_CONTOUR_H_

#include "aos/vision/blob/range_image.h"
#include "aos/vision/region_alloc.h"

namespace aos {
namespace vision {

class ContourNode {
 public:
  ContourNode(int x, int y) : pt({ x, y }) { next = this; }
  ContourNode(int x, int y, ContourNode* next) : pt({ x, y }), next(next) {}
  ContourNode() {}
  Pt pt;
  ContourNode* next;
  ContourNode* append(int x, int y, AnalysisAllocator* alloc) {
    next = alloc->cons_obj<ContourNode>(x, y);
    return next;
  }
  ContourNode* pappend(int x, int y, AnalysisAllocator* alloc) {
    return alloc->cons_obj<ContourNode>(x, y, this);
  }
};

ContourNode* range_img_to_obj(const RangeImage& rimg, AnalysisAllocator* alloc);

}  // namespace vision
}  // namespace aos

#endif  //  _AOS_VIISON_BLOB_CONTOUR_H_
