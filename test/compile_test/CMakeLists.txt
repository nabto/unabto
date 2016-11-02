cmake_minimum_required(VERSION 2.8)
project(unabto_unittest)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

option(USE_DUMMY_RANDOM "Link without crypto.so" OFF)

if(EXTERNAL_PREFIX) 
  include_directories(${EXTERNAL_PREFIX}/include)
  link_directories(${EXTERNAL_PREFIX}/lib)
endif()

set(unabto_unittest_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(src
  ${unabto_unittest_src}/main.c
  ${unabto_module_cli_gopt_src}
  ${UNABTO_MODULES_SRC_DIR}/log/unabto_basename.c
  ${UNABTO_MODULES_SRC_DIR}/diagnostics/unabto_diag.c
  ${UNABTO_MODULES_SRC_DIR}/mt/unabto_thread.c
  ${UNABTO_MODULES_SRC_DIR}/mt/unabto_mutex.c
  ${UNABTO_MODULES_SRC_DIR}/mt/unabto_cond.c
  ${unabto_src}
)

include_directories(
  ${unabto_include_directories}
  ${unabto_core_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

add_definitions(${unabto_definitions})

if(NOT MSVC)
  add_definitions(-Wall)
  add_definitions(-Werror)
endif()

add_executable(${PROJECT_NAME} ${src})

target_link_libraries(${PROJECT_NAME} ${unabto_link_libraries})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
