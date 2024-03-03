testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]

testSet = [
  'unit.bitwise',
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
