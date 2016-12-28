#ifndef _AOS_VISION_UTILS_BRESENHAM_H_
#define _AOS_VISION_UTILS_BRESENHAM_H_

#include "aos/vision/blob/range_image.h"
#include "aos/vision/image/image_types.h"

namespace aos {
namespace vision {

void draw_bresham_line(Pt a, Pt b, ImagePtr outbuf, PixelRef color);

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_UTILS_BRESENHAM_H_
