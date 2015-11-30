cmake_minimum_required(VERSION 2.8.0)
project(tick_timer_test)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
include(${UNABTO_ROOT}/build/cmake/unabto_files.cmake)

set(src 
  ${UNABTO_MODULES_SRC_DIR}/timers/tick_timer/unabto_tick_timer.c
  #${UNABTO_MODULES_SRC_DIR}/timers/unix/unabto_unix_time.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unabto_tick_timer_test.c
)

include_directories(
  ${unabto_core_include_directories}
  ${UNABTO_MODULES_SRC_DIR}/network/bsd
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${src})
