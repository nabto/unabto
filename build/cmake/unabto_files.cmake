# CMake scipt: Define Nabto directory locations and lists of files.
# set(UNABTO_ROOT ...) to location of unabto directory before including this cmake script.


# Naming convention all source files lists ends with _src all header
# files lists ends with _headers. The source file lists name starts
# with unabto then a series of labels decribe the placement in the
# source tree.
#
# e.g. unabto_module_crypto_generic_src,
# unabto_3rdparty_libtomcrypt_src

# Define different include directories to be included in a project file
set(UNABTO_INCLUDE_DIR      ${UNABTO_ROOT}/src)

# Define different root directories containing source code to be included in a project file
set(UNABTO_MODULES_SRC_DIR   ${UNABTO_ROOT}/src/modules)
set(UNABTO_PLATFORMS_SRC_DIR ${UNABTO_ROOT}/src/platforms)
set(UNABTO_SERVER_SRC_DIR    ${UNABTO_ROOT}/src)

# Other useful directories
set(UNABTO_SERVER_TEST_DIR ${UNABTO_ROOT}/test)
set(UNABTO_DEMO_DIR ${UNABTO_ROOT}/apps)
set(UNABTO_DEMO_COMMON_DIR ${UNABTO_DEMO_DIR}/common)

###############################
# UNABTO CORE SRC DEFINITIONS #
###############################

set(unabto_core_src 
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_app_adapter.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_attach.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/util/unabto_buffer.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_connection.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_context.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_crypto.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_message.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_packet.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_packet_util.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/util/unabto_queue.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_event.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_window.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_environment.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_nano_stream.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_common_main.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_next_event.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_prf.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_aes128_sha256.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_buffers.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_query_rw.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_tcp_fallback.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_debug_packet.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_extended_rendezvous.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_aes128_sha256.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_aes_cbc.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_app_adapter.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_app.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_attach.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_buffers.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_common_main.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_config_check.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_config_defaults.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_config_derived.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_connection.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_connection_type.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_context.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_crypto.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_debug_packet.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_dns_fallback.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_env_base.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_environment.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_extended_rendezvous.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_external_environment.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_hmac_sha256.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_include_platform.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_logging_defines.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_logging.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_main_contexts.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_memory.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_message.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_next_event.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_packet.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_packet_util.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_prf.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_protocol_defines.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_protocol_exceptions.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_query_rw.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_environment.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_event.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_types.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_window.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_tcp_fallback.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_util.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/util/unabto_buffer.h
  ${UNABTO_SERVER_SRC_DIR}/unabto/util/unabto_queue.h
  )
source_group(unabto FILES ${unabto_core_src})

set(unabto_core_include_directories
  ${UNABTO_INCLUDE_DIR}
)


###################################
# UNABTO PLATFORM SRC DEFINITIONS #
###################################

# Unix platform definitions
set(unabto_platform_unix_include_directories ${UNABTO_PLATFORMS_SRC_DIR}/unix)
set(unabto_platform_unix_src
  ${UNABTO_PLATFORMS_SRC_DIR}/unix/unabto_environment_unix.c
  ${UNABTO_PLATFORMS_SRC_DIR}/unix/unabto_platform_types.h
  ${UNABTO_PLATFORMS_SRC_DIR}/unix/unabto_platform.h)
source_group(platforms\\unix FILES ${unabto_platform_unix_src})


# Win32 platform definitions
set(unabto_platform_win32_include_directories ${UNABTO_PLATFORMS_SRC_DIR}/win32)
set(unabto_platform_win32_src
  ${UNABTO_PLATFORMS_SRC_DIR}/win32/unabto_platform.c
  ${UNABTO_PLATFORMS_SRC_DIR}/win32/unabto_logging_win32.h
  ${UNABTO_PLATFORMS_SRC_DIR}/win32/unabto_platform.h
  ${UNABTO_PLATFORMS_SRC_DIR}/win32/unabto_platform_types.h)
source_group(platforms\\win32 FILES ${unabto_platform_win32_src})

#################################
# UNABTO MODULE SRC DEFINITIONS #
#################################

# modules/application_event

set(unabto_module_application_event_dummy_src
  ${UNABTO_MODULES_SRC_DIR}/application_event/dummy/application_event_dummy.c)

# modules/coap
set (unabto_module_coap_src
  ${UNABTO_MODULES_SRC_DIR}/coap/er-coap-13/er-coap-13-engine.c
  ${UNABTO_MODULES_SRC_DIR}/coap/er-coap-13/er-coap-13-observing.c
  ${UNABTO_MODULES_SRC_DIR}/coap/er-coap-13/er-coap-13-separate.c
  ${UNABTO_MODULES_SRC_DIR}/coap/er-coap-13/er-coap-13-transactions.c
  ${UNABTO_MODULES_SRC_DIR}/coap/er-coap-13/er-coap-13.c
  ${UNABTO_MODULES_SRC_DIR}/coap/etimer.c
  ${UNABTO_MODULES_SRC_DIR}/coap/unabto_uip.c
  ${UNABTO_MODULES_SRC_DIR}/coap/list.c
  ${UNABTO_MODULES_SRC_DIR}/coap/erbium/erbium.c
  ${UNABTO_MODULES_SRC_DIR}/coap/er_coap_nabto.c)

set (unabto_module_coap_includes
  ${UNABTO_MODULES_SRC_DIR}/coap
  ${UNABTO_MODULES_SRC_DIR}/coap/er-coap-13
  ${UNABTO_MODULES_SRC_DIR}/coap/erbium)


# modules/crypto
set(unabto_module_crypto_generic_src
  ${UNABTO_MODULES_SRC_DIR}/crypto/generic/unabto_aes.c
  ${UNABTO_MODULES_SRC_DIR}/crypto/generic/unabto_aes_cbc.c
  ${UNABTO_MODULES_SRC_DIR}/crypto/generic/unabto_hmac_sha256.c
  ${UNABTO_MODULES_SRC_DIR}/crypto/generic/unabto_sha256.c
  ${UNABTO_MODULES_SRC_DIR}/crypto/generic/unabto_sha256.h
  ${UNABTO_MODULES_SRC_DIR}/crypto/generic/unabto_aes.h)
source_group(modules\\crypto\\generic FILES ${unabto_module_crypto_generic_src})

  
set(unabto_module_crypto_openssl_src
  ${UNABTO_MODULES_SRC_DIR}/crypto/openssl/unabto_crypto_openssl.c)
source_group(modules\\crypto\\openssl FILES ${unabto_module_crypto_openssl_src})


set(unabto_module_crypto_openssl_minimal_armv4_src
  ${UNABTO_ROOT}/3rdparty/openssl_armv4/unabto_openssl_minimal_sha256.c
  ${UNABTO_MODULES_SRC_DIR}/crypto/openssl_armv4/unabto_openssl_minimal_hmac_sha256.c
  ${UNABTO_ROOT}/3rdparty/openssl_armv4/asm/sha256-armv4.S
  ${UNABTO_ROOT}/3rdparty/openssl_armv4/asm/aes-armv4.S
  ${UNABTO_MODULES_SRC_DIR}/crypto/openssl_armv4/unabto_openssl_minimal_aes_cbc.c)
set(unabto_module_random_openssl_minimal_armv4_src
  ${UNABTO_ROOT}/3rdparty/openssl_armv4/asm/aes-armv4.S
  ${UNABTO_MODULES_SRC_DIR}/crypto/openssl_armv4/unabto_openssl_minimal_aes_cbc.c
  ${UNABTO_MODULES_SRC_DIR}/crypto/openssl_armv4/unabto_openssl_minimal_random.c)
source_group(modules\\crypto\\openssl_armv4 FILES ${unabto_random_module_openssl_minimal_armv4_src} ${unabto_module_crypto_openssl_minimal_armv4_src})


set(unabto_module_crypto_libtomcrypt_src
  ${UNABTO_MODULES_SRC_DIR}/crypto/libtomcrypt/unabto_libtomcrypt.c)
source_group(modules\\crypto\\libtomcrypt FILES ${unabto_module_crypto_libtomcrypt_src})


# modules/cli
set(unabto_module_cli_gopt_src
  ${UNABTO_MODULES_SRC_DIR}/cli/gopt/gopt.c
  ${UNABTO_MODULES_SRC_DIR}/cli/gopt/gopt.h
  ${UNABTO_MODULES_SRC_DIR}/cli/gopt/unabto_args_gopt.c
  ${UNABTO_MODULES_SRC_DIR}/diagnostics/unabto_diag.c)
source_group(modules\\cli\\gopt FILES ${unabto_module_cli_gopt_src})

# modules/diagnostics
set(unabto_module_diagnostics_src
  ${UNABTO_MODULES_SRC_DIR}/diagnostics/unabto_diag.c
  ${UNABTO_MODULES_SRC_DIR}/diagnostics/unabto_diag.h)
source_group(modules\\diagnostics FILES ${unabto_module_diagnostics_src})



# modules/dns
set(unabto_module_unix_dns_src
  ${UNABTO_MODULES_SRC_DIR}/dns/unix/unabto_unix_dns.c)
source_group(modules\\dns\\unix FILES ${unabto_module_unix_dns_src})


set(unabto_module_dns_win32_src
  ${UNABTO_MODULES_SRC_DIR}/dns/win32/unabto_win32_dns.c)
source_group(modules\\dns\\win32 FILES ${unabto_module_dns_win32_src})


# modules/dns_fallback

set(unabto_module_dns_fallback_src
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_protocol.c
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_session.c
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_data.c
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_probe.c
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_client.c
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_impl.c
  ${UNABTO_MODULES_SRC_DIR}/util/unabto_base32.c
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_client.h
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_data.h
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_probe.h
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_protocol.h
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_select.h
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_session.h
  ${UNABTO_MODULES_SRC_DIR}/dns_fallback/unabto_dns_fallback_socket.h)

source_group(modules\\dns_fallback FILES ${unabto_module_dns_fallback_src})




# modules/log
set(unabto_module_log_syslog_src
  ${UNABTO_MODULES_SRC_DIR}/log/syslog/unabto_syslog.c
  ${UNABTO_MODULES_SRC_DIR}/log/syslog/unabto_syslog.h)
source_group(modules\\log\\syslog FILES ${unabto_module_log_syslog_src})


set(unabto_module_log_dynamic_src
  ${UNABTO_MODULES_SRC_DIR}/log/dynamic/unabto_dynamic_log.c
  ${UNABTO_MODULES_SRC_DIR}/log/dynamic/unabto_dynamic_log_util.c
  ${UNABTO_MODULES_SRC_DIR}/log/dynamic/unabto_dynamic_log.h
  ${UNABTO_MODULES_SRC_DIR}/log/dynamic/unabto_dynamic_log_util.h)
source_group(modules\\log\\dynamic FILES ${unabto_module_log_dynamic_src})


set(unabto_module_log_unix_src
  ${UNABTO_MODULES_SRC_DIR}/log/unix/unabto_log_header_unix.c
  ${UNABTO_PLATFORMS_SRC_DIR}/unabto_printf_logger.c
  ${UNABTO_MODULES_SRC_DIR}/log/unabto_basename.c)

set(unabto_module_log_varargs_to_cstring_src
  ${UNABTO_MODULES_SRC_DIR}/log/unabto_varargs_to_cstring.c)

set(unabto_module_log_win32_src
  ${UNABTO_MODULES_SRC_DIR}/log/win32/unabto_log_header_win32.c
  ${UNABTO_PLATFORMS_SRC_DIR}/unabto_printf_logger.c
  ${UNABTO_MODULES_SRC_DIR}/log/unabto_basename.c)
source_group(modules\\log\\win32 FILES ${unabto_module_log_win32_src})

# modules/network
set(unabto_module_winsock_src
  ${UNABTO_MODULES_SRC_DIR}/network/winsock/unabto_winsock.c
  ${UNABTO_MODULES_SRC_DIR}/network/winsock/unabto_winsock.h)
set (unabto_module_winsock_dns_src
  ${UNABTO_MODULES_SRC_DIR}/network/winsock/unabto_winsock_dns.c)
source_group(modules\\network\\winsock FILES ${unabto_module_winsock_src} ${unabto_module_winsock_dns_src})


set(unabto_module_dns_client_src
  ${UNABTO_MODULES_SRC_DIR}/network/dns/dns_client.c
  ${UNABTO_MODULES_SRC_DIR}/network/dns/dns_client_wrapper.c)
source_group(modules\\network\\dns_client FILES ${unabto_module_dns_client_src})

set(unabto_module_network_bsd_src
  ${UNABTO_MODULES_SRC_DIR}/network/bsd/unabto_network_bsd.c)

set(unabto_module_network_epoll_src
  ${UNABTO_MODULES_SRC_DIR}/network/epoll/unabto_epoll.c
  ${UNABTO_MODULES_SRC_DIR}/network/epoll/unabto_epoll.h)

# modules/random
set(unabto_module_random_dummy_src
  ${UNABTO_MODULES_SRC_DIR}/random/dummy/unabto_random_dummy.c)
source_group(modules\\random\\dummy FILES ${unabto_module_random_dummy_src})


set(unabto_module_openssl_random_src
  ${UNABTO_MODULES_SRC_DIR}/random/openssl/unabto_random_openssl.c)
source_group(modules\\random\\openssl FILES ${unabto_module_openssl_random_src})


set(unabto_module_random_libtomcrypt_src
  ${UNABTO_MODULES_SRC_DIR}/random/ltc/unabto_random_ltc.c)
source_group(modules\\random\\libtomcrypt FILES ${unabto_module_random_libtomcrypt_src})


# modules/tcp_fallback
set(unabto_module_tcp_fallback_src
  ${UNABTO_MODULES_SRC_DIR}/tcp_fallback/tcp_fallback_select.c
  ${UNABTO_MODULES_SRC_DIR}/tcp_fallback/tcp_fallback_select.h)
source_group(modules\\tcp_fallback FILES ${unabto_module_tcp_fallback_src})


# modules/timers
set(unabto_module_timers_unix_src
  ${UNABTO_MODULES_SRC_DIR}/timers/unix/unabto_unix_time.c)

set(unabto_module_timers_tick_timer_src
  ${UNABTO_MODULES_SRC_DIR}/timers/tick_timer/unabto_tick_timer.c)


# modules/provision
set(unabto_module_provision_src
  ${UNABTO_MODULES_SRC_DIR}/provision/unabto_provision.c
  ${UNABTO_MODULES_SRC_DIR}/provision/unabto_provision_http.c
  ${UNABTO_MODULES_SRC_DIR}/provision/unabto_provision_http_curl.c
  ${UNABTO_MODULES_SRC_DIR}/provision/unabto_provision_file.c
  ${UNABTO_MODULES_SRC_DIR}/provision/unabto_provision_gopt.c)


#######################
# DEMO SOURCE SECTION #
#######################

set(unabto_server_weather_station
  ${UNABTO_DEMO_DIR}/weather_station/unabto_main.c
  ${UNABTO_DEMO_DIR}/weather_station/unabto_weather_station.c
)

set(unabto_weather_station_test
  ${UNABTO_DEMO_DIR}/weather_station/unabto_weather_station_test.c
)

###########################
# 3RDPARTY SOURCE SECTION #
###########################

set(unabto_3rdparty_libtomcrypt_src
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/ciphers/aes/aes.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/modes/cbc/cbc_decrypt.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/modes/cbc/cbc_done.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/modes/cbc/cbc_encrypt.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/modes/cbc/cbc_start.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/hashes/sha2/sha256.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/hashes/helper/hash_memory.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/mac/hmac/hmac_init.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/mac/hmac/hmac_process.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/mac/hmac/hmac_done.c

  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_argchk.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_cipher_is_valid.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_cipher_descriptor.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_find_cipher.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_register_cipher.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_register_hash.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_find_hash.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_hash_is_valid.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_hash_descriptor.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_find_prng.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_prng_is_valid.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_prng_descriptor.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/crypt/crypt_register_prng.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/prngs/fortuna.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/prngs/rng_get_bytes.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/prngs/rng_make_prng.c
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/misc/zeromem.c
  )
set(unabto_3rdparty_libtomcrypt_include_directories
  ${UNABTO_ROOT}/3rdparty/libtomcrypt/src/headers)

source_group(3rdparty\\libtomcrypt\\src FILES ${unabto_3rdparty_libtomcrypt_src})

#######################
# TEST SOURCE SECTION #
#######################
set(unabto_server_common_src_test
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_buffers.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_query_rw.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/util/unabto_buffer.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/util/unabto_queue.c
  ${UNABTO_SERVER_TEST_DIR}/modules/crypto/generic/unabto_sha256_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/unabto_hmac_sha256_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/unabto_test.c
  ${UNABTO_SERVER_TEST_DIR}/modules/crypto/generic/unabto_aes_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/unabto_aes_cbc_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/unabto_prfplus_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/unabto_aes128_sha256_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/util/unabto_util_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/util/unabto_buffer_test.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/unabto_crypto_test.c
  ${UNABTO_SERVER_TEST_DIR}/modules/util/unabto_base32_test.c
  ${UNABTO_SERVER_SRC_DIR}/modules/util/unabto_base32.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_crypto.c
  ${unabto_module_random_dummy}
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_prf.c
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_aes128_sha256.c
)

set(unabto_server_stream_test_src
  ${UNABTO_SERVER_SRC_DIR}/unabto/unabto_stream_event.c
  ${UNABTO_SERVER_TEST_DIR}/unabto/unabto_stream_event_test.c
  ${unabto_core_src}
)
