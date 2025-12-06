trace_collector = executable(
  'trace_collector',
  'main.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: [args_dep, profiler_dep],
)

profiler_pipe_exe = executable(
  'profiler_pipe',
  'tests/profiler_pipe_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: [profiler_dep],
)

pipe_args = [
  '--pipe', '/tmp/tracer_trace_collector.pipe',
  '--output', '/tmp/trace_collector_output.json',
]
test('trace_collector', trace_collector, args: pipe_args, timeout: 5)
test('profiler_ipc', profiler_pipe_exe, args: pipe_args, timeout: 5)
