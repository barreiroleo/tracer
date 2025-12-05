client_cpp14_ipc_exe = executable(
  'client_cpp14_ipc',
  'client_cpp14/ipc.cpp',
  dependencies: [ipc_dep],
  override_options: ['cpp_std=c++14'],
  cpp_args: ['-DENABLE_TRACING', '-Wall', '-Wextra', '-Werror'],
)

client_cpp14_profiler_exe = executable(
  'client_cpp14_profiler',
  'client_cpp14/profiler.cpp',
  dependencies: [profiler_dep],
  override_options: ['cpp_std=c++14'],
  cpp_args: ['-DENABLE_TRACING', '-Wall', '-Wextra', '-Werror'],
)

test('client_cpp14_ipc', client_cpp14_ipc_exe)
test('client_cpp14_profiler', client_cpp14_profiler_exe)
