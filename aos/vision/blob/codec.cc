#include "aos/vision/blob/codec.h"

namespace aos {
namespace vision {

int PredictSize(const BlobList& blob_list) {
  int count = 2;
  for (const auto& blob : blob_list) {
    count += 4;
    for (int i = 0; i < (int)blob.ranges.size(); ++i) {
      count += 2 + 4 * blob.ranges[i].size();
    }
  }
  return count;
}

void SerializeBlob(const BlobList& blob_list, char* data) {
  data = WriteInt16(data, static_cast<uint16_t>(blob_list.size()));
  for (const auto& blob : blob_list) {
    data = WriteInt16(data, static_cast<uint16_t>(blob.mini));
    data = WriteInt16(data, static_cast<uint16_t>(blob.ranges.size()));
    for (int i = 0; i < (int)blob.ranges.size(); ++i) {
      data = WriteInt16(data, static_cast<uint16_t>(blob.ranges[i].size()));
      for (const auto& range : blob.ranges[i]) {
        data = WriteInt16(data, static_cast<uint16_t>(range.st));
        data = WriteInt16(data, static_cast<uint16_t>(range.ed));
      }
    }
  }
}

const char* ParseBlobList(BlobList* blob_list, const char* data) {
  int num_items = ReadInt16(&data); 
  blob_list->reserve(num_items);
  for (int i = 0; i < num_items; ++i) {
    blob_list->emplace_back();
    auto& blob = blob_list->back();
    blob.mini = ReadInt16(&data);
    int num_ranges = ReadInt16(&data);
    blob.ranges.reserve(num_ranges);
    for (int j = 0; j < num_ranges; ++j) {
      blob.ranges.emplace_back();
      auto& ranges = blob.ranges.back();
      int num_sub_ranges = ReadInt16(&data);
      for (int k = 0; k < num_sub_ranges; ++k) {
        int st = ReadInt16(&data);
        int ed = ReadInt16(&data);
        ranges.emplace_back(st, ed);
      }
    }
  }
  return data;
}

}  // namespace vision
}  // namespace aos
