cmake_minimum_required(VERSION 2.8)
project(weather_station)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/unabto_project.cmake)

set(src
  ${unabto_src}
  ${unabto_module_provision_src}
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

add_executable(${PROJECT_NAME} ${src})

target_link_libraries(${PROJECT_NAME} ${unabto_module_provision_libraries} ${unabto_link_libraries})

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

