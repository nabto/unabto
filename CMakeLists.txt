cmake_minimum_required(VERSION 2.8)
project(unabto)

option(UNABTO_ENABLE_TESTS "Enable testing of unabto, disable this if you are crosscompiling" ON)

add_subdirectory(apps/pc_demo)
add_subdirectory(apps/weather_station)
add_subdirectory(apps/tunnel)
add_subdirectory(apps/stream_echo)

if (NOT MSVC)
  add_subdirectory(apps/multi_tunnel)
  add_subdirectory(apps/html_demo)
endif()


if (UNABTO_ENABLE_TESTS)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_subdirectory(test/unittest)
  endif()
  add_subdirectory(test/attach_test)
endif()
