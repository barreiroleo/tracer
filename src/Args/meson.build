args_lib = static_library(
  'args',
  'args.cpp',
  override_options: ['cpp_std=c++23'],
)

args_dep = declare_dependency(
  version: '0.0.1',
  include_directories: include_directories('..'),
  link_with: args_lib,
)

test_pass_cases = [
  {
    'name': 'pipe',
    'args': ['--pipe', '/var/lock/tracer_pipe'],
    'should_fail': false,
  },
  {
    'name': 'help',
    'args': ['--help'],
    'should_fail': false,
  },
]

test_fail_cases = [
  {
    'name': 'no_args',
    'args': [],
    'should_fail': true,
  },
  {
    'name': 'invalid_arg',
    'args': ['-'],
    'should_fail': true,
  },
  {
    'name': 'unknown_arg',
    'args': ['--unknown_arg'],
    'should_fail': true,
  },
]

arg_test_exe = executable(
  'args_test',
  'tests/test.cpp',
  dependencies: [args_dep],
)

foreach case : test_pass_cases + test_fail_cases
  test(
    'test_' + case['name'],
    arg_test_exe,
    should_fail: case['should_fail'],
    args: case['args'],
  )
endforeach
