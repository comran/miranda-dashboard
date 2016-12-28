#ifndef _AOS_VISION_IMAGE_IMAGE_TYPES_H_
#define _AOS_VISION_IMAGE_IMAGE_TYPES_H_

#include <stdint.h>
#include <sstream>
#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace aos {
namespace vision {

class DataRef {
 public:
  void Set(const char* data, int len) { data_ = data; len_ = len; }
  const char* data() const { return data_; }
  int size() const { return len_; }
  const char* data_;
  int len_;
};

class ImageFormat {
 public:

  ImageFormat() : w(0), h(0) { }
  ImageFormat(int nw, int nh) : w(nw), h(nh) { }
  int w;
  int h;
  std::string ToString() {
    std::ostringstream s;
    s << "ImageFormat {" << w << ", " << h << "}";
    return s.str();
  }
  int img_size() const { return w * h; }
};

class PixelRef {
 public:
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct ImgCursor {
  int x;
  int y;
};

static_assert(sizeof(PixelRef) == 3, "Problem with cows in fields!");

template<typename ImageType>
class CellIter {
 public:
  CellIter(ImageType* data) : data_(data) {}
  ImageType& operator*() { return *data_; }
  ImageType* operator->() { return data_; }
  CellIter& operator++() {
    ++data_;
    return *this;
  }
  bool operator!=(const CellIter<ImageType>& other) { return other.data_ != data_; }
 private:
  ImageType* data_;
};

template<typename ImageType>
class RowRef {
 public:
  RowRef(ImageType* data, int w) : data_(data), w_(w) {}
  CellIter<ImageType> begin() const {
    return CellIter<ImageType>((data_));
  }
  CellIter<ImageType> end() const {
    return CellIter<ImageType>((data_ + w_));
  }
  RowRef<ImageType>& operator++() {
    data_ += w_;
    return *this;
  }
  bool operator!=(const RowRef<ImageType>& other) const { return other.data_ != data_; }
  int count() { return w_; }
  ImageType& get_px(int i) const {
    if (i < 0 || i >= w_) {
      printf("%d out of range [%d]\n", i, w_);
      exit(-1);
    }
    return (data_[i]);
  }
 private:
  ImageType* data_;
  int w_;
};

template<typename ImageType>
class RowIter {
 public:
  RowIter(ImageType* data, int w) : data_(data, w) {}
  RowRef<ImageType>& operator*() { return data_; }
  RowIter<ImageType>& operator++() {
    ++data_;
    return *this;
  }
  bool operator!=(const RowIter<ImageType>& other) { return other.data_ != data_; }
 private:
  RowRef<ImageType> data_;
};

template<typename ImageT>
class RowPtr {
 public:
  typedef ImageT ImageType;
  RowPtr(ImageFormat fmt, ImageType* data) : fmt_(fmt), data_(data) {}
  RowIter<ImageType> begin() const {
    return RowIter<ImageType>(data_, fmt_.w);
  }
  RowIter<ImageType> end() const {
    return RowIter<ImageType>(data_ + fmt_.w * fmt_.h, fmt_.w);
  }
  int sub_count() const { return fmt_.w; }
  
 private:
  ImageFormat fmt_;
  ImageType* data_;
};

template<typename ImageType>
class ColRef {
 public:
  ColRef(ImageType* data, ImageFormat fmt) : data_(data), fmt_(fmt) {}
  ColRef<ImageType>& operator++() {
    ++data_;
    return *this;
  }
  bool operator!=(const ColRef<ImageType>& other) const { return other.data_ != data_; }
  int count() { return fmt_.h; }
  ImageType& get_px(int i) const {
    return (data_[(i * fmt_.w)]);
  }
 private:
  ImageType* data_;
  ImageFormat fmt_;
};

template<typename ImageType>
class ColIter {
 public:
  ColIter(ImageType* data, ImageFormat fmt) : data_(data, fmt) {}
  ColRef<ImageType>& operator*() { return data_; }
  ColIter<ImageType>& operator++() {
    ++data_;
    return *this;
  }
  bool operator!=(const ColIter<ImageType>& other) { return other.data_ != data_; }
 private:
  ColRef<ImageType> data_;
};

template<typename ImageT>
class ColPtr {
 public:
  typedef ImageT ImageType;
  ColPtr(ImageFormat fmt, ImageType* data) : fmt_(fmt), data_(data) {}
  ColIter<ImageType> begin() const {
    return ColIter<ImageType>(data_, fmt_);
  }
  ColIter<ImageType> end() const {
    return ColIter<ImageType>(data_ + fmt_.w, fmt_);
  }
  int sub_count() const { return fmt_.h; }
  
 private:
  ImageFormat fmt_;
  ImageType* data_;
};

template<typename ImageType>
class Array2dPtr {
 public:
  ImageFormat fmt;
  ImageType* data;
  CellIter<ImageType> begin() const {
    return CellIter<ImageType>((data));
  }
  CellIter<ImageType> end() const {
    return CellIter<ImageType>((data + fmt.w * fmt.h));
  }
  RowPtr<ImageType> by_row() const {
    return RowPtr<ImageType> { fmt, data }; 
  }
  ColPtr<ImageType> by_col() const {
    return ColPtr<ImageType> { fmt, data }; 
  }
  ImageType& get_px(int i, int j) const {
#ifndef NBOUNDSCHECK
    if (i < 0 || i >= fmt.w || j < 0 || j >= fmt.h) {
      printf("%d, %d out of range [%dx %d]\n", i, j, fmt.w, fmt.h);
      exit(-1);
    }
#endif  // NBOUNDSCHECK
    return data[(i + j * fmt.w)];
  }
  void CopyFrom(const Array2dPtr& other) const {
    memcpy(data, other.data, sizeof(ImageType) * fmt.img_size());
  }
};

template<typename ImageType>
class ValueArray2d {
 public:
  ValueArray2d() {}
  ValueArray2d(ImageFormat fmt) : fmt_(fmt) {
    data_.reset(new ImageType[fmt.img_size()]);
  } 
  Array2dPtr<ImageType> img_ptr() {
    return Array2dPtr<ImageType>{ fmt_, data_.get()};
  }
  ImageType* data() { return data_.get(); }
  ImageFormat fmt_;
  std::unique_ptr<ImageType[]> data_;
};

typedef Array2dPtr<PixelRef> ImagePtr; 
typedef ValueArray2d<PixelRef> ImageValue;
typedef ValueArray2d<int16_t> GrayImageValue;
typedef Array2dPtr<int16_t> GrayImagePtr;

class PxSum {
 public:
  void add_px(const PixelRef& pix) {
    r += pix.r;
    g += pix.g;
    b += pix.b;
    ++c;
  }
  void sub_px(const PixelRef& pix) {
    r -= pix.r;
    g -= pix.g;
    b -= pix.b;
    --c;
  }
  int c = 0;
  int r = 0;
  int g = 0;
  int b = 0;
  PixelRef get_avg() const { return PixelRef{(uint8_t)(r / c), (uint8_t)(g / c), (uint8_t)(b / c)}; }
};

class GraySum {
 public:
  int c = 0;
  int val = 0;
  int16_t get_avg() const { return val / c; }
  void add_px(int16_t pix) { val += pix; c++; }
  void sub_px(int16_t pix) { val -= pix; c--; }
};

template <typename ImageType>
struct PixelTypeSumTraits {};

template <>
struct PixelTypeSumTraits<PixelRef> {
  typedef PxSum SumType;
};

template <>
struct PixelTypeSumTraits<int16_t> {
  typedef GraySum SumType;
};

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_IMAGE_IMAGE_TYPES_H_
