#include "aos/vision/image/image_stream_legacy.h"

#include <limits>
#include <math.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <time.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "gflags/gflags.h"

#include "aos/vision/image/jpeg_routines.h"
#include "aos/vision/image/events.h"
#include "aos/vision/blobs.h"
#include "aos/vision/image/file_stream.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
template<typename... T>
std::string Sprintf(const char* fmt, T&& ... args) {
  std::vector<char> buffer;
  buffer.resize(snprintf(NULL, 0, fmt, args...) + 1);
  snprintf(&buffer[0], buffer.size() + 1, fmt, args...);
  return std::string(&buffer[0], buffer.size() - 1);
}
#pragma GCC diagnostic pop

std::string asctime(const struct tm *timeptr, int ms) {
  static const char wday_name[][4] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char mon_name[][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  return Sprintf("%.3s %.3s%3d %.2d:%.2d:%.2d.%.3d %d",
    wday_name[timeptr->tm_wday],
    mon_name[timeptr->tm_mon],
    timeptr->tm_mday, timeptr->tm_hour,
    timeptr->tm_min, timeptr->tm_sec, ms,
    1900 + timeptr->tm_year);
}

std::string time_to_string(int64_t time) {
  time_t time_s = time / 1000;
  struct tm time_spec = *localtime(&time_s);
  return asctime(&time_spec, time % 1000);
}

namespace aos {
namespace vision {

//DEFINE_string(replay_fname, "/tmp/output", "File to record playback to. Empty string means no recording.");
DEFINE_int32(replay_speed, 128, "frames to play per second of frame");
DEFINE_int32(replay_ms_per_frame, 20, "ms_per_frame.");
DEFINE_int32(replay_start, 0, "frames to start at");
DEFINE_bool(start_unpaused, false, "start paused");

class FileReplayReader : public ImageReader {
 public:
  FileReplayReader(const std::string& fname) : stream_read(fname) {
    output_fname = fname;
  }

  void load_image(int i) {
    data_ref = stream_read.read_at(i);
  }

  ImageFormat get_format() override {
    load_image(0);
    return get_fmt(data_ref);
  }

  LoopEvent* get_read_event() override {
    return new TimeoutEvent(FLAGS_replay_ms_per_frame, [this] () {
      handle_timeout();
    });
  }

  void handle_timeout() {
    if (i > end_step) {
      review_mode = false;
    }
    if (i >= stream_read.size()) {
      i = stream_read.size() - 1;
      if (i < 0) {
        main_loop_quit();
        return;
      }
    }

    if (i != old_i) {
      printf("hey! %d at time: %s\n", i, time_to_string(stream_read.get_time(i)).c_str());
      load_image(i);
      stream->process((void *)data_ref.data(), data_ref.size(), i);
      old_i = i;
    }
    if (review_mode) {
      i += delta_controller;
    } else {
      i += delta;
    }
  }

  void read_cur_to(ImageValue* img) {
    process_jpeg(data_ref, img->data());
  }

  void set_stream(ImageStream* stream) {
    this->stream = stream;
    stream->key_press = [this] (unsigned int keyval) {
      switch(keyval) {
      case GDK_KEY_q:
        main_loop_quit();
        break;
      case GDK_KEY_p:
        if (delta != 0) {
          delta = 0;
        } else {
          delta = delta_controller;
        }
        break;
      case GDK_KEY_h:
        printf("Key press directory:\n");
        printf(" q -> quit\n");
        printf(" p -> pause\n");
        printf(" h -> help_message\n");
        printf(" b -> sets begin mark to current index\n");
        printf(" e -> sets end mark to current index\n");
        printf(" r -> reviews begin mark to end mark at current speed\n");
        printf(" s -> saves begin mark to end mark at current speed\n");
        printf("      to a file specified from the command line\n");
        printf(" up arrow -> increases current speed by 2x (currently %d)\n", delta_controller);
        printf(" down arrow -> decreases current speed by 2x (currently %d)\n", delta_controller);
        printf(" right arrow -> increases in time\n");
        printf(" left arrow -> decreases in time\n");
        break;
      case GDK_KEY_b:
        begin_step = i;
        printf("set begin to: %d\n", i);
        break;
      case GDK_KEY_e:
        end_step = i;
        printf("set end to: %d\n", i);
        break;
      case GDK_KEY_r:
        review_mode = !review_mode;
        if (review_mode) {
          if (begin_step != -1 && end_step != -1) {
            i = begin_step;
          }
        }
        break;
      case GDK_KEY_s:
        {
        printf("please enter output postfix (on command line):\n");
        if (begin_step == -1 || end_step == -1) {
          printf("please set begin and end before saving\n");
          break;
        }
        std::string image_postfix;
        std::getline(std::cin, image_postfix);
        if (image_postfix.size() == 0) {
          printf("empty postfix not accepted because that would imply file override\n");
          break;
        }
        std::cout << "full output name is: " << output_fname << image_postfix << "\n";

        JpegStreamWriter outfile(output_fname + image_postfix);

        for (int i = begin_step; i <= end_step; i += delta_controller) {
          outfile.WriteImage(stream_read.read_at(i), stream_read.get_time(i));
        }

        begin_step = end_step = -1;
        break;
        }
      case GDK_KEY_Up:
        delta_controller *= 2;
        printf("speed changed to: %d\n", delta_controller);
        break;
      case GDK_KEY_Down:
        if (delta_controller > 1) {
          delta_controller /= 2;
          printf("speed changed to: %d\n", delta_controller);
        }
        break;
      case GDK_KEY_Right:
        i += delta_controller;
        if (i >= stream_read.size()) {
          i = stream_read.size() - 1;
        }
        break;
      case GDK_KEY_Left:
        i -= delta_controller;
        if (i < 0) { i = 0; }
        break;
      }
    };
  }

  int begin_step = -1;
  int end_step = -1;
  
  bool review_mode = false;

  int delta_controller = FLAGS_replay_speed > 0 ? FLAGS_replay_speed : 1;
  int delta = FLAGS_start_unpaused ? delta_controller : 0;
  int i = FLAGS_replay_start;
  int old_i = -1;
  DataRef data_ref;
  JpegStreamReader stream_read;
  std::string output_fname;
 public:
  ImageStream* stream;
};

bool hasEnding(const std::string& str, const std::string& postfix) {
  if (str.size() < postfix.size()) return false;
  return (0 == str.compare(str.size() - postfix.size(), 
                           postfix.size(), postfix));
}

void strip_ending(std::string* str, const std::string& postfix) {
  if (hasEnding(*str, postfix)) {
    *str = str->substr(0, str->size() - postfix.size());
  }
}

void do_main(const ImageStreamConfig& cfg_inp) {
  using namespace std;

  cout << numeric_limits<streamoff>::max() << endl;
  ImageStreamConfig cfg = cfg_inp;

  std::unique_ptr<FileReplayReader> reader(new FileReplayReader(cfg_inp.filename));

  ImageFormat fmt = reader->get_format();
  printf("%dx%d\n", fmt.w, fmt.h);
  ImageValue base_img(fmt);
  auto base_ptr = base_img.img_ptr();
//reader->load_image(200);
//  reader->read_cur_to(&(base_img));

  cfg.make_reader = [&](ImageStream* stream) {
    reader->set_stream(stream);
    return std::move(reader);
  };
  
  ImageStream stream(cfg);
  int img_num = 0;
  //*
  stream.vision_cb = [&](const ImagePtr& img) {
    if (img_num % 50 == 0) base_ptr.CopyFrom(img);
    auto a = base_ptr.begin();
    auto b = img.begin();
    for (; b != img.end(); ++b) {
      //PixelRef& ref = *b;
      /*
      ref.r -= a->r;
      ref.g -= a->g;
      ref.b -= a->b;
      ref.r += 127;
      ref.g += 127;
      ref.b += 127;
      */
      ++a;
    }
    for (int j = 1; j < img.fmt.h - 1; j++) {
      for (int i = 1; i < img.fmt.w - 1; i++) {

      }
    }
    ++img_num;
  };
    
    /*
    HBoxFilter(img, 10);
    VBoxFilter(img, 10);
    HBoxFilter(img, 10);



    {
    bool last_val = true;
    int steps = 0;
    int total = 0;
    for (PixelRef& ref : img) {
      bool cur_val = true;
      if ((ref.r < 137 && ref.r >= 117) && 
          (ref.g < 137 && ref.g >= 117) &&
          (ref.b < 137 && ref.b >= 117)) {
          ref = {0, 0, 0};
          cur_val = false;
      } else {
          total += 3;
      }
      if (cur_val != last_val) {
        steps ++;
      }
      last_val = cur_val;
    }
    printf("steps: %d, total: %d\n", steps, total);
    }



    int hist[256];
    for (PixelRef& ref : img) {
      ++hist[ref.r];
      ++hist[ref.g];
      ++hist[ref.b];
    }
    */
  /*
*/
  auto view = GtkView(&stream);

  printf("hey! \n");
  stream.run();
}

}  // namespace vision
}  // namespace aos

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  aos::vision::ImageStreamConfig config;
  auto argv_copy = (const char**)argv;
  config.ParseFromArgv(&argc, &argv_copy);
  aos::vision::strip_ending(&config.filename, ".img_");
  aos::vision::strip_ending(&config.filename, ".img_s");
  aos::vision::strip_ending(&config.filename, ".img_index");
  config.LogConfig();
  aos::vision::do_main(config);
  return 0;
}
