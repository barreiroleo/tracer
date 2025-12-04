profiler_lib = static_library(
  'profiler',
  'trace.cpp',
  dependencies: [args_dep, ipc_dep],
)

profiler_dep = declare_dependency(
  version: '0.0.1',
  include_directories: include_directories('..'),
  link_with: profiler_lib,
  dependencies: [args_dep, ipc_dep],
)

subdir('tests')
