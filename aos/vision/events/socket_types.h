#ifndef _AOS_VISION_EVENTS_SOCKET_TYPES_H_
#define _AOS_VISION_EVENTS_SOCKET_TYPES_H_

#include <stdint.h>
#include <poll.h>

#include "aos/vision/image/image_types.h"
#include "aos/vision/events/tcp_server.h"
#include "google/protobuf/message.h"

namespace aos {
namespace events {

using namespace vision;

class BufferedLengthDelimReader {
 public:
  union data_len {
    uint32_t len;
    char buf[4];
  };

  BufferedLengthDelimReader() {
    num_read_ = 0;
    img_read_ = -1;
  }

  template <typename Lamb>
  void up(events::ReadEpollEvent::ReadContext *ctx, Lamb lam) {
    ssize_t count;
    if (img_read_ < 0) {
      count = ctx->Read(&len_.buf[num_read_], sizeof(len_.buf) - num_read_);
      if (count < 0) return;
      num_read_ += count;
      if (num_read_ < 4) return;
      num_read_ = 0;
      img_read_ = 0;
      data_.clear();
      if (len_.len > 200000) {
        printf("bad size: %d\n", len_.len);
        exit(-1);
      }
      data_.resize(len_.len);
    } else {
      count = ctx->Read(&data_[img_read_], len_.len - img_read_);
      if (count < 0) return;
      img_read_ += count;
      if (img_read_ < (int)len_.len) return;
      lam(DataRef{&data_[0], static_cast<int>(len_.len)});
      img_read_ = -1;
    }
  }

 private:
  data_len len_;
  int num_read_;

  std::vector<char> data_;
  int img_read_;
};

class DataSocket : public events::SocketConn {
 public:

  union data_len {
    uint32_t len;
    char buf[4];
  };

  DataSocket(events::TCPServerBase* serv, int fd)
      : events::SocketConn(serv, fd) {
  }

  ~DataSocket() { printf ("Closed connection on descriptor %d\n", fd()); }

  void ReadEvent(ReadContext* ctx) override {
    // ignore reads
    ssize_t count;
    char buf[512];
    count = ctx->Read(buf, sizeof buf);
    if (count <= 0) return;
  }
  
  void Emit(const google::protobuf::Message& data) {
    std::string d;
    if (data.SerializeToString(&d)) {
      Emit(d);
    }
  }
  
  void Emit(const std::string& data) {
    DataRef datRef;
    datRef.Set(data.data(), data.size());
    Emit(datRef);
  }

  void Emit(DataRef data) {
    data_len len;
    len.len = data.size();
    int res = write(fd(), len.buf, sizeof len.buf);
    if (res == -1) {
      printf("Emit Error on write\n");
    }
    int write_count = 0;
    while (write_count < data.size()) {
      int len = write(fd(), &data.data()[write_count], data.size() - write_count);
      if (len == -1) {
        if (errno == EAGAIN) {
          struct pollfd waiting;
          waiting.fd = fd();
          waiting.events = POLLOUT; 
          poll(&waiting, 1, -1);
        } else {
          close(fd());
          return;
        }
      } else {
        write_count += len;
      }
      if (write_count != data.size()) printf("wrote: %d\n", len);
    }
  }
};

}  // namespace events
}  // namespace aos

#endif  // _AOS_VISION_EVENTS_SOCKET_TYPES_H_
