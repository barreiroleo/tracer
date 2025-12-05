profiler_exe = executable(
  'profiler',
  'profiler_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: [profiler_dep],
)

chrome_json_exe = executable(
  'chrome_json',
  'chrome_json_test.cpp',
  dependencies: [profiler_dep],
)

test('chrome_json', chrome_json_exe)
test('profiler_test', profiler_exe)
