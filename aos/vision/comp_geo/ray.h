#ifndef AOS_VISION_COMP_GEO_RAY_H_
#define AOS_VISION_COMP_GEO_RAY_H_

#include "aos/vision/comp_geo/vector.h"

namespace aos {
namespace vision {
namespace comp_geo {

// Size dimensional ray.
template<int Size>
class Ray {
 public:
    Ray() {}
  Ray(const Vector<Size>& pos, const Vector<Size>& dir) : pos(pos), dir(dir) {}
  Vector<Size> pos;
  Vector<Size> dir;
};

}  // namespace comp_geo
}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_COMP_GEO_RAY_H_

