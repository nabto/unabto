cmake_minimum_required(VERSION 2.8)
project(unabto-tunnel C ASM)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

option(USE_LIB_PROFILER "Link executables with libprofiler.so" OFF)
option(LOG_ALL "Print all unabto log statements" OFF)
option(USE_STUN_CLIENT "Enable a stun client in the unabto server" OFF)
option(USE_TEST_WEBSERVER "Enable a mongoose test webserver" OFF)

include(${UNABTO_ROOT}/build/cmake/unabto_project.cmake)

if(EXTERNAL_PREFIX) 
  include_directories(${EXTERNAL_PREFIX}/include)
  link_directories(${EXTERNAL_PREFIX}/lib)
endif()

set(unabto_server_unabto_tunneling_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(
  # make it possible to override unabto_config.h with a supplied file
  # to the build tree.
  ${CMAKE_CURRENT_BINARY_DIR}

  ${unabto_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

# build the tunnelling library with the unabto_config.h defined in this project.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/tunnel ${CMAKE_CURRENT_BINARY_DIR}/tunnel_library)


source_group(tunnel\\src FILES ${unabto_tunnel_src})

set(tunnel_src
  )

if (USE_TEST_WEBSERVER)
  list(APPEND tunnel_src
	${unabto_server_unabto_tunneling_src}/mongoose/mongoose.c
	${unabto_server_unabto_tunneling_src}/test_webserver.c
	)
  
  add_definitions(-DUSE_TEST_WEBSERVER=1)
endif()


if (NOT WIN32 AND USE_STUN_CLIENT)
  add_definitions(-DUSE_STUN_CLIENT)
  list(APPEND tunnel_src
    ${UNABTO_MODULES_SRC_DIR}/mt/unabto_cond.c
    ${UNABTO_MODULES_SRC_DIR}/mt/unabto_thread.c
    ${UNABTO_MODULES_SRC_DIR}/mt/unabto_mutex.c
    ${unabto_server_unabto_tunneling_src}/tunnel_application_requests.c
    ${unabto_server_unabto_tunneling_src}/stun/stun.cxx
    ${unabto_server_unabto_tunneling_src}/stun/udp.cxx
    ${unabto_server_unabto_tunneling_src}/stun/stuntest.cpp)
endif()

if (NOT WIN32)
    SET(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -static-libgcc -static-libstdc++")
endif()

add_definitions(${unabto_definitions})

if (USE_LIB_PROFILER)
  list(APPEND unabto_link_libraries profiler)
endif()

# Called by developer - build and install

add_executable(unabto_tunnel ${tunnel_src} ${unabto_server_unabto_tunneling_src}/main.c)
add_executable(custom_tunnel_demo ${tunnel_src} ${unabto_server_unabto_tunneling_src}/custom_main.c)

target_link_libraries(unabto_tunnel unabto_tunnel_library ${unabto_link_libraries})
target_link_libraries(custom_tunnel_demo unabto_tunnel_library ${unabto_link_libraries})


install( TARGETS unabto_tunnel
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
