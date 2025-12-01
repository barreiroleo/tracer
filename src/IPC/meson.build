ipc_dep = declare_dependency(
  version: '0.0.1',
  include_directories: include_directories('..'),
)

server_exe = executable(
  'server',
  'tests/server_test.cpp',
  dependencies: args_dep,
)
client_exe = executable(
  'client',
  'tests/client_test.cpp',
  dependencies: args_dep,
)

pipe_args = ['--pipe', '/tmp/tracer.pipe']
test('client_test', client_exe, args: pipe_args, timeout: 5)
test('server_test', server_exe, args: pipe_args, timeout: 5)
