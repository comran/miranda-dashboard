#include "aos/vision/image/image_stream_legacy.h"
#include "eigen3/Eigen/Dense"

#include <limits>
#include <math.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <map>
#include <iostream>

#include "aos/vision/blobs.h"
#include "aos/vision/comp_geo/vector.h"

namespace aos {
namespace vision {

using namespace comp_geo;

class RobotModel {
 public:
  RobotModel(double wheelbase, double max_velocity)
      : wheelbase_(wheelbase), max_vel_(max_velocity), pos_(0, 0, 0) {}
  ~RobotModel() {}

  // accelerate each wheel in m/s^2
  void Accelerate(double left_acc, double right_acc) {
    left_vel_ += left_acc;
    if (left_vel_ > max_vel_) {
      left_vel_ = max_vel_;
    } else if (left_vel_ < -max_vel_) {
      left_vel_ = -max_vel_;
    }
    right_vel_ += right_acc;
    if (right_vel_ > max_vel_) {
      right_vel_ = max_vel_;
    } else if (right_vel_ < -max_vel_) {
      right_vel_ = -max_vel_;
    }
  }

  // tick time by applying wheel velocity to position
  void Tick() {
    // change in angle is differential velocity scaled by time
    // over the wheel base
    double d_theta = std::tan(dt_ * (right_vel_ - left_vel_) / wheelbase_);
    pos_.z(pos_.z() + d_theta);
    // robot velocity is the average of the wheels
    double velocity = (left_vel_ + right_vel_) / 2.0;
    // project velocity to the axis and scale to time
    double x = dt_ * std::cos(pos_.z()) * velocity;
    double y = dt_ * std::sin(pos_.z()) * velocity;
    // rotate by theta
    double xp = x * std::cos(pos_.z()) - y * std::sin(pos_.z());
    double yp = x * std::sin(pos_.z()) + y * std::cos(pos_.z());
    // update position
    pos_.x(pos_.x() + xp);
    pos_.y(pos_.y() + yp);
  }

  // distance from left to right wheel in meters
  double wheelbase_;
  // as fast as the robot can go
  double max_vel_;
  // velocity of the left wheel in m/s
  double left_vel_ = 0.0;
  // velocity of right wheel in m/s
  double right_vel_ = 0.0;
  // robot field position (x, y, theta)
  Vector<3> pos_;
  // time step in seconds
  double dt_ = 0.005;
};

class BlankReplayReader : public ImageReader {
 public:
  BlankReplayReader(ImageFormat fmt) : fmt_(fmt) {
  }

  ImageFormat get_format() {
    return fmt_;
  }

  ReadEvent* get_read_event() {
    return NULL;
  }

 private:
  ImageFormat fmt_;
};

void do_main(const ImageStreamConfig& cfg_inp) {
  ImageStreamConfig cfg = cfg_inp;

  ImageFormat fmt {1280, 960};
  std::unique_ptr<BlankReplayReader> reader(new BlankReplayReader(fmt));

  cfg.make_reader = [&](ImageStream* /*stream*/) {
    return std::move(reader);
  };

  ImageStream stream(cfg);
  printf("fmt: %dx%d\n", stream.fmt.w, stream.fmt.h);
  auto view = GtkView(&stream);

  // draw the origin so we know where things are
  LinesOverlay* origin = view->add_line_overlay();
  origin->color = {255, 0, 0};
  // parker I don't know what the scale factor here is for
  origin->scale = 2 / 0.0254;

  // draw waypoints as a collection of circles
  CircleOverlay* waypoints = view->add_circle_overlay();
  waypoints->color = {0, 255, 255};
  waypoints->scale = 2 / 0.0254;
  
  // draw robot path by adding new lines
  LinesOverlay* path = view->add_line_overlay();
  path->color = {0, 255, 0};
  path->scale = 2 / 0.0254;

  // model of the robot on the field
  // I don't have it but I guess the base is 15cm wide
  // and it can travel about a quarter as fast as the real one
  RobotModel robot(0.15, 4.2);

  origin->add_line(Vector<2>(0.0, -0.2), Vector<2>(0.0, 0.2));
  origin->add_line(Vector<2>(-0.2, 0.0), Vector<2>(0.2, 0.0));
  
  waypoints->add_circle(Vector<2>(1.0, 1.0), 0.1);
  waypoints->add_circle(Vector<2>(0.0, 2.0), 0.1);

  for (int i = 0; i < 176; i++) {
    // complicated navigation algorithm:
    // speed up and turn a little
    // I'm tired, best I can do tonight.
    robot.Accelerate(0.02, 0.0215);
    robot.Tick();
    path->add_point(Vector<2>(robot.pos_.x(), robot.pos_.y()));
  }

  stream.refresh_rate = 1;
  stream.run();
}

}  // namespace vision
}  // namespace aos

int main(int argc, const char **argv) {
  aos::vision::ImageStreamConfig config;
  config.ParseFromArgv(&argc, &argv);
  config.LogConfig();
  aos::vision::do_main(config);
  return 0;
}
