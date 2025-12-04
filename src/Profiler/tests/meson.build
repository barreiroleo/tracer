profiler_exe = executable(
  'profiler',
  'profiler_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: [profiler_dep],
  override_options: ['cpp_std=c++23']
)

chrome_json_exe = executable(
  'chrome_json',
  'chrome_json_test.cpp',
  dependencies: [profiler_dep],
  override_options: ['cpp_std=c++17']
)

test('chrome_json', chrome_json_exe)
test('profiler_test', profiler_exe)
