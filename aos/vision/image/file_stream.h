#ifndef AOS_VISION_JPEG_STREAM_FILE_H_
#define AOS_VISION_JPEG_STREAM_FILE_H_

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <limits>
#include <assert.h>

#include "aos/vision/image/image_types.h"

namespace aos {
namespace vision {

class JpegStreamWriter {
 public:
  JpegStreamWriter(std::string fname) : 
      imgfile(fname + ".img_s"),
      indexfile(fname + ".img_index") {}

  void WriteImage(const DataRef& binary_blob, int64_t stamp) {
    offsets += binary_blob.size();
    indexfile.write((const char*)&offsets, sizeof(int));
    indexfile.write((const char*)&stamp, sizeof(int64_t));
    imgfile.write(binary_blob.data(), binary_blob.size());
    imgfile.flush();
    indexfile.flush();
  }

  int offsets = 0;
  std::ofstream imgfile;
  std::ofstream indexfile;
};

class JpegStreamReader {
 public:
  JpegStreamReader(const std::string& fname) : imgfile(fname + ".img_s") { 
    std::string indexfname = fname + ".img_index";
    int count;
    {
      std::ifstream indexfile(indexfname, std::ios::binary | std::ios::ate);
      printf("fsize: %d\n", (int)indexfile.tellg());
      count = (indexfile.tellg()) / (sizeof(int) + sizeof(int64_t));
    }
    std::ifstream indexfile(indexfname);
    printf("size is: %d\n", count);
    index.reserve(count);
    times.reserve(count);
    uint64_t offset = 0;
    int max_size = 0;
    for (int i = 0; i < count; i++) {
      uint32_t val = 0;
      int64_t time = 0;
      indexfile.read((char *)&val, sizeof(uint32_t));
      indexfile.read((char *)&time, sizeof(int64_t));
      int size = val - ((uint32_t)offset);
      if (size > max_size) max_size = size;
      offset += size;
      index.emplace_back(offset);
      times.emplace_back(time);
      //printf("%ld %d\n", offset, size);
    }
    for (int ival : index) {
      offset = ival;
    }
    printf("allocing buffer of size %d\n", max_size);

    data.reset(new char[max_size]);
    assert(count > 0);
  }

  DataRef read_at(int i) {
    int64_t start_off = 0;
    if (i > 0) { start_off = index[i - 1]; }
    int len = index[i] - start_off;
    printf("jpeg_len: %d\n", len);
    imgfile.seekg(start_off);
    DataRef data_ref;
    imgfile.read(data.get(), len);
    data_ref.Set(data.get(), len);
    return data_ref;
  }

  int64_t get_time(int i) {
    return times[i];
  }

  int size() {
    return index.size();
  }

 private:
  std::unique_ptr<char[]> data;
  std::ifstream imgfile;
  std::vector<int64_t> index;
  std::vector<int64_t> times;
};

}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_JPEG_STREAM_FILE_H_
