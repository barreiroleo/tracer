ipc_client_lib = static_library(
  'ipc_client',
  ['client.cpp', 'message.cpp'],
  override_options: ['cpp_std=c++17'],
)

ipc_server_lib = static_library(
  'ipc_server',
  ['server.cpp', 'message.cpp'],
  override_options: ['cpp_std=c++23'],
)

ipc_dep = declare_dependency(
  version: '0.0.1',
  include_directories: include_directories('..'),
  link_with: [ipc_client_lib, ipc_server_lib],
)

server_exe = executable(
  'server',
  'tests/server_test.cpp',
  dependencies: [args_dep, ipc_dep],
)
client_exe = executable(
  'client',
  'tests/client_test.cpp',
  dependencies: [args_dep, ipc_dep],
)

pipe_args = ['--pipe', '/tmp/tracer-IPC_test.pipe']
test('client_test', client_exe, args: pipe_args, timeout: 5)
test('server_test', server_exe, args: pipe_args, timeout: 5)
