cmake_minimum_required (VERSION 3.0)

include(FetchContent)

FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
  )

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

macro(module_test)
  set(id ${ARGV0}_test)

  add_executable(${id} ${id}.cpp)
  add_test(${id} ${id} COMMAND $<TARGET_FILE:${id}>)
  target_link_libraries(${id}
    PRIVATE
      jchannel
      gtest_main
  )

  unset(id)
endmacro()

module_test(ctor)
module_test(close)
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
module_test(interrupt)
module_test(unreaded)
