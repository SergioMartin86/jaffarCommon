project('SDL_TTF','c',
  version: '1.0.0',
  default_options : [ 'default_library=static', 'buildtype=release'],
)

# SDL_TTF Core sources

SDL_TTFSrc =  [
  'SDL_ttf.c',
  'showfont.c',
  'glfont.c',
]

SDL_TTFIncludeDirs = [
  '.',
]

SDL_TTFCompileArgs = [
  '-fPIC'
]

# SDL_TTF Dependency

 SDL_TTFDependency = declare_dependency(
  compile_args        : [  SDL_TTFCompileArgs ],
  link_args           : [  ],
  include_directories : include_directories(SDL_TTFIncludeDirs),
  sources             : [ SDL_TTFSrc ],
  dependencies        : [  
                        ]
 )

# Library

SDL_TTFLibrary = library('SDL_TTF', dependencies: [ SDL_TTFDependency ] )