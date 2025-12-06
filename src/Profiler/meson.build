profiler_lib = static_library(
  'profiler',
  [
    'trace.cpp',
    'chrome_event.cpp',
    'exporters/file_exporter.cpp',
    'exporters/ipc_exporter.cpp',
  ],
  dependencies: [ipc_dep],
  override_options: ['cpp_std=c++17'],
)

profiler_dep = declare_dependency(
  version: '0.0.1',
  include_directories: include_directories('..'),
  link_with: profiler_lib,
  dependencies: [ipc_dep],
)

subdir('tests')
