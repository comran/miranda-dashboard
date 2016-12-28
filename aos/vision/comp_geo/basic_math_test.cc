#include <ostream>
#include "gtest/gtest.h"

#include "aos/vision/comp_geo/vector.h"
#include "aos/vision/comp_geo/segment.h"
#include "aos/vision/comp_geo/ray.h"
#include "aos/vision/comp_geo/matrix4x4.h"
//#include "aos/vision/comp_geo/transformation.h"

namespace aos {
namespace vision {
namespace comp_geo {
namespace testing {

using namespace aos::vision::comp_geo;

class BasicMathTest : public ::testing::Test {
 protected:
  BasicMathTest() { }
  ~BasicMathTest() { }
};

// Tests basic vector math.
TEST_F(BasicMathTest, BasicVector) {
  Vector<3> vec1(1.0, 1.0, 1.0);
  Vector<3> vec2(2.0, 4.0, 6.0);
  // Equality.
  EXPECT_FALSE(vec1 == vec2);
  EXPECT_TRUE(Vector<3>(2.0, 4.0, 6.0) == vec2);

  // Addition.
  Vector<3> vec3 = vec1 + vec2;
  EXPECT_EQ(3.0, vec3.x());
  EXPECT_EQ(5.0, vec3.y());
  EXPECT_EQ(7.0, vec3.z());

  // Multiplication.
  vec1 *= 2.0;
  EXPECT_EQ(2.0, vec1.x());
  EXPECT_EQ(2.0, vec1.y());
  EXPECT_EQ(2.0, vec1.z());

  vec2 = 2.0 * vec1;
  EXPECT_EQ(4.0, vec2.x());
  EXPECT_EQ(4.0, vec2.y());
  EXPECT_EQ(4.0, vec2.z());

  
  // Magnitude.
  Vector<3> vec4(1.0, 1.0, 1.0);
  EXPECT_NEAR(1.732, vec4.Mag(), 0.001);

  // Combo of opertaions.
  Vector<3> vec5(2.0, 2.0, 1.0);
  Vector<3> vec6(1.0, 1.0, 1.0);
  EXPECT_NEAR(1.414, (vec5 - vec6).Mag(), 0.001);
  EXPECT_NEAR(1.414, (vec6 - vec5).Mag(), 0.001);

  // Sign.
  Vector<3> vec7 = vec5 - vec6;
  Vector<3> vec8 = vec6 - vec5;
  EXPECT_EQ(1.0, vec7.x());
  EXPECT_EQ(1.0, vec7.y());
  EXPECT_EQ(0.0, vec7.z());
  EXPECT_EQ(-1.0, vec8.x());
  EXPECT_EQ(-1.0, vec8.y());
  EXPECT_EQ(0.0, vec8.z());

  std::ostringstream ss;
  Vector<3> vec9(0.1, 0.2, 0.3);
  ss << vec9;
  EXPECT_EQ("[0.1, 0.2, 0.3]", ss.str());
}

// Tests basic segment math.
TEST_F(BasicMathTest, VectorMath) {
  // Angle
  Vector<3> vec1(1.0, 0.0, 0.0);
  Vector<3> vec2(0.0, 1.0, 0.0);
  EXPECT_NEAR(M_PI/2, vec1.AngleTo(vec2), 0.0001);
  vec2 = Vector<3>(1.0, 1.0, 0.0);
  EXPECT_NEAR(M_PI/4, vec1.AngleTo(vec2), 0.0001);

  // Projection then Rejection.
  vec2 = Vector<3>(0.725, 0.658, 0.0);
  EXPECT_EQ(Vector<3>(0.725, 0.0, 0.0), vec2.ProjectTo(vec1));
  EXPECT_EQ(Vector<3>(0.0, 0.658, 0.0), vec2.RejectFrom(vec1));
}

// Tests basic segment math.
TEST_F(BasicMathTest, BasicSegment) {
	Segment<3> seg(1.0, 1.0, 1.0, 2.0, 2.0, 1.0);
	EXPECT_NEAR(1.414, seg.Mag(), 0.001);
}

// Tests basic ray math.
TEST_F(BasicMathTest, BasicRay) {
	Ray<3> ray1(Vector<3>(1.1, 2.2, 3.3),
			Vector<3>(0.1, 0.1, 0.1));
}

// Tests basic matrix4x4 math.
TEST_F(BasicMathTest, BasicMatrix4x4) {
	Matrix4x4 mat;
	mat = mat.mult(3.0);
	EXPECT_EQ(3.0, mat[0][0]);
	EXPECT_EQ(3.0, mat[1][1]);
	EXPECT_EQ(0.0, mat[0][1]);
	EXPECT_EQ(0.0, mat[1][0]);
}

// Tests basic transformation math.
TEST_F(BasicMathTest, BasicTransformation) {
//	Transformation trans;
}


}  // namespace testing
}  // namespace comp_geo
}  // namespace vision
}  // namespace aos

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

