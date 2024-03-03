testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]

# Code coverage configuration
if get_option('b_coverage')
  testCommonCppArgs += ['-fno-inline','-Wno-error=cpp' ]
endif

testSet = [
  'unit.bitwise',
  'unit.exceptions',
  'unit.file',
  'unit.hash',
  'coverage.includes'
]

# Adding tests to the suite
foreach testFile : testSet
  testSuite = testFile.split('.')[0]
  testName = testFile.split('.')[1]

  exec = executable(testName, 
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
