cmake_minimum_required(VERSION 3.15...4.2)
project(weather_station)
include (CheckIncludeFiles)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/unabto_project.cmake)

CHECK_INCLUDE_FILES(curl/curl.h HAVE_CURL_H)


set(src
  ${unabto_src}
  ${unabto_module_cli_gopt_src}
  ${unabto_module_util_src}
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
  list(APPEND src ${unabto_module_provision_src})
  list(APPEND unabto_link_libraries ${unabto_module_provision_libraries})
endif()

add_executable(${PROJECT_NAME} ${src})

target_link_libraries(${PROJECT_NAME} ${unabto_link_libraries})

# Bundle the html device driver as html_dd.zip so it can be served to
# clients via the -U option, e.g. -U file://${CMAKE_CURRENT_BINARY_DIR}/html_dd.zip
# The zip is staged from the weather station's own html_dd plus the shared
# framework libs from apps/html_device_driver that index.html depends on.
set(html_dd_dir ${CMAKE_CURRENT_SOURCE_DIR}/html_dd)
set(html_dd_shared_libs ${CMAKE_CURRENT_SOURCE_DIR}/../html_device_driver/static/libs)
set(html_dd_stage ${CMAKE_CURRENT_BINARY_DIR}/html_dd_stage)
file(GLOB_RECURSE html_dd_files
  ${html_dd_dir}/nabto/*
  ${html_dd_dir}/static/*
  ${html_dd_shared_libs}/*
)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html_dd.zip
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${html_dd_stage}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${html_dd_shared_libs} ${html_dd_stage}/static/libs
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${html_dd_dir}/nabto ${html_dd_stage}/nabto
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${html_dd_dir}/static ${html_dd_stage}/static
  COMMAND ${CMAKE_COMMAND} -E chdir ${html_dd_stage} ${CMAKE_COMMAND} -E tar cf ${CMAKE_CURRENT_BINARY_DIR}/html_dd.zip --format=zip nabto static
  DEPENDS ${html_dd_files}
  COMMENT "Creating html_dd.zip"
)
add_custom_target(html_dd ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html_dd.zip)
add_dependencies(${PROJECT_NAME} html_dd)

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/html_dd.zip DESTINATION share/${PROJECT_NAME})

