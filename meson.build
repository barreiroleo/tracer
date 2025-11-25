project(
  'tracer',
  'cpp',
  version: 'v0.0.1',
  meson_version: '>= 1.3.0',
  default_options: ['warning_level=3', 'cpp_std=c++23'],
)

subdir('lib/args')
# inc_lib = include_directories('lib')

tracer_exe = executable(
  'tracer',
  'src/tracer.cpp',
  dependencies: [args_dep],
)

client_exe = executable(
  'client',
  'tests/client_test.cpp',
  dependencies: [args_dep],
)

pipe_args = ['--pipe', '/var/run/tracer.pipe']
test('client_test', client_exe, args: pipe_args, timeout: 5)
test('server_test', tracer_exe, args: pipe_args, timeout: 5)
