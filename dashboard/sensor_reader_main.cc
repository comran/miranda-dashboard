#include "sensor_reader.h"

#include "aos/linux_code/init.h"

int main(int, char**) {
  ::aos::Init();
  ::dashboard::sensor_reader::SensorReader sensor_reader;
  ::aos::Cleanup();
  return 0;
}
