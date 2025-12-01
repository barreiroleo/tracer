subdir('Args')
subdir('IPC')
subdir('Profiler')

executable(
  'tracer',
  'tracer.cpp',
  cpp_args: ['-DENABLE_TRACING'],
)

tracer_client = executable(
  'tracer_client',
  'tests/tracer_client.cpp',
  cpp_args: ['-DENABLE_TRACING'],
  dependencies: profiler_dep,
)
test('tracer_client_test', tracer_client)
