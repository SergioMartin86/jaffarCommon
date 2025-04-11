project('SDLTTF','c',
  version: '1.0.0',
  default_options : [ 'default_library=static', 'buildtype=release'],
)

# SDLTTF Core sources

SDLTTFSrc =  [
  'SDL_ttf.c',
  'showfont.c',
  'glfont.c',
]

SDLTTFIncludeDirs = [
  '.',
]

SDLTTFCompileArgs = [
  '-fPIC'
]

# SDLTTF Dependency

 SDLTTFDependency = declare_dependency(
  compile_args        : [  SDLTTFCompileArgs ],
  link_args           : [  ],
  include_directories : include_directories(SDLTTFIncludeDirs),
  sources             : [ SDLTTFSrc ],
  dependencies        : [  
                        ]
 )

# Library

SDLTTFLibrary = library('SDLTTF', dependencies: [ SDLTTFDependency ] )