cmake_minimum_required(VERSION 2.8)
project(unabto_tcp_test)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)

include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

set(src
  ${unabto_src}
  ${unabto_module_application_event_dummy_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/unabto_tcp_test.c
  ${unabto_module_util_src}
)

if (WIN32)
  list(APPEND src
    ${unabto_module_tcp_win32_src}
    )
else ()
  list(APPEND src
    ${unabto_module_tcp_unix_src}
    )
endif()

include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
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
