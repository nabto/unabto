cmake_minimum_required(VERSION 2.8)
project(unabto_async_test)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/unabto_project.cmake)

set(src
  ${unabto_src}
  ${unabto_module_cli_gopt_src}
  ${unabto_module_util_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/async_application_event.c
)

add_definitions(
  ${unabto_definitions}
)

include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(tick_src
  ${src}
  ${CMAKE_CURRENT_SOURCE_DIR}/main_tick.c
  )

set(select_src
  ${src}
  ${CMAKE_CURRENT_SOURCE_DIR}/main_select.c
  )

add_executable(${PROJECT_NAME}_tick ${tick_src})

add_executable(${PROJECT_NAME}_select ${select_src})

target_link_libraries(${PROJECT_NAME}_tick ${unabto_link_libraries})
target_link_libraries(${PROJECT_NAME}_select ${unabto_link_libraries})

install( TARGETS ${PROJECT_NAME}_tick ${PROJECT_NAME}_select
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
