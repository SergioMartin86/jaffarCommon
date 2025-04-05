testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]

# Code coverage configuration
if get_option('b_coverage')
  testCommonCppArgs += ['-fno-inline','-Wno-error=cpp' ]
endif

unitTestSet = [
  'bitwise',
  'concurrent',
  'exceptions',
  'file',
  'hash',
  'json',
  'string',
  'timing',
  'serialization',
  'dethreader'
]

# Only add logger tests if running in an interactive node
if get_option('testLoggers') == true
 unitTestSet += [ 'loggerStdio', 'loggerNCurses' ]
endif

# Adding tests to the suite
foreach testFile : unitTestSet
  testSuite = 'unit'
  testName = testFile

  exec = executable('t' + testName, 
  files([testName + '.cpp']),
  dependencies: jaffarCommonDependency,
  link_args: [ testCommonLinkArgs ],
  cpp_args: [ testCommonCppArgs ]
  )

  test(testName,
       exec,
       workdir : meson.current_source_dir(),
       suite : [ testSuite ])
endforeach

# Subproject testing
if get_option('includeFFmpeg') == true and get_option('includeZlib') == true

  exec = executable('tffmpeg',
  files(['ffmpeg.cpp']),
  dependencies: [ jaffarCommonDependency, ffmpegDependency, zlibDependency ],
  link_args: [ testCommonLinkArgs ],
  cpp_args: [ testCommonCppArgs ]
  )

  test(testName,
       exec,
       workdir : meson.current_source_dir(),
       suite : [ testSuite ])
endif
