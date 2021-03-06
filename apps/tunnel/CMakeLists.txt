cmake_minimum_required(VERSION 2.8)
project(unabto-tunnel C)


set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

option(USE_LIB_PROFILER "Link executables with libprofiler.so" OFF)
option(LOG_ALL "Print all unabto log statements" OFF)

include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

INCLUDE (CheckIncludeFiles)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )

CHECK_LIBRARY_EXISTS(c getifaddrs "" HAVE_GETIFADDRS_SYMBOL)
CHECK_INCLUDE_FILES (ifaddrs.h HAVE_IFADDRS_H)
if (HAVE_IFADDRS_H AND HAVE_GETIFADDRS_SYMBOL)
  add_definitions(-DHAVE_IFADDRS_H)
endif()

if(EXTERNAL_PREFIX) 
  include_directories(${EXTERNAL_PREFIX}/include)
  link_directories(${EXTERNAL_PREFIX}/lib)
endif()

set(unabto_server_unabto_tunneling_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(ENABLE_TUNNEL_TCP 1)
set(ENABLE_TUNNEL_UART 1)
set(ENABLE_TUNNEL_ECHO 1)

include_directories(
  # make it possible to override unabto_config.h with a supplied file
  # to the build tree.
  ${CMAKE_CURRENT_BINARY_DIR}

  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

# build the tunnelling library with the unabto_config.h defined in this project.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/tunnel ${CMAKE_CURRENT_BINARY_DIR}/tunnel_library)

set(tunnel_src
  ${unabto_server_unabto_tunneling_src}/platform_checks.c  
  ${unabto_module_util_src}
  ${unabto_module_fingerprint_acl_src}
  )

add_definitions(${unabto_definitions})

if (UNABTO_HAVE_EPOLL)
  add_definitions("-DNABTO_ENABLE_EPOLL=1")
endif()

if (USE_LIB_PROFILER)
  list(APPEND unabto_link_libraries profiler)
endif()

# Called by developer - build and install

add_executable(unabto_tunnel ${tunnel_src} ${unabto_server_unabto_tunneling_src}/main.c)
add_executable(custom_tunnel_demo ${tunnel_src} ${unabto_server_unabto_tunneling_src}/custom_main.c)

target_link_libraries(unabto_tunnel unabto_tunnel_library ${unabto_link_libraries})
target_link_libraries(custom_tunnel_demo unabto_tunnel_library ${unabto_link_libraries})


install( TARGETS unabto_tunnel
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
