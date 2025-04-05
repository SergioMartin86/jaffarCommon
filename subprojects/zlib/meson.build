project('zlib','c',
  version: '1.0.0',
  default_options : [ 'default_library=shared', 'buildtype=release'],
)

# zlib Core sources

zlibSrc =  [
  'adler32.c',
  'compress.c',
  'crc32.c',
  'deflate.c',
  'gzclose.c',
  'gzlib.c',
  'gzread.c',
  'gzwrite.c',
  'infback.c',
  'inffast.c',
  'inflate.c',
  'inftrees.c',
  'trees.c',
  'uncompr.c',
  'zutil.c',
]

zlibIncludeDirs = [
  '.',
]

zlibCompileArgs = [
]

# zlib Dependency

 zlibDependency = declare_dependency(
  compile_args        : [  zlibCompileArgs ],
  link_args           : [  ],
  include_directories : include_directories(zlibIncludeDirs),
  sources             : [ zlibSrc ],
  dependencies        : [  
                        ]
 )
