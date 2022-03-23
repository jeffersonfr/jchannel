cmake_minimum_required (VERSION 3.0)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

project (jchannel CXX)

option(TESTS "Enable unit tests" OFF)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE
  )

target_include_directories(${PROJECT_NAME} INTERFACE 
  ${PROJECT_SOURCE_DIR}/include
  )

target_link_libraries(${PROJECT_NAME} INTERFACE
  Threads::Threads
  )

enable_testing()

if (TESTS)
  add_subdirectory(tests)
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.pc.in
  ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc
  @ONLY
)

install(DIRECTORY include/${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(FILES ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
