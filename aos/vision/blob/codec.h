#ifndef _AOS_VISION_BLOB_CODEC_H_
#define _AOS_VISION_BLOB_CODEC_H_

#include <string>

#include "aos/vision/blob/range_image.h"

namespace aos {
namespace vision {

template <typename T>
inline char* WriteGenericInt(char* data, T ival) {
  *(reinterpret_cast<T*>(data)) = ival;
  return data + sizeof(T);
}
template <typename T>
inline T ReadGenericInt(const char** data) {
  auto datum = *(reinterpret_cast<const T*>(*data));
  *data += sizeof(T);
  return datum;
}
namespace {
auto WriteInt64 = WriteGenericInt<uint64_t>;
auto ReadInt64 = ReadGenericInt<uint64_t>;

auto WriteInt32 = WriteGenericInt<uint32_t>;
auto ReadInt32 = ReadGenericInt<uint32_t>;

auto WriteInt16 = WriteGenericInt<uint16_t>;
auto ReadInt16 = ReadGenericInt<uint16_t>;
}

int PredictSize(const BlobList& blob_list);
void SerializeBlob(const BlobList& blob_list, char* data);

inline void SerializeBlobTo(const BlobList& blob_list, std::string* out) {
  int len = PredictSize(blob_list);
  out->resize(len, 0);
  SerializeBlob(blob_list, &(*out)[0]);
}
const char* ParseBlobList(BlobList* blob_list, const char* data);

}  // namespace vision
}  // namespace aos

#endif  // _AOS_VISION_BLOB_CODEC_H_
