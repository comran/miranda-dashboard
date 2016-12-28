#include "aos/vision/robot.h"

namespace aos {
namespace vision {

RobotInterface::RobotInterface() {
  tx.Connect("10.89.71.2", 1110);
  rx.Bind(1250);
  packet_count = 0;
}

msg_data RobotInterface::GetEncoders() {
  msg_data data;
  rx.Recv((char*)&data, sizeof(msg_data));
  return data;
}

void RobotInterface::SendDrivetrain(bool enabled, uint8_t stick_coded, uint8_t wheel_coded) {
  wheel_coded = -wheel_coded;
  enabled = enabled && packet_count > 0;
  uint8_t enable_coded = enabled ? 0x4 : 0;
	uint8_t ds_packet[] = {(uint8_t)((packet_count >> 8) & 0xff), (uint8_t)(packet_count & 0xff),
    1, enable_coded, 16, 5, 12, 12, 4, wheel_coded, 0, 0, 0, 13, 0, 0, 1, 255, 255, 9, 12, 3, 0, stick_coded, 127, 11, 0, 0, 0, 9, 12, 3, 127, 0, 217, 11, 0, 0, 0, 12, 12, 4, 255, 128, 127, 128, 12, 0, 0, 1, 255, 255};
  tx.Send((char*)ds_packet, sizeof(ds_packet));
  ++packet_count;
}

std::unique_ptr<std::thread> RobotInterface::start_watcher(std::function<void()> cb) {
  std::unique_ptr<std::thread> thread(new std::thread([cb] {
    while(true) {
      cb();
    }
  }));

  return std::move(thread);
}

}  // namespace vision
}  // namespace aos
