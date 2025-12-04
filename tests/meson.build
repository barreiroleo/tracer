pipe_args = [
  '--pipe', '/tmp/tracer_trace_collector_cpp17.pipe',
  '--output', '/tmp/trace_collector_output_cpp17.json',
]
profiler_pipe_exe = executable(
  'profiler_pipe_cpp17',
  './profiler_pipe_cpp17_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  override_options: ['cpp_std=c++17'],
  dependencies: [profiler_dep],
)

test('trace_collector_cpp17', trace_collector, args: pipe_args, timeout: 5)
test('profiler_ipc_cpp17', profiler_pipe_exe, args: pipe_args, timeout: 5)
