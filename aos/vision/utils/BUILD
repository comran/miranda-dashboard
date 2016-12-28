#!/usr/bin/python3
package(default_visibility = ["//visibility:public"])

cc_library(name = "bresenham",
    hdrs = ["bresenham.h"],
    srcs = ["bresenham.cc"],
    deps = [
        "//aos/vision/image:image_types", 
        "//aos/vision/blob:range_image",
        "//third_party/eigen",
      ])
