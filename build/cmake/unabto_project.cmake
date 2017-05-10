# This project makes a default project for unabto.
# It finds the default platform.
# A default random module, can be overriden by UNABTO_RANDOM_MODULE
# A default crypto module, can be overridden by UNABTO_CRYPTO_MODULE
# A default network module, can be overridden by UNABTO_NETWORK_MODULE


# UNABTO_RANDOM_MODULE
# possibilities: dummy openssl libtomcrypt openssl_armv4 none

# UNABTO_CRYPTO_MODULE
# possibilities: generic openssl libtomcrypt openssl_armv4 none

# UNABTO_NETWORK_MODULE
# possibilities: bsd winsock none

# UNABTO_DNS_MODULE
# possibilities win32 unix builtin none

# An external library dir can be added such that e.g. an openssl library can be found there
# Specify the cmake option UNABTO_EXTERNAL_BUILD_DIR to give a hint about the library location.

set(UNABTO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

include(${UNABTO_ROOT}/build/cmake/unabto_files.cmake)
include (CheckIncludeFiles)
include (CheckFunctionExists)
INCLUDE (CheckLibraryExists)

set(unabto_src
  ${unabto_core_src}
)

set(unabto_include_directories
  ${unabto_core_include_directories}
)

set(unabto_definitions

)


set(unabto_module_provision_libraries
   curl
)

set(unabto_link_libraries

)

# Detect crypto suite
CHECK_INCLUDE_FILES (openssl/rand.h HAVE_SYSTEM_OPENSSL_RANDOM_H)
CHECK_INCLUDE_FILES (openssl/evp.h HAVE_SYSTEM_OPENSSL_EVP_H)
CHECK_INCLUDE_FILES (openssl/hmac.h HAVE_SYSTEM_OPENSSL_HMAC_H)

if (EXISTS ${UNABTO_EXTERNAL_BUILD_ROOT}/include/openssl/rand.h)
  message("openssl/rand.h found in ${UNABTO_EXTERNAL_BUILD_ROOT}")
  set(HAVE_OPENSSL_RANDOM_H 1)
endif()
if (EXISTS ${UNABTO_EXTERNAL_BUILD_ROOT}/include/openssl/evp.h)
  message("openssl/evp.h found in ${UNABTO_EXTERNAL_BUILD_ROOT}")
  set(HAVE_OPENSSL_EVP_H 1)
endif()
if (EXISTS ${UNABTO_EXTERNAL_BUILD_ROOT}/include/openssl/hmac.h)
  message("openssl/hmac.h found in ${UNABTO_EXTERNAL_BUILD_ROOT}")
  set(HAVE_OPENSSL_HMAC_H)
endif()

if (UNABTO_EXTERNAL_BUILD_ROOT AND EXISTS ${UNABTO_EXTERNAL_BUILD_ROOT}/lib/libcrypto.so)
  SET(OPENSSL_CRYPTO_LIBRARY ${UNABTO_EXTERNAL_BUILD_ROOT}/lib/libcrypto.so)
elseif(UNABTO_EXTERNAL_BUILD_ROOT AND EXISTS ${UNABTO_EXTERNAL_BUILD_ROOT}/lib/libcrypto.a)
  SET(OPENSSL_CRYPTO_LIBRARY ${UNABTO_EXTERNAL_BUILD_ROOT}/lib/libcrypto.a)
else()
  SET(OPENSSL_CRYPTO_LIBRARY crypto)
endif()

if (UNABTO_EXTERNAL_BUILD_ROOT)
  list(APPEND unabto_include_directories ${UNABTO_EXTERNAL_BUILD_ROOT}/include)
endif()

if (EXISTS ${UNABTO_EXTERNAL_BUILD_ROOT}/lib/libcrypto.so OR EXISTS ${UNABTO_EXTERNAL_BUILD_ROOT}/lib/libcrypto.a)
  set(HAVE_OPENSSL_EXTERNAL_CRYPTO_LIB 1)
endif()

if (NOT UNABTO_RANDOM_MODULE)
  if (HAVE_OPENSSL_RANDOM_H OR HAVE_OPENSSL_EXTERNAL_CRYPTO_LIB OR HAVE_SYSTEM_OPENSSL_RANDOM_H)
    set(UNABTO_RANDOM_MODULE openssl)
  else()
    set(UNABTO_RANDOM_MODULE libtomcrypt)
  endif()
endif()

if (NOT UNABTO_CRYPTO_MODULE)
  if ((HAVE_OPENSSL_EVP_H AND HAVE_OPENSSL_HMAC_H) OR HAVE_OPENSSL_EXTERNAL_CRYPTO_LIB OR (HAVE_SYSTEM_OPENSSL_EVP_H AND HAVE_SYSTEM_OPENSSL_HMAC_H))
    set(UNABTO_CRYPTO_MODULE openssl)
  else()
    set(UNABTO_CRYPTO_MODULE libtomcrypt)
  endif()
endif()

if (NOT UNABTO_NETWORK_MODULE)
  if (WIN32)
    set(UNABTO_NETWORK_MODULE winsock)
  else()
    set(UNABTO_NETWORK_MODULE bsd)
  endif()
endif()

if (NOT UNABTO_DNS_MODULE)
  if (WIN32)
    set(UNABTO_DNS_MODULE win32)
  else()
    set(UNABTO_DNS_MODULE unix)
  endif()
endif()


#detect epoll
if (NOT UNABTO_DISABLE_EPOLL)
  CHECK_INCLUDE_FILES("sys/epoll.h" UNABTO_HAVE_EPOLL)
endif()

if (UNABTO_RANDOM_MODULE MATCHES dummy)
  list(APPEND unabto_src ${unabto_module_random_dummy_src})
elseif(UNABTO_RANDOM_MODULE MATCHES openssl_armv4)
  list(APPEND unabto_src ${unabto_module_random_openssl_minimal_armv4_src})
elseif(UNABTO_RANDOM_MODULE MATCHES openssl)
  list(APPEND unabto_src ${unabto_module_openssl_random_src})
  
  list(APPEND unabto_link_libraries ${OPENSSL_CRYPTO_LIBRARY})

  list(APPEND unabto_link_libraries dl)
elseif(UNABTO_RANDOM_MODULE MATCHES libtomcrypt)
  list(APPEND unabto_include_directories ${unabto_3rdparty_libtomcrypt_include_directories})
  list(APPEND unabto_src ${unabto_module_random_libtomcrypt_src})
  list(APPEND unabto_src ${unabto_3rdparty_libtomcrypt_src})
  list(APPEND unabto_definitions -DLTC_NO_ASM)
else()
  message(WARNING "No random module")
endif()

if (UNABTO_CRYPTO_MODULE MATCHES generic)
  list(APPEND unabto_src ${unabto_module_crypto_generic_src})
elseif(UNABTO_CRYPTO_MODULE MATCHES libtomcrypt)
  list(APPEND unabto_include_directories ${unabto_3rdparty_libtomcrypt_include_directories})
  list(APPEND unabto_src ${unabto_module_crypto_libtomcrypt_src})
  list(APPEND unabto_src ${unabto_3rdparty_libtomcrypt_src})
  list(APPEND unabto_definitions -DLTC_NO_ASM)
  if (WIN32)
    list(APPEND unabto_definitions "-DLTC_NO_PROTOTYPES")
  endif()
  if (CMAKE_SYSTEM_NAME MATCHES "WindowsCE")
    list(APPEND unabto_link_libraries Coredll)
    list(APPEND unabto_definitions "-DARGTYPE=2")
  endif()
elseif(UNABTO_CRYPTO_MODULE MATCHES openssl_armv4)
  list(APPEND unabto_src ${unabto_module_crypto_openssl_minimal_armv4_src})
elseif(UNABTO_CRYPTO_MODULE MATCHES openssl)
  list(APPEND unabto_src ${unabto_module_crypto_openssl_src})
  list(APPEND unabto_link_libraries ${OPENSSL_CRYPTO_LIBRARY})
  list(APPEND unabto_link_libraries dl)
else()
  message(WARNING "No crypto module")
endif()

if (UNABTO_NETWORK_MODULE MATCHES bsd)
  list(APPEND unabto_src ${unabto_module_network_bsd_src})
elseif(UNABTO_NETWORK_MODULE MATCHES winsock)
  list(APPEND unabto_src ${unabto_module_winsock_src})
  list(APPEND unabto_definitions "-DFD_SETSIZE=512")
endif()

if (UNABTO_DNS_MODULE MATCHES unix)
  list(APPEND unabto_src ${unabto_module_unix_dns_src})
endif()

if (UNABTO_DNS_MODULE MATCHES win32)
  list(APPEND unabto_src ${unabto_module_winsock_dns_src})
endif()

if (UNABTO_DNS_MODULE MATCHES builtin)
  list(APPEND unabto_src ${unabto_module_builtin_dns_src})
endif()

if (UNABTO_EXTERNAL_BUILD_ROOT)
  list(APPEND unabto_include_directories ${UNABTO_EXTERNAL_BUILD_ROOT}/include)
endif()

list(APPEND unabto_src ${unabto_module_dns_fallback_src})
list(APPEND unabto_definitions "-DJSON_USE_EXCEPTION=0")
if(${CMAKE_SYSTEM} MATCHES Linux)

  # Glibc 2.17 and newer dows not use -lrt and moxa does not have the lib at all.
  CHECK_LIBRARY_EXISTS(rt clock_gettime "" HAVE_RT_LIB)
  CHECK_LIBRARY_EXISTS(pthread pthread_create "" HAVE_PHTREAD_LIB)

  list(APPEND unabto_include_directories
    ${unabto_platform_unix_include_directories}
    )

  list(APPEND unabto_src
    ${unabto_platform_unix_src}
    ${unabto_module_log_unix_src}
    ${unabto_module_timers_unix_src}
    )
  list(APPEND unabto_link_libraries m)

  if (HAVE_PHTREAD_LIB)
    list(APPEND unabto_link_libraries pthread)
  else()
    list(APPEND unabto_definitions -pthread)
  endif()

  if (MOXA)
  elseif (HAVE_RT_LIB)
    list(APPEND unabto_link_libraries rt)
  endif()

  if (UNABTO_HAVE_EPOLL)
    list(APPEND unabto_src ${unabto_module_network_epoll_src})
  endif()
  
endif()

if (WIN32)
  list(APPEND unabto_include_directories
    ${unabto_platform_win32_include_directories}
    )
  list(APPEND unabto_src
    ${unabto_core_src}
    ${unabto_module_cli_gopt_src}
    ${unabto_platform_win32_src}
    ${unabto_module_log_win32_src}
    )
  if (CMAKE_SYSTEM_NAME MATCHES "WindowsCE")
    list(APPEND unabto_link_libraries Ws2)
  else()
    list(APPEND unabto_link_libraries Ws2_32)
  endif()
  list(APPEND unabto_definitions -DWIN32_LEAN_AND_MEAN)
  list(APPEND unabto_definitions -D_CRT_SECURE_NO_WARNINGS)
  list(APPEND unabto_definitions -D_WINSOCK_DEPRECATED_NO_WARNINGS)

  option(MSVC_STATIC_RUNTIME "Build with static runtime libs (/MT)" ON)

  if (MSVC_STATIC_RUNTIME)
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()
  endif ()

endif()

if (APPLE OR IOS)
  list(APPEND unabto_include_directories
    ${unabto_platform_unix_include_directories}
    )

  list(APPEND unabto_src
    ${unabto_module_unix_dns_src}
    ${unabto_platform_unix_src}
    ${unabto_module_log_unix_src}
    ${unabto_module_timers_unix_src}
    )
  list(APPEND unabto_link_libraries pthread m)
endif()

message(STATUS "Configuration of uNabto")
message(STATUS "  Crypto module: " ${UNABTO_CRYPTO_MODULE})
message(STATUS "  Random module: " ${UNABTO_RANDOM_MODULE})
message(STATUS "  Network module: " ${UNABTO_NETWORK_MODULE})
