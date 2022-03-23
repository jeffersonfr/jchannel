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

  target_compile_options(${ARGV0}_test
    PRIVATE
      -g3 -ggdb3
    )
endmacro()

module_test(ctor)
module_test(empty_poll)
module_test(sigpipe)
module_test(stress)
module_test(fifo)
module_test(blocking)
module_test(stream_data)
module_test(packet_data)
module_test(fork)
module_test(init_poll)
module_test(stream_poll)
module_test(packet_poll)
module_test(multi_poll)
module_test(unreaded)
