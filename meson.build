project('jaffarCommon','c','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release']
)

# Common compile flags
commonCppArgs = [ '-D__INLINE__=__attribute__((__used__)) inline' ]

# Common link flags
commonLinkArgs = [ '-latomic' ]

# Common includes
commonIncludes = include_directories(['third_party', 'include'])

# Common sources
commonSources = [ 
   'third_party/metrohash128/metrohash128.cpp',
   'third_party/xdelta3/xdelta3.c',
   ]

# Common dependencies
commonDependencies = [
  dependency('openmp', required: true),
  dependency('tbb', required: true),
  dependency('ncurses', required: true)
]

jaffarCommonDependency = declare_dependency(
  compile_args        : commonCppArgs,
  link_args           : commonLinkArgs,
  include_directories : commonIncludes,
  sources             : commonSources,
  dependencies        : commonDependencies
 )

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

# Building tests
subdir('tests')
  
endif # If not subproject