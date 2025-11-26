project(
  'tracer',
  'cpp',
  version: 'v0.0.1',
  meson_version: '>= 1.3.0',
  default_options: ['warning_level=3', 'cpp_std=c++23'],
)

subdir('lib/args')
# inc_lib = include_directories('lib')
# add_project_arguments('-DENABLE_TRACING', language: 'cpp')

executable(
  'tracer',
  'src/tracer.cpp',
  dependencies: [args_dep],
  cpp_args: ['-DENABLE_TRACING'],
)

server_exe = executable(
  'server',
  'tests/server_test.cpp',
  dependencies: [args_dep],
)
client_exe = executable(
  'client',
  'tests/client_test.cpp',
  dependencies: [args_dep],
)
profiler_exe = executable(
  'profiler',
  'tests/profiler_test.cpp',
  cpp_args: ['-DENABLE_TRACING'],
)
chrome_json_exe = executable(
  'chrome_json',
  'tests/chrome_json_test.cpp',
)

pipe_args = ['--pipe', '/var/run/tracer.pipe']
test('client_test', client_exe, args: pipe_args, timeout: 5)
test('server_test', server_exe, args: pipe_args, timeout: 5)
test('profiler_test', profiler_exe)
test('chrome_json', chrome_json_exe)
