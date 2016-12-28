#!/usr/bin/python
package(default_visibility = ["//visibility:public"])

cc_test(
    name = "basic_image_test",
    srcs = ["basic_image_test.cc"],
    deps = [
        ":image_types",
        '//aos/testing:googletest',
        ":jpeg_routines",
        ":reader",
        ":image_stream_legacy"],
)

cc_library(name = "file_stream",
    hdrs = ["file_stream.h",
        ":image_types"
        ])

cc_library(
    name = "events",
    srcs = ["events.cc"],
    hdrs = ["events.h"],
    deps = [
        '@usr_repo//:gtk+-3.0',
        ],
)
cc_library(
    name = "image_stream_legacy",
    srcs = ["image_stream_legacy.cc"],
    hdrs = ["image_stream_legacy.h"],
    deps = [
        ":events",
        ":jpeg_routines",
        ":reader",
        ":file_stream",
        ":image_types",
        '//third_party/gflags',
        '//aos/vision/comp_geo:segment',
        '//aos/vision/comp_geo:vector',
        '//aos/vision/debug:overlay',
    ],
)

cc_library(
    name = "image_types",
    hdrs = ["image_types.h"]
)

cc_library(
    name = 'reader',
    srcs = ['reader.cc'],
    hdrs = ['V4L2.h', 'reader.h'],
    deps = [
      "//aos/common:time",
      "//aos/common/logging:logging",
      ":image_types",
      ],
)

cc_library(
    name = 'jpeg_routines',
    srcs = ['jpeg_routines.cc'],
    hdrs = ['jpeg_routines.h'],
    deps = [
        '//third_party/libjpeg',
        ':image_types'
        ],
)

cc_library(name = "image_stream",
    hdrs = ["image_stream.h"],
    deps = [
        "//aos/vision/events:epoll_events",
        "//aos/vision/image:reader"
    ]
)

