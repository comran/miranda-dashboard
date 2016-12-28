#ifndef AOS_VISION_CHECKER_BOARD_H_
#define AOS_VISION_CHECKER_BOARD_H_

#include "aos/vision/image/image_types.h"

namespace aos {
namespace vision {

void FindCheckers(const ImagePtr& img, const ImagePtr& dbg);

}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_CHECKER_BOARD_H_
