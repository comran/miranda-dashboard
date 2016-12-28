#ifndef AOS_VISION_COMP_GEO_CONVEXHULL_H_
#define AOS_VISION_COMP_GEO_CONVEXHULL_H_

#include "aos/vision/comp_geo/vector.h"

namespace aos {
namespace vision {
namespace comp_geo {

std::vector<Vector<2>> convexhull(std::vector<Vector<2>> points);

}  // namespace comp_geo
}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_COMP_GEO_CONVEXHULL_H_
