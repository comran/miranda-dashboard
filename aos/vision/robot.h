#ifndef AOS_VISION_ROBOT_H_
#define AOS_VISION_ROBOT_H_

#include <functional>
#include <memory>
#include <thread>

#include "aos/vision/events/udp.h"

namespace aos {
namespace vision {

struct msg_data {
  double left_encoder;
  double right_encoder;
};

class RobotInterface { 
 public:
  RobotInterface();

  void SendDrivetrain(bool enabled, uint8_t stick_coded, uint8_t wheel_coded);

  msg_data GetEncoders();

  std::unique_ptr<std::thread> start_watcher(std::function<void()> cb);

  int get_wait_fd() { return rx.get_fd(); }

 private:
  int packet_count;
  RXUdpSocket rx;
  TXUdpSocket tx;
};

}  // namespace vision
}  // namespace aos

#endif  // AOS_VISION_ROBOT_H_
