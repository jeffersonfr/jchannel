cmake_minimum_required (VERSION 3.0)

macro(module_test)
  add_executable(${ARGV0}_test
    ${ARGV0}_test.cpp
  )

  add_test(${ARGV0}_test ${ARGV0}_test
    COMMAND $<TARGET_FILE:${ARGV0}>
  )

  target_link_libraries(${ARGV0}_test
    PRIVATE
      jchannel
  )
endmacro()

module_test(ctor)
module_test(empty_poll)
module_test(sigpipe)
module_test(stress)
module_test(fifo)
