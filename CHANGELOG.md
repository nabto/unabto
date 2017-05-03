# Change Log

All notable changes to this project will be documented in this
file. This projects changelog started with version [3.0.15] 2017-02-21
for change logs prior to this date contact nabto.

The format is based on [Keep a Changelog](http://keepachangelog.com/)

Guide: always keep an unreleased section which keeps track of current
changes. When a release is made the unreleased section is renamed to
the release and a new unreleased section is added.

## 4.1.0 Unreleased

...


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


