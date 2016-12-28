#include <iostream>
#include <stdlib.h>
#include "sensor_reader.h"

namespace dashboard {
namespace sensor_reader {

SensorReader::SensorReader() {
  serial_.Init();

  serial_.Write("atz");
  if (serial_.WaitForResponse("ELM327 v1.5", 10)) {
    ::std::cout << "Connected to interface." << ::std::endl;
  }

  serial_.Write("at@1");
  if (serial_.WaitForResponse("OBDII to RS232 Interpreter", 10)) {
    ::std::cout << "Verified interface. (step 1)" << ::std::endl;
  }

  serial_.Write("at@2");
  if (serial_.WaitForResponse("?", 10)) {
    ::std::cout << "Verified interface. (step 2)" << ::std::endl;
  }

  // Keep trying to ping the ECU until it is detected.
  bool ecu_detected = false;
  while (!ecu_detected) {
    serial_.Write("0100");

    char* response = new char[500];

    for (int i = 0; i < 10; i++) {
      serial_.Read(response);
      ::std::cout << "RESPONSE: " << response << ::std::endl;
      if (!strcmp(
              response,
              "CONNECTED")) {  // TODO(comran): See what actually goes here...
        ecu_detected = true;
        break;
      } else if (!strcmp(
              response,
              "UNABLE TO CONNECT")) {
        break;
      }
    }
  }
  ::std::cout << "Detected ECU." << ::std::endl;

  serial_.Write("010C");
  if (serial_.WaitForResponse(".........", 10)) {
    ::std::cout << "Matched RPM." << ::std::endl;
  }
}

}  // namespace sensor_reader
}  // namespace dashboard
