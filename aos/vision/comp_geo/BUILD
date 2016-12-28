#!/usr/bin/python
package(default_visibility = ["//visibility:public"])

#cc_library(
#    name = "transformation",
#    srcs = ["transformation.cc"],
#    hdrs = ["transformation.h"],
#)

cc_library(
    name = "vector",
    hdrs = ["vector.h"],
    deps = ["//third_party/eigen"])

cc_library(
    name = "segment",
    hdrs = ["segment.h"],
    deps = [":vector"])

cc_library(
    name = "convexhull",
    srcs = ["convexhull.cc"],
    hdrs = ["convexhull.h"],
    deps = [':vector'],
)

cc_test(
    name = "basic_math_test",
    srcs = [
        "basic_math_test.cc",
        "vector.h",
        "ray.h",
        "segment.h",
        "matrix4x4.h"],
    deps = ['//aos/testing:googletest'],
)

