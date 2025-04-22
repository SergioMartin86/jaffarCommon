project('SDL2_TTF','c',
  version: '1.0.0',
  default_options : [ 'default_library=static', 'buildtype=release'],
  subproject_dir: 'external'
)

# SDL2_TTF Core sources

freetypeSubproject = subproject('freetype')

SDL2_TTFSrc =  [
  'SDL_ttf.c',
]

SDL2_TTFIncludeDirs = [
  '.',
  'external/SDL2/include',
]

SDL2_TTFCompileArgs = [
  '-fPIC',
]

# SDL2_TTF Dependency

 SDL2_TTFDependency = declare_dependency(
  compile_args        : [  SDL2_TTFCompileArgs ],
  link_args           : [  ],
  include_directories : include_directories(SDL2_TTFIncludeDirs),
  sources             : [ SDL2_TTFSrc ],
  dependencies        : [  
                            dependency('freetype2'),
                        ]
 )

# Library

SDL2_TTFLibrary = library('SDL2_TTF', dependencies: [ SDL2_TTFDependency ] )