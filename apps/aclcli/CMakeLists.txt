cmake_minimum_required(VERSION 2.8)
project(unabto-tunnel C ASM)


set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

option(LOG_ALL "Print all unabto log statements" OFF)

include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

INCLUDE (CheckIncludeFiles)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )


if(EXTERNAL_PREFIX) 
  include_directories(${EXTERNAL_PREFIX}/include)
  link_directories(${EXTERNAL_PREFIX}/lib)
endif()


include_directories(
  # make it possible to override unabto_config.h with a supplied file
  # to the build tree.
  ${CMAKE_CURRENT_BINARY_DIR}

  ${unabto_include_directories}
  ${UNABTO_ROOT}/src/modules/fingerprint_acl
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

set(aclcli_src ${CMAKE_CURRENT_SOURCE_DIR}/src)


# modules/fingerprint_acl
set(unabto_module_fp_src
  ${UNABTO_MODULES_SRC_DIR}/fingerprint_acl/fp_acl_memory.c
  ${UNABTO_MODULES_SRC_DIR}/fingerprint_acl/fp_acl_file.c
  ${UNABTO_MODULES_SRC_DIR}/fingerprint_acl/fp_acl.c
  )


set(nabto_src
  ${unabto_module_util_src}
  ${unabto_module_fp_src}
  ${UNABTO_MODULES_SRC_DIR}/cli/gopt/gopt.c
  ${UNABTO_MODULES_SRC_DIR}/cli/gopt/gopt.h
  ${unabto_module_read_hex}
  ${unabto_module_log_unix_src}
  )

add_definitions(${unabto_definitions})


# Called by developer - build and install

add_executable(aclcli ${nabto_src} ${aclcli_src}/unabto_acl.c)

target_link_libraries(aclcli ${unabto_link_libraries})


install( TARGETS aclcli
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
