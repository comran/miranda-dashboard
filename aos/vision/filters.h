#ifndef AOS_VISION_FILTERS_H_
#define AOS_VISION_FILTERS_H_

#include "aos/vision/image/image_types.h"

namespace aos {
namespace vision {

template <typename GetRowOrColPtr>
void BoxFilter(const GetRowOrColPtr &row_or_col_ptr, int box_num) {
  typedef typename GetRowOrColPtr::ImageType ImageType;
  typedef typename PixelTypeSumTraits<ImageType>::SumType SumType;
  int buff_window = box_num * 2 + 1;

  int row_or_col_count = row_or_col_ptr.sub_count();
  if (box_num * 2 + 1 > row_or_col_count) return;
  ImageType buff[buff_window];
  for (auto row_or_col : row_or_col_ptr) {
    SumType sum;
    for (int i = 0; i < box_num; ++i) {
      sum.add_px(row_or_col.get_px(i));
    }
    int i = 0;
    for (i = 0; i < box_num; ++i) {
      sum.add_px(row_or_col.get_px(i + box_num));
      buff[i % buff_window] = sum.get_avg();
    }
    for (; i < row_or_col_count - box_num; ++i) {
      sum.add_px(row_or_col.get_px(i + box_num));
      buff[i % buff_window] = sum.get_avg();
      sum.sub_px(row_or_col.get_px(i - box_num));
      row_or_col.get_px(i - box_num) = buff[(i - box_num) % buff_window];
    }
    for (; i < row_or_col_count; ++i) {
      buff[i % buff_window] = sum.get_avg();
      sum.sub_px(row_or_col.get_px(i - box_num));
      row_or_col.get_px(i - box_num) = buff[(i - box_num) % buff_window];
    }
    for (i -= box_num; i < row_or_col_count; ++i) {
      row_or_col.get_px(i) = buff[(i) % buff_window];
    }
  }
}

template <typename ImageType>
void HBoxFilter(const Array2dPtr<ImageType> &img, int box_num) {
  BoxFilter(img.by_row(), box_num);
}

template <typename ImageType>
void VBoxFilter(const Array2dPtr<ImageType> &img, int box_num) {
  BoxFilter(img.by_col(), box_num);
}

inline void HDeriv(const GrayImagePtr &deriv, const ImagePtr &src) {
  auto d_it = deriv.by_row().begin();
  for (auto row : src.by_row()) {
    auto rderiv = *d_it;
    for (int i = 1; i < row.count() - 1; ++i) {
      rderiv.get_px(i) = row.get_px(i + 1).r - row.get_px(i - 1).r;
    }
    ++d_it;
  }
}

inline void VDeriv(const GrayImagePtr &deriv, const ImagePtr &src) {
  auto d_it = deriv.by_col().begin();
  for (auto col : src.by_col()) {
    auto rderiv = *d_it;
    for (int i = 1; i < col.count() - 1; ++i) {
      rderiv.get_px(i) = col.get_px(i + 1).r - col.get_px(i - 1).r;
    }
    ++d_it;
  }
}

template <typename ImageType>
void DoGuass(const Array2dPtr<ImageType> &img, int box_num) {
  HBoxFilter(img, box_num);
  VBoxFilter(img, box_num);
  HBoxFilter(img, box_num);
  VBoxFilter(img, box_num);
  HBoxFilter(img, box_num);
  VBoxFilter(img, box_num);
}

inline void Mult(const GrayImagePtr &dest, const GrayImagePtr &a,
                 const GrayImagePtr &b) {
  auto d_it = dest.begin();
  auto a_it = a.begin();
  auto b_it = b.begin();
  for (; d_it != dest.end(); ++d_it, ++a_it, ++b_it) {
    *d_it = (*a_it) * (*b_it);
  }
}

}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_FILTERS_H_
