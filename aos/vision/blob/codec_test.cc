#include "aos/vision/blob/codec.h"

#include <algorithm>
#include "gtest/gtest.h"

namespace aos {
namespace vision {

TEST(CodecTest, WriteRead) {
  BlobList blobl;
  {
    blobl.emplace_back();
    auto& blob = blobl.back();
    blob.mini = 10;
    blob.ranges.emplace_back(std::vector<ImageRange> {{10, 11}});
    blob.ranges.emplace_back(std::vector<ImageRange> {{15, 17}});
    blob.ranges.emplace_back(std::vector<ImageRange> {{19, 30}});
  }

  {
    blobl.emplace_back();
    auto& blob = blobl.back();
    blob.mini = 13;
    blob.ranges.emplace_back(std::vector<ImageRange> {{18, 19}});
    blob.ranges.emplace_back(std::vector<ImageRange> {{12, 13}});
    blob.ranges.emplace_back(std::vector<ImageRange> {{12, 17}});
  }

  std::string out;
  SerializeBlobTo(blobl, &out);
  BlobList blobl2;
  ParseBlobList(&blobl2, out.data());
  EXPECT_EQ(ShortDebugPrint(blobl), ShortDebugPrint(blobl2));
}

}  // namespace vision
}  // namespace aos
