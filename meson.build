project('jaffarCommon','c','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release']
)

# Common application flags
commonCppArgs = [ ]

# Common includes
commonIncludes = include_directories(['extern'])

# Common sources
commonSources = [ 
   'extern/metrohash128/metrohash128.cpp',
   'extern/xdelta3/xdelta3.c',
   ]

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

# Building tests
subdir('tests')
  
endif # If not subproject