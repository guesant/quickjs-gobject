project('quickjs-gobject',
  'cpp',
  version: '1.0.0',
)
# =============================================

project_include = include_directories('deps', 'src')

# =============================================

project_source_files = files(
  'src/module.cc',
  'src/module.hh',
  'src/jsapi/BootstrapGI.cc',
  'src/jsapi/BootstrapGI.hh',
  'src/utils/jsutils.cc',
  'src/utils/jsutils.hh',
)

# =============================================

project_lib_args = ['-DJS_SHARED_LIBRARY']

add_project_link_arguments(
  '-Wl,--unresolved-symbols=ignore-in-object-files',
  language: ['cpp']
)

# =============================================

project_lib_target = shared_library(
  meson.project_name(),
  project_source_files,
  install: true,
  c_args: project_lib_args,
  cpp_args: project_lib_args,
  include_directories: project_include
)
