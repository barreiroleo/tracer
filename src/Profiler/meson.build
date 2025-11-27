profiler_dep = declare_dependency(
  version: '0.0.1',
  include_directories: include_directories('..'),
)

profiler_exe = executable(
  'profiler',
  'tests/profiler_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
)
chrome_json_exe = executable(
  'chrome_json',
  'tests/chrome_json_test.cpp',
)

test('profiler_test', profiler_exe)
test('chrome_json', chrome_json_exe)
