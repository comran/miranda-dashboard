#include "aos/vision/image/file_stream.h"
#include "gflags/gflags.h"

DEFINE_string(replay_fname, "/tmp/output", "File to record playback to. Empty string means no recording.");
DEFINE_string(out_fname, "/tmp/output2", "File to record playback to. Empty string means no recording.");
DEFINE_int32(replay_speed, 1, "frames to play per second of frame");
DEFINE_int32(replay_start, 0, "frames to start at");
DEFINE_int32(replay_count, 100, "number of frames to play");

namespace aos {
namespace vision {

void do_main() {
  using namespace std;

  if (FLAGS_out_fname == FLAGS_replay_fname) {
    fprintf(stderr, "error! input and output are the same\n");
    exit(-1);
  }

  JpegStreamWriter outfile(FLAGS_out_fname);
  JpegStreamReader stream_read(FLAGS_replay_fname);

  int j = 0;
  for (int i = FLAGS_replay_start;
       j < FLAGS_replay_count && i < stream_read.size();
       j++, i += FLAGS_replay_speed) {
    outfile.WriteImage(stream_read.read_at(i), stream_read.get_time(i));
  }
}

}  // namespace vision
}  // namespace aos

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  aos::vision::do_main();
  return 0;
}
