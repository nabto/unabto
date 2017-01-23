cmake_minimum_required(VERSION 2.8)
project(axistunnel)

set(axis_sdk_dir $ENV{AXIS_SDK})

include_directories(SYSTEM
  ${axis_sdk_dir}/target/${AXIS_BUILDTYPE}/include
  ${axis_sdk_dir}/target/${AXIS_BUILDTYPE}/usr/include
)


set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

set(ENABLE_TUNNEL_TCP 1)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/tunnel ${CMAKE_CURRENT_BINARY_DIR}/tunnel_library)

set(src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/axis.c
  ${unabto_module_util_src}
)

if(UNABTO_HAVE_EPOLL) 
  add_definitions(-DNABTO_ENABLE_EPOLL=1)
endif()

include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

add_definitions(${unabto_definitions})

add_definitions(-Wall)

# Just build a static library let the axis build system handle the 
# complicated linking
add_library(${PROJECT_NAME} STATIC ${src})

