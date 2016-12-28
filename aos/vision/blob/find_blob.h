#ifndef _AOS_VISION_BLOB_FIND_BLOB_H_
#define _AOS_VISION_BLOB_FIND_BLOB_H_

#include "aos/vision/blob/range_image.h"

namespace aos {
namespace vision {

BlobList find_blobs(const RangeImage& rimg);

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_BLOB_FIND_BLOB_H_
