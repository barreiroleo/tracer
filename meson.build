project(
  'tracer',
  'cpp',
  version: 'v0.0.1',
  meson_version: '>= 1.3.0',
  default_options: ['warning_level=3', 'werror=true', 'cpp_std=c++23'],
)
cpp_compiler = meson.get_compiler('cpp')

# Compiler GCC version
if cpp_compiler.get_id() == 'gcc' and cpp_compiler.version().version_compare('<14.0')
  error(
    'GCC version below 14.0. Update the GCC compiler or use clang instead. ',
    'Specify CXX=clang++ when configuring Meson.',
  )
endif
# Set C++ standard library for Clang/LLVM
if cpp_compiler.get_id() == 'clang' or cpp_compiler.get_id() == 'llvm'
  add_project_arguments(['-stdlib=libc++'], language: 'cpp')
  add_project_link_arguments(['-stdlib=libc++', '-lc++abi'], language: 'cpp')
endif

subdir('src')
subdir('tests')
