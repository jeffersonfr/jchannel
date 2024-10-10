cmake_minimum_required (VERSION 3.0)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

project(jchannel
  LANGUAGES CXX
  HOMEPAGE_URL https://github.com/jeffersonfr/jchannel
  DESCRIPTION "A static channel implementation"
  VERSION 1.0.0
)

option(JCHANNEL_TESTS "Enable unit tests" OFF)
option(JCHANNEL_CHECKER "Enable static code analysing" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (JCHANNEL_CHECKER)
   set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,android-*,bugprone-*,clang-analyzer-*,hicpp-*,misc-*,modernize-*,portability-*)
endif()

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE
  )

target_include_directories(${PROJECT_NAME}
  INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/jchannel>
  )

target_link_libraries(${PROJECT_NAME}
  INTERFACE
    Threads::Threads
  )

enable_testing()

if (JCHANNEL_TESTS)
  add_subdirectory(tests)
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.pc.in
  ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc
  @ONLY
)

install(DIRECTORY include/${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# generate pc file
configure_file(jchannel.pc.in jchannel.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/jchannel.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
