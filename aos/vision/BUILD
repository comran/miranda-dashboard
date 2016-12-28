#!/usr/bin/python3
package(default_visibility = ["//visibility:public"])

cc_binary(name = "capture",
          srcs = ["capture.cc"],
          deps = [
              "//aos/vision/image:image_stream_legacy",
              ":blobs"
              ])

cc_binary(name = "replay",
          srcs = ["replay.cc"],
          deps = [
              "//aos/vision/image:image_stream_legacy",
              ":blobs"
              ])

cc_binary(name = "stream_snip",
          srcs = ["stream_snip.cc"],
          deps = [
              "//aos/vision/image:image_stream_legacy",
              ":blobs"
              ])

cc_library(name = "region_alloc",
    srcs = [],
    hdrs = ["region_alloc.h"])

cc_library(name = "robot",
    srcs = ["robot.cc"],
    hdrs = ["robot.h"],
    deps = ["//aos/vision/events:udp"],
    )

cc_library(name = "checker_board",
    srcs = ["checker_board.cc"],
    hdrs = ["checker_board.h"],
    deps = ["//aos/vision/image:image_types",
      "//aos/vision/comp_geo:vector",
      "//aos/vision/comp_geo:segment",
      ":blobs",
      ":region_alloc",
      ])

cc_binary(name = "image_viewer",
    srcs = ["image_viewer.cc"],
    copts = ['-Wno-unused-parameter'],
    deps = [
        ":robot",
        "//aos/vision/image:image_stream_legacy",
        ":blobs",
        ":checker_board",
        ],
    )

cc_binary(name = "path_demo",
    srcs = ["path_demo.cc"],
    deps = [
        "//aos/vision/image:image_stream_legacy",
        ":blobs",
        ":region_alloc",
        ])
