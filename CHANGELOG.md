# Change Log

All notable changes to this project will be documented in this
file. This projects changelog started with version [3.0.15] 2017-02-21
for change logs prior to this date contact nabto.

The format is based on [Keep a Changelog](http://keepachangelog.com/)

Guide: always keep an unreleased section which keeps track of current
changes. When a release is made the unreleased section is renamed to
the release and a new unreleased section is added.

## 4.5 UNRELEASED

## 4.4.3 2019-06-25

- NABTO-1947: Fixed overflow in streaming buffer calculations.

## 4.4.2 2019-04-01

### Changed
- NABTO-1925: Fixed timestamp overflow causing devices to not be reachable after having been attached for 49 days to the basestation.


## 4.4.0 2018-10-21

### Changed
- In streaming is the segment pool concept changed to also include
  recv segments. This has some configuration implications.

### Breaking Changes
- NABTO-1891: replaced configuration options
  `NABTO_STREAM_SEND_SEGMENT_SIZE` and
  `NABTO_STREAM_RECEIVE_SEGMENT_SIZE` with the option
  `NABTO_STREAM_SEGMENT_SIZE` to simplify configuration of the
  streaming.
- NABTO-1891: configuration option
  `NABTO_STREAM_SEND_SEGMENT_POOL_SIZE` is renamed to
  `NABTO_STREAM_SEGMENT_POOL_SIZE` and does now also include recv
  segments.

### Added
- NABTO-1891: added configuration option
  `NABTO_STREAM_SEGMENT_POOL_MAX_RECEIVE_SEGMENTS`

## 4.3.0 2018-10-02

### Changed
- NABTO-1850: The streaming implementation now uses a pool of send
  segments.

### Breaking Changes
- NABTO-1187: all `uint32_t` ip addresses is now `struct
  nabto_ip_address` this affects the following function definitions:
  `nabto_read`, `nabto_write`, `nabto_get_local_ip`,
  `nabto_dns_is_resolved`
- NABTO-1187: `nabto_init_socket` no longer takes the `uint32_t
  localAddr` argument.
- NABTO-1187: function renamed:
  `nabto_init_socket`->`nabto_socket_init`
  `nabto_close_socket`->`nabto_socket_close`
- NABTO-1187: `nabto_get_local_ip` function renamed to
  `nabto_get_local_ipv4`
- NABTO-1187: `nabto_resolve_ipv4`, `nabto_socket_set_invalid`,
  `nabto_socket_is_equal` functions added and must be implemented on
  all platforms
- `aes128_cbc_encrypt` and `aes128_cbc_decrypt` renamed to
  `unabto_aes128_cbc_encrypt` and `unabto_aes128_cbc_decrypt`

### Added
- NABTO-1850: The function `nabto_stream_event` can not be called with
  an additional event.
- NABTO-1850: The `unabto_config.h` has got an additional configuration
  option to control the size of the streaming send segment
  pool. `NABTO_STREAM_SEND_SEGMENT_POOL_SIZE`

## 4.2.0 2017-12-14

### Breaking Changes
- in file `src/unabto/unabto_app.h` the function argument
  unabto_query_request* readBuffer has been removed from the function
  `application_poll` the argument was always NULL. It has been removed
  to limit confusion of how the api can be used.
- the module `stateful_push_service` was renamed to `push_service`
- Using unabto_push no longer requires `unabto_push_init()` to be called

## 4.1.0 2017-07-11

### Added
### Removed

### Changed
- NABTO-1480: triple acks were not handled correct, this fix improves streaming throughput.
- NABTO-1542: dns interface has been changed to be able to accept and array of resolved ipv4 addresses.

### Breaking Changes
- unabto_tunnel takes a new commandline format.


## 4.0.0 2017-04-28

### Added
- Stream stats has been added whenever a stream ends a packet with
  statistics information is sent to the basestation.
- Firebase push notification support.
- Webhooks push notification support.

### Removed
- The arduino demo has been moved to (https://github.com/nabto/unabto-arduino-sdk)

### Breaking changes
- The W5100 driver interface has changed.
- Changed version scheme to semver hence the bump of version from 3.x.x to 4.x.x.

## 3.0.15 2017-02-23

### Added
- Fingerprints, such that clients can connect with self signed certificates.
- Fingerprint acl module in `src/modules/fingerprint_acl`
- Multi Tunnel tool, a tool which can act as both tcp, uart and echo tunnel on linux. `apps/multi_tunnel`

### Removed
- Nano streaming. Use micro streaming which has always been the default streaming module.
- PIC32 demo is moved to (https://github.com/nabto/unabto-pic32-sdk)
- PIC18 demo has been removed as xc8 did not support the mla ip stack.
- Nabduino demo has been removed since the PIC18 with c18 compiler support was removed.

### Deprecated
- html_dd applications. See client change logs and documentation.
- Browser plugins. See client change logs and documentation.

### Breaking Changes
- Tunnel module has been changed. Read readme in the `src/modules/tunnel`
- The deprecated `buffer_read_t` and `buffer_write_t` type definitions in the `application_event()` callback have been replaced with `unabto_query_request` and `unabto_query_response`, respectively. This means that calls to `buffer_read_<TYPE>` and `buffer_write_<TYPE>` must be replaced with `unabto_query_read_<TYPE>` and `unabto_query_write_<TYPE>`, respectively.
