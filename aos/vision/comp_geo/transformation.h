#ifndef AOS_VISION_COMP_GEO_TRANSFORMATION_H_
#define AOS_VISION_COMP_GEO_TRANSFORMATION_H_

#include "aos/vision/comp_geo/matrix4x4.h"

namespace aos {
namespace vision {
namespace comp_geo {

class Transformation {
 public:
   Matrix4x4 m;
   Matrix4x4 invm;
   Matrix4x4 tran_invm;
   static Transformation tra(Vector<4>);
   static Transformation rot(Vector<4>);
   static Transformation sca(Vector<4>);
   Transformation comp(Transformation);
   static Matrix4x4 lie_cross(Vector<4>);
};

}  // namespace comp_geo
}  // namespace vision
}  // namespace aos

#endif //AOS_VISION_COMP_GEO_TRANSFORMATION_H_
