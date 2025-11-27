args_dep = declare_dependency(
  include_directories: include_directories('..'),
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
)

foreach case : test_pass_cases + test_fail_cases
  test(
    'test_' + case['name'],
    arg_test_exe,
    should_fail: case['should_fail'],
    args: case['args'],
  )
endforeach
