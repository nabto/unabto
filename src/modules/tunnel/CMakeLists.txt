cmake_minimum_required(VERSION 2.8)
project(unabto_tunnel_library C ASM)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

set(unabto_server_unabto_tunneling_src ${CMAKE_CURRENT_SOURCE_DIR})

if ((NOT ENABLE_TUNNEL_TCP) AND (NOT ENABLE_TUNNEL_UART) AND (NOT ENABLE_TUNNEL_ECHO))
  message(FATAL_ERROR "no tunnel modes enabled")
endif()

set(unabto_tunnel_src
  ${unabto_server_unabto_tunneling_src}/tunnel.c
  ${unabto_server_unabto_tunneling_src}/tunnel_select.c
  )

if (UNABTO_HAVE_EPOLL)
  add_definitions("-DNABTO_ENABLE_EPOLL=1")
  list(APPEND unabto_tunnel_src ${unabto_server_unabto_tunneling_src}/tunnel_epoll.c)
endif()

source_group(modules\\tunnel\\src FILES ${unabto_tunnel_src})

if(ENABLE_TUNNEL_TCP)
  list(APPEND unabto_src ${unabto_module_tunnel_tcp_src})
  add_definitions("-DNABTO_ENABLE_TUNNEL_TCP")
endif()

if(ENABLE_TUNNEL_UART)
  list(APPEND unabto_src ${unabto_module_tunnel_uart_src})
  add_definitions("-DNABTO_ENABLE_TUNNEL_UART")
endif()

if(ENABLE_TUNNEL_ECHO)
  list(APPEND unabto_src ${unabto_module_tunnel_echo_src})
  add_definitions("-DNABTO_ENABLE_TUNNEL_ECHO")
endif()

list(APPEND unabto_src
  ${unabto_tunnel_src}
  
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
  ${unabto_server_unabto_tunneling_src}
  )

if (NOT TUNNEL_LIBRARY_NAME)
  set(TUNNEL_LIBRARY_NAME ${PROJECT_NAME})
endif()

add_library(${TUNNEL_LIBRARY_NAME} STATIC ${unabto_src})
