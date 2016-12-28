#include "sensor_reader.h"

namespace dashboard {
namespace sensor_reader {

SensorReader::SensorReader() {
  serial_.Init();
  serial_.Write("atz");
}

}  // namespace sensor_reader
}  // namespace dashboard
