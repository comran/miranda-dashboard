#include "aos/common/time.h"

#include "aos/vision/image/reader.h"
#include "aos/common/logging/logging.h"

void process_save_file(void *buf, size_t len, uint32_t seqnum) {
  LOG(DEBUG, "got img: %d of size(%d)\n", seqnum, (int)len);
  char fname[1024];
  snprintf(fname, sizeof(fname), "/tmp/imgs/img%d.jpg", seqnum);
  printf("writing out to %s\n", fname);
  int f = open(fname, O_WRONLY | O_CREAT | O_TRUNC,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  printf("err: %s\n", strerror(errno));
  int var = write(f, buf, len);
  printf("hey!: %d\n", var);
  close(f);
}

namespace camera {

struct Reader::Buffer {
  void *start;
  size_t length; // for munmap
};

Reader::Reader(const char *dev_name, process_cb process, CameraParams params)
    : dev_name_(dev_name), process_(process), params_(params) {
  struct stat st; 
  if (stat(dev_name, &st) == -1) {
    LOG(FATAL, "Cannot identify '%s' because of %d: %s\n",
        dev_name, errno, strerror(errno));
  }
  if (!S_ISCHR(st.st_mode)) {
    LOG(FATAL, "%s is no device\n", dev_name);
  }

  fd_ = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
  if (fd_ == -1) {
    LOG(FATAL, "Cannot open '%s' because of %d: %s\n",
        dev_name, errno, strerror(errno));
  }

  Init();
}

void Reader::QueueBuffer(v4l2_buffer *buf) {
  if (xioctl(fd_, VIDIOC_QBUF, buf) == -1) {
    LOG(WARNING, "ioctl VIDIOC_QBUF(%d, %p) failed with %d: %s."
        " losing buf #%" PRIu32 "\n",
        fd_, &buf, errno, strerror(errno), buf->index);
  } else {
//    LOG(DEBUG, "put buf #%" PRIu32 " into driver's queue\n", buf->index);
    ++queued_;
  }
}

int64_t timespec_to_us(struct timeval tv) {
  return ((int64_t)tv.tv_sec) * ((int64_t)1000000) + tv.tv_usec;
}

int64_t timespec_to_us(struct timespec tv) {
  return ((int64_t)tv.tv_sec) * ((int64_t)1000000) + tv.tv_nsec / 1000;
}

int64_t getEpochTimeShift(){
  struct timeval epochtime;
  struct timespec  vsTime;

  gettimeofday(&epochtime, NULL);
  clock_gettime(CLOCK_MONOTONIC, &vsTime);

  return timespec_to_us(epochtime) - timespec_to_us(vsTime);
}

int64_t toEpochOffset_ms = getEpochTimeShift();

void Reader::ReadFrame() {
  v4l2_buffer buf;
  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (xioctl(fd_, VIDIOC_DQBUF, &buf) == -1) {
    if (errno != EAGAIN) {
      LOG(ERROR, "ioctl VIDIOC_DQBUF(%d, %p) failed with %d: %s\n",
          fd_, &buf, errno, strerror(errno));
    }
    return;
  }
  --queued_;

  // Get a timestamp now as proxy for when the image was taken
  // TODO(ben): the image should come with a timestamp, parker
  // will know how to get it.
  int64_t time = aos::time::Time::Now().ToNSec();

  process_(buffers_[buf.index].start, buf.bytesused, time);
  QueueBuffer(&buf);
}


void Reader::mmap_buffs() {

  buffers_ = new Buffer[kNumBuffers];
  v4l2_buffer buf;
  for (unsigned int n = 0; n < kNumBuffers; ++n) {
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n;
    if (xioctl(fd_, VIDIOC_QUERYBUF, &buf) == -1) {
      LOG(FATAL, "ioctl VIDIOC_QUERYBUF(%d, %p) failed with %d: %s\n",
          fd_, &buf, errno, strerror(errno));
    }     
    buffers_[n].length = buf.length;
    buffers_[n].start = mmap(NULL, buf.length,
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd_, buf.m.offset);
    if (buffers_[n].start == MAP_FAILED) {
      LOG(FATAL, "mmap(NULL, %zd, PROT_READ | PROT_WRITE, MAP_SHARED, %d, %jd)"
          " failed with %d: %s\n", (size_t)buf.length, fd_, static_cast<intmax_t>(buf.m.offset),
          errno, strerror(errno)); 
    }     
  } 
}

void Reader::init_mmap() {
  v4l2_requestbuffers req;
  CLEAR(req);
  req.count = kNumBuffers;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (xioctl(fd_, VIDIOC_REQBUFS, &req) == -1) {
    if (EINVAL == errno) {
      LOG(FATAL, "%s does not support memory mapping\n", dev_name_);
    } else {
      LOG(FATAL, "ioctl VIDIOC_REQBUFS(%d, %p) failed with %d: %s\n",
          fd_, &req, errno, strerror(errno));
    }
  }
  queued_ = kNumBuffers;
  if (req.count < kNumBuffers) {
    LOG(FATAL, "Insufficient buffer memory on %s\n", dev_name_);
  }
}

// Sets one of the camera's user-control values.
// Prints the old and new values.
// Just prints a message if the camera doesn't support this control or value.
bool Reader::SetCameraControl(uint32_t id, const char *name, int value) {
  struct v4l2_control getArg = {id, 0U};
  int r = xioctl(fd_, VIDIOC_G_CTRL, &getArg);
  if (r == 0) {
    if (getArg.value == value) {
      printf("Camera control %s was already %d\n", name, getArg.value);
      return true;
    }
  } else if (errno == EINVAL) {
    printf("Camera control %s is invalid\n", name);
    errno = 0;
    return false;
  }

  struct v4l2_control setArg = {id, value};
  r = xioctl(fd_, VIDIOC_S_CTRL, &setArg);
  if (r == 0) {
    printf("Set camera control %s from %d to %d\n", name, getArg.value, value);
    return true;
  }

  printf("Couldn't set camera control %s to %d: %s (errno %d)\n",
         name, value, strerror(errno), errno);
  errno = 0;
  return false;
}

void Reader::Init() {
  v4l2_capability cap;
  if (xioctl(fd_, VIDIOC_QUERYCAP, &cap) == -1) {
    if (EINVAL == errno) {
      LOG(FATAL, "%s is no V4L2 device\n",
          dev_name_);
    } else {
      LOG(FATAL, "ioctl VIDIOC_QUERYCAP(%d, %p) failed with %d: %s\n",
          fd_, &cap, errno, strerror(errno));
    }
  }
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    LOG(FATAL, "%s is no video capture device\n",
        dev_name_);
  }
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    LOG(FATAL, "%s does not support streaming i/o\n",
        dev_name_);
  }

  /* Select video input, video standard and tune here. */

  v4l2_cropcap cropcap;
  CLEAR(cropcap);
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(fd_, VIDIOC_CROPCAP, &cropcap) == 0) {
    v4l2_crop crop;
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */

    if (xioctl(fd_, VIDIOC_S_CROP, &crop) == -1) {
      switch (errno) {
      case EINVAL:
        /* Cropping not supported. */
        break;
      default:
        /* Errors ignored. */
        break;
      }
    }
  } else {        
    /* Errors ignored. */
  }

  v4l2_format fmt;
  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = params_.width; 
  fmt.fmt.pix.height = params_.height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  //fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
  if (xioctl(fd_, VIDIOC_S_FMT, &fmt) == -1) {
    LOG(FATAL, "ioctl VIDIC_S_FMT(%d, %p) failed with %d: %s\n",
        fd_, &fmt, errno, strerror(errno));
  }
  /* Note VIDIOC_S_FMT may change width and height. */

  /* Buggy driver paranoia. */
  unsigned int min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  if (!SetCameraControl(V4L2_CID_EXPOSURE_AUTO,
                        "V4L2_CID_EXPOSURE_AUTO", V4L2_EXPOSURE_MANUAL)) {
    LOG(FATAL, "Failed to set exposure\n");
  }

  if (!SetCameraControl(V4L2_CID_EXPOSURE_ABSOLUTE,
                        "V4L2_CID_EXPOSURE_ABSOLUTE", params_.exposure)) {
    LOG(FATAL, "Failed to set exposure\n");
  }

  if (!SetCameraControl(V4L2_CID_BRIGHTNESS, "V4L2_CID_BRIGHTNESS", params_.brightness)) {
    LOG(FATAL, "Failed to set up camera\n");
  }

  if (!SetCameraControl(V4L2_CID_GAIN, "V4L2_CID_GAIN", params_.gain)) {
    LOG(FATAL, "Failed to set up camera\n");
  }

// #if 0
  // set framerate
  struct v4l2_streamparm *setfps;
  setfps = (struct v4l2_streamparm *) calloc(1, sizeof(struct v4l2_streamparm));
  memset(setfps, 0, sizeof(struct v4l2_streamparm));
  setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  setfps->parm.capture.timeperframe.numerator = 1;
  setfps->parm.capture.timeperframe.denominator = params_.fps;
  if (xioctl(fd_, VIDIOC_S_PARM, setfps) == -1) {
    LOG(ERROR, "ioctl VIDIOC_S_PARM(%d, %p) failed with %d: %s\n",
        fd_, setfps, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  LOG(INFO, "framerate ended up at %d/%d\n",
      setfps->parm.capture.timeperframe.numerator,
      setfps->parm.capture.timeperframe.denominator);
// #endif

  init_mmap();
  LOG(INFO, "Bat Vision Successfully Initialized.\n");
}

aos::vision::ImageFormat Reader::get_format() {
  struct v4l2_format fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(fd_, VIDIOC_G_FMT, &fmt) == -1) {
    LOG(FATAL, "ioctl VIDIC_G_FMT(%d, %p) failed with %d: %s\n",
        fd_, &fmt, errno, strerror(errno));
  }

  return aos::vision::ImageFormat { (int)fmt.fmt.pix.width, (int)fmt.fmt.pix.height };
}

void Reader::Start() {
  LOG(DEBUG, "queueing buffers for the first time\n");
  v4l2_buffer buf;
  for (unsigned int i = 0; i < kNumBuffers; ++i) {
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    QueueBuffer(&buf);
  }
  LOG(DEBUG, "done with first queue\n");

  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(fd_, VIDIOC_STREAMON, &type) == -1) {
    LOG(FATAL, "ioctl VIDIOC_STREAMON(%d, %p) failed with %d: %s\n",
        fd_, &type, errno, strerror(errno));
  }
}

void Reader::Run() {
  Start();
  mmap_buffs();

  fd_set fds;
  timeval tv;
  while (true) {
    // HAVE TO DO THIS EACH TIME THROUGH THE LOOP
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(fd_, &fds);
    //FD_SET(server_fd_, &fds);
  //switch (select(std::max(fd_, server_fd_) + 1, &fds, NULL, NULL, &tv)) {
    switch (select(fd_ + 1, &fds, NULL, NULL, &tv)) {
    case -1:
      if (errno != EINTR) {
        LOG(ERROR, "select(%d, %p, NULL, NULL, %p) failed with %d: %s\n",
            //std::max(fd_, server_fd_) + 1, &fds, &tv, errno, strerror(errno));
            fd_ + 1, &fds, &tv, errno, strerror(errno));
      }
      continue;
    case 0:
      LOG(WARNING, "select timed out\n");
      continue;
    }

    if (FD_ISSET(fd_, &fds)) {
      LOG(DEBUG, "Got a frame\n");
      ReadFrame();
    }
#if 0
    if (FD_ISSET(server_fd_, &fds)) {
      const int sock = accept4(server_fd_, NULL, NULL, SOCK_NONBLOCK);
      if (sock == -1) {
        LOG(ERROR, "accept4(%d, NULL, NULL, SOCK_NONBLOCK(=%d) failed with %d: %s\n",
            server_fd_, SOCK_NONBLOCK, errno, strerror(errno));
      } else {
        SendFD(sock);
      }
    }
#endif
  }
}
//const char *const Reader::dev_name = "/dev/video0";

} // namespace camera


