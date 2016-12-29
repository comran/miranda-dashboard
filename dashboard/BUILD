package(default_visibility = ['//visibility:public'])

load('/aos/build/queues', 'queue_library')

queue_library(
  name = 'sensors_queue',
  srcs = [
    'sensors.q'
  ],
)

cc_library(
  name = 'serial',
  visibility = ['//visibility:public'],
  srcs = [
    'serial.cc',
  ],
  hdrs = [
    'serial.h',
  ],
)

cc_library(
  name = 'sensor_reader_lib',
  visibility = ['//visibility:public'],
  srcs = [
    'sensor_reader.cc',
  ],
  hdrs = [
    'sensor_reader.h',
  ],
  deps = [
    ':sensors_queue',
    ':serial',
    '//aos/linux_code:init',
  ],
)

cc_binary(
  name = 'sensor_reader',
  visibility = ['//visibility:public'],
  srcs = [
    'sensor_reader_main.cc',
  ],
  deps = [
    'sensor_reader_lib',
  ],
)
