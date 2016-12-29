#include "serial.h"
#include "dashboard/sensors.q.h"

#include <iostream>
#include <stdlib.h>

namespace dashboard {
namespace sensor_reader {
namespace {
char const* kPidCodeRpm = "0C";
char const* kPidCodeMph = "0D";
char const* kPidCodeCoolantTemp = "05";
}  // namespace

class SensorReader {
 public:
  SensorReader();

 private:
  void Reset();
  void ProcessData();
  void WriteDataToQueue(char*);
  int ProcessHexData(char const*);

  ::dashboard::sensor_reader::serial::Serial serial_;
};

}  // namespace sensor_reader
}  // namespace dashboard
