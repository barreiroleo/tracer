profiler_exe = executable(
  'profiler',
  'profiler_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: [profiler_dep],
)

profiler_pipe_exe = executable(
  'profiler_pipe',
  'profiler_pipe_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: [profiler_dep],
)

chrome_json_exe = executable(
  'chrome_json',
  'chrome_json_test.cpp',
  dependencies: [profiler_dep],
)

pipe_args = [
  '--pipe', '/tmp/tracer_trace_collector.pipe',
  '--output', '/tmp/trace_collector_output.json'
]
test('chrome_json', chrome_json_exe)
test('profiler_test', profiler_exe)
test('trace_collector', trace_collector, args: pipe_args, timeout: 5)
test('profiler_ipc', profiler_pipe_exe, args: pipe_args, timeout: 5)
