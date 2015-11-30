cmake_minimum_required(VERSION 2.8)
project(urrdtool_test)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)


set(src
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unix_platform.c
  ${CMAKE_CURRENT_SOURCE_DIR}/CuTest.c
  ${UNABTO_ROOT}/src/modules/urrdtool/urrdtool.c
  ${UNABTO_ROOT}/src/modules/urrdtool/urrdtool_backend_eeprom.c
)

set(unix_file_src
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/urrd_file_backend.c
  ${CMAKE_CURRENT_SOURCE_DIR}/CuTest.c
  ${UNABTO_ROOT}/src/modules/urrdtool/urrdtool.c
)

set(cfs_backend_src
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cfs/cfs-posix.c
  ${CMAKE_CURRENT_SOURCE_DIR}/CuTest.c
  ${UNABTO_ROOT}/src/modules/urrdtool/urrdtool.c
  ${UNABTO_ROOT}/src/modules/urrdtool/urrdtool_backend_cfs.c
)

set(cfs_coffee_backend_src
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cfs-coffee/cfs-coffee.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cfs-coffee/cfs-coffee-unix.c
  ${CMAKE_CURRENT_SOURCE_DIR}/CuTest.c
  ${UNABTO_ROOT}/src/modules/urrdtool/urrdtool.c
  ${UNABTO_ROOT}/src/modules/urrdtool/urrdtool_backend_cfs.c
)

include_directories(
  ${UNABTO_ROOT}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cfs
)

add_definitions("-Wall")

add_executable(${PROJECT_NAME}_flat ${src})
set_target_properties(${PROJECT_NAME}_flat PROPERTIES COMPILE_DEFINITIONS "URRD_BACKEND_EEPROM=1")

add_executable(${PROJECT_NAME}_unix_file ${unix_file_src})
set_target_properties(${PROJECT_NAME}_unix_file PROPERTIES COMPILE_DEFINITIONS "URRD_BACKEND_FILE=1")

add_executable(${PROJECT_NAME}_cfs ${cfs_backend_src})
set_target_properties(${PROJECT_NAME}_cfs PROPERTIES COMPILE_DEFINITIONS "URRD_BACKEND_CFS=1")

add_executable(${PROJECT_NAME}_cfs_coffee ${cfs_coffee_backend_src})
set_target_properties(${PROJECT_NAME}_cfs_coffee PROPERTIES COMPILE_DEFINITIONS "URRD_BACKEND_CFS=1")




#target_link_libraries(${PROJECT_NAME})