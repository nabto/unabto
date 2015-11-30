cmake_minimum_required(VERSION 2.8)

project(unabto_linux_test)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${UNABTO_ROOT}/build/cmake/unabto_files.cmake)

option(LOG_ALL "Enable all log output" OFF)

if (LOG_ALL)
  add_definitions(-DNABTO_LOG_ALL=1)
endif()

include_directories(
  ${UNABTO_PLATFORMS_SRC_DIR}/unix
  ${unabto_core_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${UNABTO_SERVER_TEST_DIR}
)

set(src
  ${unabto_platform_unix_src}
  ${unabto_module_log_unix_src}
  ${unabto_crypto_src}
  ${unabto_module_timers_unix_src}
  ${unabto_module_random_dummy_src}
  ${unabto_server_common_src_test}
  ${unabto_server_stream_test_src}
  ${unabto_module_network_bsd_src}
  ${unabto_module_unix_dns_src}
  ${unabto_module_application_event_dummy_src}
  ${unabto_module_crypto_generic_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/runLinuxTest.c)

add_definitions(-Wall)

add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME} pthread rt m)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
