project('quickjs-gobject',
  'cpp',
  version: '1.0.0',
)
# =============================================

gi_dep = dependency('gobject-introspection-1.0')
project_include = include_directories('deps', 'src')

# =============================================

project_source_files = files(
  'src/module.cc',
  'src/module.hh',
  'src/gi/function.cc',
  'src/gi/function.hh',
  'src/gi/type.cc',
  'src/gi/type.hh',
  'src/gi/value.hh',
  'src/gi/boxed.hh',
  'src/jsapi/BootstrapGI.cc',
  'src/jsapi/BootstrapGI.hh',
  'src/jsapi/opaque/JSFunctionInfo.cc',
  'src/jsapi/opaque/JSFunctionInfo.hh',
  'src/jsapi/opaque/FunctionInfo.cc',
  'src/jsapi/opaque/FunctionInfo.hh',  
  'src/utils/jsutils.cc',
  'src/utils/jsutils.hh',
  'src/utils/error.cc',
  'src/utils/error.hh',
  'src/utils/macros.hh',
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
  dependencies: [gi_dep],
  include_directories: project_include
)
