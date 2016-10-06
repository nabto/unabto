cmake_minimum_required(VERSION 3.0)

project(attach_test)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/unabto_project.cmake)

set(src
  ${unabto_src}
  ${unabto_module_cli_gopt_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)

include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_definitions(
  ${unabto_definitions}
)

add_executable(${PROJECT_NAME} ${src})

target_link_libraries(${PROJECT_NAME} ${unabto_link_libraries})

install( TARGETS ${PROJECT_NAME} 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
