#!/usr/bin/python3
package(default_visibility = ["//visibility:public"])

cc_library(name = "epoll_events",
    srcs = ["epoll_events.cc"],
    hdrs = ["epoll_events.h"],
)

cc_library(name = "socket_types",
    hdrs = ["socket_types.h"],
    deps = [
        "//aos/vision/events:tcp_server",
        "//aos/vision/image:image_types",
    ],
)

cc_library(name = "tcp_server",
    srcs = ["tcp_server.cc"],
    hdrs = ["tcp_server.h"],
    deps = [":epoll_events"],
)

cc_library(name = "tcp_client",
    srcs = ["tcp_client.cc"],
    hdrs = ["tcp_client.h"],
    deps = [":epoll_events"],
)

cc_library(
    name = "udp",
    srcs = ["udp.cc"],
    hdrs = ["udp.h"],
)

cc_test(
    name = "udp_test",
    srcs = ["udp_test.cc"],
    deps = [
        ":udp",
        '//aos/testing:googletest'
        ],
)

cc_library(name = "gtk_event",
    srcs = ["gtk_event.cc"],
    hdrs = ["gtk_event.h"],
    deps = [
        ":epoll_events",
        '@usr_repo//:gtk+-3.0',
    ],
)

cc_binary(name = "gtk_example",
    srcs = ["gtk_example.cc"],
    deps = [
        ":gtk_event",
        '@usr_repo//:gtk+-3.0',
    ],
)
