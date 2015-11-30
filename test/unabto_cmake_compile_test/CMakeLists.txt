project(unabto_cmake_compile_test)
cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/unabto_project.cmake)

set(src
  ${unabto_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../demo/pc_demo/src/unabto_main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../demo/pc_demo/src/unabto_application.c
  ${unabto_module_cli_gopt_src}
)

link_libraries(
  ${unabto_link_libraries}
)

include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  ${unabto_definitions}
)

add_executable(testprogram ${src})