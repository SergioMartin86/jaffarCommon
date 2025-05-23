project('jaffarCommon','c','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
  subproject_dir: 'subprojects'
)

# Adding subprojects
if get_option('includeFFmpeg') == true
 ffmpegSubproject = subproject('ffmpeg')
 ffmpegLibrary = ffmpegSubproject.get_variable('ffmpegLibrary')
endif

if get_option('includeZlib') == true
 zlibSubproject = subproject('zlib')
 zlibLibrary = zlibSubproject.get_variable('zlibLibrary')
endif

if get_option('includeSDL2_TTF') == true
 SDL2_TTFSubproject = subproject('SDL2_TTF')
 SDL2_TTFDependency = SDL2_TTFSubproject.get_variable('SDL2_TTFDependency')
endif

# Common compile flags
commonCppArgs = [ '-D__JAFFAR_COMMON_INLINE__=__attribute__((__used__)) inline' ]

# Common link flags
commonLinkArgs = [ '-latomic' ]

# Common includes
commonIncludes = include_directories(['third_party', 'include'])

# Common sources
commonSources = [ 
   'third_party/metrohash128/metrohash128.cpp',
   'third_party/xdelta3/xdelta3.c',
   'third_party/libco/libco.c',
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