cmake_minimum_required(VERSION 2.8)
project(unabto)

option(UNABTO_ENABLE_TESTS "Enable testing of unabto, disable this if you are crosscompiling" ON)

add_subdirectory(apps/pc_demo)
add_subdirectory(apps/weather_station)
add_subdirectory(apps/html_demo)
add_subdirectory(apps/tunnel)
if (NOT MSVC)
  add_subdirectory(apps/uart_tunnel)
endif()
add_subdirectory(apps/stream_echo)
add_subdirectory(test/attach_test)


