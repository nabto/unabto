cmake_minimum_required(VERSION 2.8)
set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
include(${UNABTO_ROOT}/build/cmake/unabto_files.cmake)

set(src
  ${unabto_core_src}
  ${unabto_module_crypto_generic_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/network_adapter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dns_adapter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/time_adapter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/random_adapter.c

)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${UNABTO_ROOT}/src/
)

link_libraries(crypto pthread)

add_executable(porting ${src})
