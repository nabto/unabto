cmake_minimum_required(VERSION 2.8)
project(openssl_minimal_test C ASM)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)

include(${UNABTO_ROOT}/build/cmake/unabto_files.cmake)

set(src
  ${unabto_crypto_openssl_minimal_armv4_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  )

include_directories(${UNABTO_ROOT}/src ${UNABTO_ROOT}/src/platforms/unix ${CMAKE_CURRENT_SOURCE_DIR}) 

add_executable(${PROJECT_NAME} "${src}")