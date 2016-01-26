cmake_minimum_required(VERSION 2.8)
project(weather_station)
include (CheckIncludeFiles)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/unabto_project.cmake)

CHECK_INCLUDE_FILES(curl.h HAVE_CURL_H)


set(src
  ${unabto_src}
  ${unabto_module_cli_gopt_src}
  ${unabto_server_weather_station}
)

add_definitions(
  ${unabto_definitions}
)

include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/config
)

if (HAVE_CURL_H)
  add_definitions(-DNABTO_ENABLE_PROVISIONING=1)
  list(append src ${unabto_module_provision_src})
  list(append unabto_link_libraries ${unabto_module_provision_libraries})
endif()

add_executable(${PROJECT_NAME} ${src})

target_link_libraries(${PROJECT_NAME} ${unabto_link_libraries})

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

