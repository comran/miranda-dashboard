#include "aos/vision/image/image_stream_legacy.h"

#include <limits>
#include <math.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#include "aos/vision/blobs.h"
#include "gflags/gflags.h"
#include "aos/vision/image/file_stream.h"

namespace aos {
namespace vision {

/*
int64_t GetTimeStamp() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(int64_t)1000 + tv.tv_usec / 1000;
}
*/

DEFINE_string(out_fname, "/tmp/output", "File to record playback to. Empty string means no recording.");

void do_main(const ImageStreamConfig& cfg) {
  ImageStream stream(cfg);
  JpegStreamWriter outfile(FLAGS_out_fname);

  int i = 0;
  //int offsets = 0;
  int64_t last_time = 0;
  stream.raw_cb = [&](const DataRef& binary_blob) {
    auto cur_time = GetTimeStamp();
    if (cur_time - last_time > 500) {
    printf("time_delta: %ld\n", cur_time - last_time);
    printf("len: %d\n", binary_blob.size());
    outfile.WriteImage(binary_blob, cur_time);
    last_time = cur_time;
    }
    ++i;
  };
  stream.run();
}

}  // namespace vision
}  // namespace aos

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  aos::vision::ImageStreamConfig config;
  auto argv_copy = (const char **)argv;
  config.ParseFromArgv(&argc, &argv_copy);
  config.LogConfig();
  aos::vision::do_main(config);
  return 0;
}
