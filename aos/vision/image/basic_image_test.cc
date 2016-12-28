#include "gtest/gtest.h"

#include "aos/vision/image/image_types.h"
#include "aos/vision/image/reader.h"
#include "aos/vision/image/jpeg_routines.h"
#include "aos/vision/image/image_stream_legacy.h"

namespace aos {
namespace vision {
namespace image {
namespace testing {

using namespace aos::vision::image;
using namespace camera;

class BasicImageTest : public ::testing::Test {
 protected:
  BasicImageTest() { }
  ~BasicImageTest() { }
};

// Tests basic vector math.
TEST_F(BasicImageTest, BasicImage) {
	ImageFormat f {100, 100};
	f.ToString();
}

// Tests basic vector math.
TEST_F(BasicImageTest, BasicReader) {
//ImageStream s("/dev/video0");
//	s.handleFrame();
}

}  // namespace testing
}  // namespace cimage
}  // namespace vision
}  // namespace aos

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

