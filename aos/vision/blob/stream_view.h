#ifndef _AOS_VISION_BLOB_STREAM_VIEW_H_
#define _AOS_VISION_BLOB_STREAM_VIEW_H_

#include "aos/vision/blob/range_image.h"
#include "aos/vision/image/image_types.h"
#include "aos/vision/debug/debug_viewer.h"

#include <memory>

namespace aos {
namespace vision {

class BlobStreamViewer {
 public:
  BlobStreamViewer() : view_(false) {}
  void Submit(ImageFormat fmt, const BlobList& blob_list) {
    SetFormatAndClear(fmt);
    DrawBlobList(blob_list, {255, 255, 255}); 
  }
    
  inline void SetFormatAndClear(ImageFormat fmt) {
    if (fmt.w != ptr_.fmt.w || fmt.h != ptr_.fmt.h) {
      printf("resizing data: %d, %d\n", fmt.w, fmt.h);
      outbuf_.reset(new PixelRef[fmt.img_size()]);
      ptr_ = ImagePtr { fmt, outbuf_.get() };
      view_.UpdateImage(ptr_);
    }
    memset(ptr_.data, 0, fmt.img_size() * sizeof(PixelRef));
  }

  inline void DrawBlobList(const BlobList& blob_list, PixelRef color) {
    for (const auto& blob : blob_list) {
      for (int i = 0; i < (int)blob.ranges.size(); ++i) {
        for (const auto& range : blob.ranges[i]) {
          for (int j = range.st; j < range.ed; ++j) {
            ptr_.get_px(j, i + blob.mini) = color;
          }
        }
      }
    }
  }

  inline void DrawSecondBlobList(const BlobList& blob_list, PixelRef color1, PixelRef color2) {
    for (const auto& blob : blob_list) {
      for (int i = 0; i < (int)blob.ranges.size(); ++i) {
        for (const auto& range : blob.ranges[i]) {
          for (int j = range.st; j < range.ed; ++j) {
            auto px = ptr_.get_px(j, i + blob.mini);
            if (px.r == 0 && px.g == 0 && px.b == 0) {
              ptr_.get_px(j, i + blob.mini) = color1;
            } else {
              ptr_.get_px(j, i + blob.mini) = color2;
            }
          }
        }
      }
    }
  }

  DebugViewer* view() { return &view_; }

 private:
  std::unique_ptr<PixelRef[]> outbuf_;
  ImagePtr ptr_;
  
  DebugViewer view_;
};

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_BLOB_STREAM_VIEW_H_
