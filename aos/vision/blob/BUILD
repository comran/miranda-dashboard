#!/usr/bin/python3
package(default_visibility = ["//visibility:public"])

cc_library(name = "range_image",
  hdrs = ["range_image.h"],
  srcs = ["range_image.cc"],
  deps = [
      "//aos/vision/comp_geo:vector",
      "//aos/vision/debug:overlay",
      "//aos/vision/comp_geo:segment",
      "//aos/vision/image:image_types", 
      "//third_party/eigen",
    ])

cc_library(name = "contour",
    hdrs = ["contour.h"],
    srcs = ["contour.cc"],
    deps = [
      "//aos/vision/debug:overlay",
      "//aos/vision/comp_geo:segment",
      ":range_image",
      "//aos/vision:region_alloc",
        ])

cc_library(name = "threshold",
    hdrs = ["threshold.h"],
    deps = [":range_image",
        "//aos/vision/image:image_types", 
        ])

cc_library(name = "hierarchical_contour_merge",
    hdrs = ["hierarchical_contour_merge.h"],
    srcs = ["hierarchical_contour_merge.cc"],
    copts = ["-Wno-strict-overflow"],
    deps = [
      ":contour",
      ":disjoint_set",
      ":range_image",
      "//third_party/eigen",
      ])

cc_library(name = "disjoint_set",
    hdrs = ["disjoint_set.h"])

cc_library(name = "find_blob",
    hdrs = ["find_blob.h"],
    srcs = ["find_blob.cc"],
    deps = [
      "//aos/vision/debug:overlay",
      "//aos/vision/comp_geo:segment",
      ":disjoint_set",
      ":range_image",
      "//third_party/eigen",
    ])

cc_library(name = "codec",
    hdrs = ["codec.h"],
    srcs = ["codec.cc"],
    copts = ["-Wno-unused-variable"],
    deps = [
      "//aos/vision/debug:overlay",
      "//aos/vision/comp_geo:segment",
      ":range_image",
      "//third_party/eigen",
        ])

cc_test(name = "codec_test",
    srcs = ["codec_test.cc"],
    deps = [":codec",
    '//aos/testing:googletest',
      ])

cc_library(name = "stream_view",
    hdrs = ["stream_view.h"],
    deps = [
      ":range_image",
      "//aos/vision/debug:debug_viewer",
      "//aos/vision/image:image_types",
      ])
