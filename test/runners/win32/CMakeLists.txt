cmake_minimum_required(VERSION 2.8)

project("unabto-unittest-win32")

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${UNABTO_ROOT}/build/cmake/unabto_files.cmake)

set(common_test_src
  ${unabto_weather_station_test}
  ${unabto_server_common_src_test}
  ${unabto_module_crypto_generic_src}
)

set(src_test ${common_test_src} ${unabto_pc_src_minimal})

add_definitions(-DWIN32_LEAN_AND_MEAN)

include_directories(
  ${unabto_core_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/config
  ${UNABTO_SERVER_TEST_DIR}
)

add_executable(${PROJECT_NAME} ${src_test})

target_link_libraries(${PROJECT_NAME} Ws2_32)


install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

