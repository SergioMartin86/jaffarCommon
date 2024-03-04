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
  'loggerStdio',
  'loggerNCurses',
  'string',
  'timing',
  'serialization'
]

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
