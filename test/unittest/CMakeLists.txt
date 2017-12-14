project(unabto_unittest)

#unittest project which builds all unittests for windows/linux/mac platforms.


set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
include(${UNABTO_ROOT}/build/cmake/unabto_files.cmake)

include_directories(
  ${unabto_core_include_directories}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${UNABTO_SERVER_TEST_DIR}
)



set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

set(src
  ${unabto_crypto_src}
  ${unabto_module_random_dummy_src}
  ${unabto_server_common_src_test}
  ${unabto_server_stream_test_src}
  ${unabto_module_application_event_dummy_src}
  ${unabto_module_crypto_generic_src}
  ${unabto_module_fingerprint_acl_src}
  
  ${src_dir}/unabto_unittest.c)

add_definitions("-DNABTO_ENABLE_TEST_CODE")

if (MSVC)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  include_directories(${unabto_platform_win32_include_directories})
  list(APPEND src
    ${unabto_platform_win32_src}
    ${unabto_module_log_win32_src}
    ${unabto_module_winsock_src}
    ${unabto_module_dns_win32_src}
  )
else()
  include_directories(${unabto_platform_unix_include_directories})
  list(APPEND src
    ${unabto_platform_unix_src}
    ${unabto_module_log_unix_src}
    ${unabto_module_timers_unix_src}
    ${unabto_module_network_bsd_src}
    ${unabto_module_unix_dns_src}
  )
endif()


add_executable(${PROJECT_NAME} ${src})

if (MSVC)
  target_link_libraries(${PROJECT_NAME} Ws2_32)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(${PROJECT_NAME} pthread rt m)
else()
  target_link_libraries(${PROJECT_NAME} pthread m)
endif()
  
install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
