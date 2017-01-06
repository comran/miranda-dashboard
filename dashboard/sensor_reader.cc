#include "sensor_reader.h"
#include <sstream>
#include "stdio.h"

namespace dashboard {
namespace sensor_reader {

SensorReader::SensorReader() {
  Reset();

  while (true) {
    SendPidCode(kPidCodeRpm);
    SendPidCode(kPidCodeMph);
    SendPidCode(kPidCodeCoolantTemp);

    ::dashboard::sensors_queue.FetchLatest();
    ::std::cout << "RPM: " << ::std::setw(15) << ::dashboard::sensors_queue->rpm
                << " MPH: " << ::std::setw(15)
                << ::dashboard::sensors_queue->mph
                << " Coolant temp: " << ::std::setw(15)
                << ::dashboard::sensors_queue->coolant_temp << ::std::endl;
  }
}

void SensorReader::Reset() {
  serial_.Init();
  serial_.Write("atz");
  if (serial_.WaitForResponse("ELM327 v1.5", 10)) {
    ::std::cout << "Connected to interface." << ::std::endl;
  }

  serial_.Write("at@1");
  if (serial_.WaitForResponse("OBDII to RS232 Interpreter", 10)) {
    ::std::cout << "Verified interface. (step 1)" << ::std::endl;
  }

  serial_.Write("at0");
  if (serial_.WaitForResponse("?", 10)) {  // TODO(comran): Update this.
    ::std::cout << "Verified interface. (step 2)" << ::std::endl;
  }

  // Keep trying to ping the ECU until it is detected.
  bool ecu_detected = false;
  while (!ecu_detected) {
    serial_.Write("0100");
    serial_.WaitForResponse(">0100", 10);
    ::std::cout << "Got echo." << ::std::endl;
    serial_.WaitForResponse("SEARCHING...", 10);
    ::std::cout << "Got search notification." << ::std::endl;

    char* response = new char[500];

    for (int i = 0; i < 3; i++) {
      serial_.Read(response);
      if (!strcmp(response, "UNABLE TO CONNECT")) {
        break;
      }
    }

    ecu_detected = true;  // TODO(comran): Detect this better.
  }

  ::std::cout << "Detected ECU." << ::std::endl;
}

void SensorReader::SendPidCode(char const* pid_code) {
  // Prepend the read code to the pid that we want.
  char* pid = new char[500];
  strcat(pid, "01");
  strcat(pid, pid_code);
  serial_.Write(pid);

  ProcessData();
}

void SensorReader::ProcessData() {
  char* response = new char[500];

  serial_.Read(response);  // Read the result.
  if (response[0] == '>') {
    serial_.Read(response);
    WriteDataToQueue(response);
  }
}

void SensorReader::WriteDataToQueue(char* data) {
  int parsed_data = ProcessHexData(data);
  char* id = new char[500];
  strncpy(id, data + 3, 2);
  id[2] = '\0';

  static double last_mph = 0, last_rpm = 0, last_coolant_temp = 0;
  if (::dashboard::sensors_queue.FetchLatest()) {
    last_rpm = ::dashboard::sensors_queue->rpm;
    last_mph = ::dashboard::sensors_queue->mph;
    last_coolant_temp = ::dashboard::sensors_queue->coolant_temp;
  }

  if (!strcmp(id, kPidCodeRpm)) {
    last_rpm = parsed_data / 4.0;
  } else if (!strcmp(id, kPidCodeMph)) {
    last_mph = parsed_data / 1.609;
  } else if (!strcmp(id, kPidCodeCoolantTemp)) {
    last_coolant_temp = (parsed_data - 40) * 9.0 / 5.0 + 32.0;
  } else {
    ::std::cerr << "ERROR: Unknown id. (" << id << ")" << ::std::endl;
    return;
  }
    ::dashboard::sensors_queue.MakeWithBuilder()
        .rpm(last_rpm)
        .mph(last_mph)
        .coolant_temp(last_coolant_temp)
        .Send();
}

int SensorReader::ProcessHexData(char const* response_parameter) {
  char* response = new char[500];
  strcpy(response, response_parameter);
  response += 6;  // Cut off the identifier before the actual number we want.

  // Cut out all the spaces.
  for (size_t i = 0; i < strlen(response); i++) {
    if (response[i] == ' ') {
      for (size_t j = i; j < strlen(response) - 1; j++) {
        response[j] = response[j + 1];
      }
      response[strlen(response) - 1] = '\0';
    }
  }

  return (int)strtol(response, NULL, 16);
}


}  // namespace sensor_reader
}  // namespace dashboard
