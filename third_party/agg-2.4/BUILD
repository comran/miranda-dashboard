licenses(['notice'])

cc_library(
  name = 'agg-2.4',
  visibility = ['//visibility:public'],
  srcs = glob(['src/**/*.cpp',]),
  hdrs = glob(['include/**/*.h',]),
  includes = ['include'],
  deps = ['//third_party/libX11:libX11'],
  copts = [
    '-Wno-unused-const-variable',
    '-Wno-unused-parameter',
    '-Wno-reorder',
    '-Wno-switch',
    '-Wno-format-nonliteral',
    '-Wno-format',
    '-Wno-switch-enum',
  ],
)
