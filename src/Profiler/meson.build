profiler_dep = declare_dependency(
  version: '0.0.1',
  include_directories: include_directories('..'),
  dependencies: [args_dep, ipc_dep]
)

trace_collector = executable(
  'trace_collector',
  'trace_collector.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: [profiler_dep],
)

subdir('tests')
