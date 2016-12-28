#ifndef _AOS_VISION_IMAGE_JPEGROUTINES_H_
#define _AOS_VISION_IMAGE_JPEGROUTINES_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "aos/vision/image/image_types.h"

namespace aos {
namespace vision {

bool process_jpeg(unsigned char *out, const unsigned char *image, size_t size);

// Returns true if successful false if an error was encountered.
inline bool process_jpeg(DataRef data, PixelRef *out) {
  return process_jpeg(
      (unsigned char *)out,
      (const unsigned char *)data.data(),
      (size_t)data.size());
}

ImageFormat get_fmt(DataRef data);

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_IMAGE_JPEGROUTINES_H_

