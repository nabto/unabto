cmake_minimum_required(VERSION 2.8)
project(unabto-tunnel)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

option(USE_LIB_PROFILER "Link executables with libprofiler.so" OFF)
option(LOG_ALL "Print all unabto log statements" OFF)
option(USE_TEST_WEBSERVER "Enable a mongoose test webserver" OFF)

set(unabto_server_unabto_tunneling_src ${CMAKE_CURRENT_SOURCE_DIR}/../src)


include_directories(
  # make it possible to override unabto_config.h with a supplied file
  # to the build tree.
  ${CMAKE_CURRENT_BINARY_DIR}

  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  )

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../src/modules/tunnel ${CMAKE_CURRENT_BINARY_DIR}/tunnel_library)

add_definitions(${unabto_definitions})

if (USE_LIB_PROFILER)
  list(APPEND unabto_link_libraries profiler)
endif()

add_executable(test_tunnels ${CMAKE_CURRENT_SOURCE_DIR}/src/test_tunnels.c)

target_link_libraries(test_tunnels unabto_tunnel_library ${unabto_link_libraries})


install( TARGETS test_tunnels
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
