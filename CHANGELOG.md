# Change Log

All notable changes to this project will be documented in this
file. This projects changelog started with version [3.0.15] 2017-02-21
for change logs prior to this date contact nabto.

The format is based on [Keep a Changelog](http://keepachangelog.com/)

Guide: always keep an unreleased section which keeps track of current
changes. When a release is made the unreleased section is renamed to
the release and a new unreleased section is added.

## Unreleased

### Added
- Fingerprints, such that clients can connect with self signed certificates.
- Fingerprint acl module in `src/modules/fingerprint_acl`
- Multi Tunnel tool, a tool which can act as both tcp, uart and echo tunnel on linux. `apps/multi_tunnel`

### Removed
- Nano streaming. Use micro streaming which has always been the default streaming module.

### Deprecated
- html_dd applications. See client change logs and documentation.
- Browser plugins. See client change logs and documentation.

### Breaking Changes
- Tunnel module has been changed. Read readme in the `src/modules/tunnel`


