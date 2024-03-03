testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]

bitwiseTester = executable('bitwise',
  files(['bitwise.cpp']),
  include_directories: [ commonIncludes ],
  link_args: [ testCommonLinkArgs ],
  cpp_args: [ commonCppArgs, testCommonCppArgs ]
  )

test('bitwise', bitwiseTester,
  protocol: 'gtest',
  suite: 'unit'
)
