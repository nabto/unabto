cmake_minimum_required(VERSION 2.8)
project(tunnel-package)

set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${UNABTO_ROOT}/build/cmake/unabto_version.cmake)

SET(CMAKE_INSTALL_PREFIX_PATH "/")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/")

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/unabto_tunnel
  DESTINATION etc/init.d
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/unabto_tunnel.conf
  DESTINATION etc)


# Called from nabto/unabto/build/unix/package.sh
install(PROGRAMS ${INSTALL_ROOT}/unabto_tunnel
  DESTINATION usr/bin
  )

set(conffiles_file "${CMAKE_CURRENT_SOURCE_DIR}/conffiles")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${conffiles_file})


# Called from nabto/unabto/build/unix/package.sh
if(${PLATFORM_NAME} MATCHES "64")
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
else()
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
endif()
set(CPACK_SYSTEM_NAME ${PLATFORM_NAME})
if(PLATFORM_NAME MATCHES win)
  set(CPACK_GENERATOR ZIP)
elseif(PLATFORM_NAME MATCHES mac)
  set(CPACK_GENERATOR TGZ)
else()
  set(CPACK_GENERATOR TGZ DEB)
endif()

set(CPACK_PROJECT_NAME unabto-tunnel)
set(CPACK_PACKAGE_CONTACT info@nabto.com)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Nabto micro tunnel")

include(CPack)