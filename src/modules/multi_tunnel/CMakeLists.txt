cmake_minimum_required(VERSION 2.8)
project(unabto_multi_tunnel_library C ASM)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

set(unabto_server_unabto_multi_tunneling_src ${CMAKE_CURRENT_SOURCE_DIR})

set(unabto_multi_tunnel_src
  ${unabto_server_unabto_multi_tunneling_src}/tunnel.c
  ${unabto_server_unabto_multi_tunneling_src}/tunnel_select.c
  )

if (UNABTO_HAVE_EPOLL)
  add_definitions("-DNABTO_ENABLE_EPOLL=1")
  list(APPEND unabto_multi_tunnel_src ${unabto_server_unabto_multi_tunneling_src}/tunnel_epoll.c)
endif()

source_group(modules\\tunnel\\src FILES ${unabto_multi_tunnel_src})

list(APPEND unabto_src
  ${unabto_multi_tunnel_src}
  
  ${unabto_module_cli_gopt_src}
  ${unabto_module_diagnostics_src}
  ${unabto_module_log_dynamic_src}
  ${unabto_module_log_syslog_src}
  ${unabto_module_tcp_fallback_src}
  ${unabto_module_tunnel_common_src}
)

add_definitions(${unabto_definitions})

include_directories(
  ${unabto_include_directories}
  ${unabto_server_unabto_multi_tunneling_src}
  )

add_library(${PROJECT_NAME} STATIC ${unabto_src})
