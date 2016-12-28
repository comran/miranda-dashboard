#import "serial.h"

namespace dashboard {
namespace sensor_reader {

class SensorReader {
 public:
  SensorReader();
 private:
  ::dashboard::sensor_reader::serial::Serial serial_;
};

}  // namespace sensor_reader
}  // namespace dashboard
