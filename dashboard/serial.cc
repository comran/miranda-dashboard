#include "serial.h"

#include <iostream>

namespace dashboard {
namespace sensor_reader {
namespace serial {

void Serial::Init() {
  serial_fd_ = open(port_name_, O_RDWR | O_NOCTTY | O_NDELAY);

  if (serial_fd_ == -1) {
    ::std::cerr << "ERROR: Could not open file descriptor." << ::std::endl;
  }

  struct termios serial_options;
  memset(&serial_options, 0, sizeof serial_options);
  if (tcgetattr(serial_fd_, &serial_options) != 0) {
    ::std::cerr << "ERROR: " << errno << " from tcgetattr: " << strerror(errno)
                << ::std::endl;
  }

  cfsetispeed(&serial_options, (speed_t)B38400);
  serial_options.c_cflag &= ~PARENB;  // Make 8n1
  serial_options.c_cflag &= ~CSTOPB;
  serial_options.c_cflag &= ~CSIZE;
  serial_options.c_cflag |= CS8;

  serial_options.c_cflag &= ~CRTSCTS;        // no flow control
  serial_options.c_cc[VMIN] = 1;             // read doesn't block
  serial_options.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
  serial_options.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
  cfmakeraw(&serial_options);

  tcflush(serial_fd_, TCIFLUSH);
  if (tcsetattr(serial_fd_, TCSANOW, &serial_options) != 0) {
    ::std::cerr << "ERROR: " << errno << " from tcsetattr" << ::std::endl;
  }
}

void Serial::Write(char const* command_parameter) {
  if (serial_fd_ != -1) {
    int len = strchr(command_parameter, '\0') - command_parameter;

    // Put a CR before the end character.
    char* command = (char*)malloc((len + 1) * sizeof(char));
    strcpy(command, command_parameter);
    // TODO(comran): Check this... (two lines below)
    command[len] = '\r';
    command[len + 1] = '\0';

    // Write command to serial.
    int count = write(serial_fd_, command, strlen(command));
    if (count < 0) {
      ::std::cerr << "ERROR: Unexpected count returned.";
    }
  }
}

// Read a line from UART.
// Return a 0 len string in case of problems with UART
// http://stackoverflow.com/questions/18108932/linux-c-serial-port-reading-writing
void Serial::Read(char *&buffer) {
  char c;
  char* b = buffer;
  int rx_length = -1;
  while (1) {
    rx_length = read(serial_fd_, (void*)(&c), 1);

    if (rx_length <= 0) {
      // wait for messages
      usleep(100);
    } else {
      if (c == '\r') {
        *b++ = '\0';
        break;
      }
      *b++ = c;
    }
  }
}

bool Serial::WaitForResponse(char const* expected_response, int max_iterations) {
  char* response = new char[500];

  for(int i = 0;i < max_iterations;i++){
    Read(response);
    ::std::cout << "RESPONSE: " << response << ::std::endl;
    if(!strcmp(response, expected_response)) {
      return true;
    }
  }

  return false;
}

void Serial::Close(void) { close(serial_fd_); }

}  // namespace dashboard
}  // namespace sensor_reader
}  // namespace serial
