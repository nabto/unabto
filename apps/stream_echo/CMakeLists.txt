cmake_minimum_required(VERSION 2.8)
project(stream_echo)

option(ENABLE_TCP_FALLBACK "enable tcp fallback" 0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/unabto_project.cmake)

set(src
  ${unabto_src}
  ${unabto_module_cli_gopt_src}
  ${unabto_module_util_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_echo.c
  )

if (ENABLE_TCP_FALLBACK)
  list(APPEND src ${unabto_module_tcp_fallback_src})
  if (NOT MSVC)
    list(APPEND src ${unabto_module_tcp_unix_src})
  else()
    list(APPEND src ${unabto_module_tcp_win32_src})
  endif()
  add_definitions("-DENABLE_TCP_FALLBACK")
endif()


set(src_tick
  ${src}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main_tick.c
)

set(src_select
  ${src}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main_select.c
)

set(src_epoll
  ${src}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main_epoll.c
  )

add_definitions(
  ${unabto_definitions}
)

include_directories(
  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

CHECK_INCLUDE_FILES("sys/epoll.h" HAVE_EPOLL)

add_executable(${PROJECT_NAME}_tick ${src_tick})
add_executable(${PROJECT_NAME} ${src_select})

if (UNABTO_HAVE_EPOLL)
  add_executable(${PROJECT_NAME}_epoll ${src_epoll})
  target_compile_definitions(${PROJECT_NAME}_epoll PUBLIC "-DNABTO_ENABLE_EPOLL=1")
  target_link_libraries(${PROJECT_NAME}_epoll ${unabto_link_libraries})
endif()
  
target_link_libraries(${PROJECT_NAME}_tick ${unabto_link_libraries})
target_link_libraries(${PROJECT_NAME} ${unabto_link_libraries})

  
install( TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_tick
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

if (HAVE_EPOLL)
  install( TARGETS ${PROJECT_NAME}_epoll
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif()
  
