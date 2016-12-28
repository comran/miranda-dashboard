#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <inttypes.h>
#include <string.h>

namespace dashboard {
namespace sensor_reader {
namespace serial {
class Serial {
 public:
  void Init();
  void Write(char const* command);
  void Read(char const*);
  void Close(void);

 private:
  int serial_fd_ = -1;
  char const* port_name_ = "/dev/ttyUSB0";
};

}  // namespace dashboard
}  // namespace sensor_reader
}  // namespace serial
#endif
