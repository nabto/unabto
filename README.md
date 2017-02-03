# uNabto SDK [![Build Status](https://travis-ci.org/nabto/unabto.svg?branch=master)](https://travis-ci.org/nabto/unabto)

Nabto provides a full communication infrastructure to allow direct, encrypted communication between clients and IoT devices - the Nabto  communication platform. The platform supports direct peer-to-peer connectivity through NAT traversal.

uNabto (pronounced 'micro nabto') is an open source C framework supplied by Nabto, which can be integrated with your existing device application.

### Documentation

A general introduction to the Nabto platform can be found on [www.nabto.com](https://www.nabto.com). More detailed documentation and tutorials are available on [developer.nabto.com](https://developer.nabto.com). Also see our blog [blog.nabto.com](https://blog.nabto.com) with examples and projects.

The document [TEN036 "Security in Nabto Solutions"](https://www.nabto.com/downloads/docs/TEN036%20Security%20in%20Nabto%20Solutions.pdf) is mandatory read when considering Nabto for use in production solutions.

The TESTING.md document in this directory describes how integrators may test their uNabto SDK based applications and what their responsibilities are.

### Source code

The `src` directory contains the uNabto framework source code and device specific config files.

`build/cmake/unabto_files.cmake` sets the build environment used by some of the demos.

`external` is where you put all uNabto externals.

### Example applications and demos

The `apps` directory contains uNabto examples for different platforms (e.g. Windows, Unix, Microchip, Arduino, etc.).

## Building Linux, Windows And MacOS

### General build instructions

```
clone repository
mkdir builddir
cd builddir
cmake ..
make
```

### Windows

Prerequisities:
  * CMake,
  * Visual studio E.g. 2015

```
clone repository
mkdir builddir
cd builddir
cmake -DCMAKE_GENERATOR_TOOLSET=v140_xp ..
cmake --build .
```

## License

Copyright (C) 2008-2016 Nabto - All Rights Reserved.

This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"

FreeRTOS+UDP uses a dual license model that allows the software to be used under a standard GPL open source license, or a commercial license. The standard GPL license (unlike the modified GPL license under which FreeRTOS itself is distributed) requires that all software statically linked with FreeRTOS+UDP is also distributed under the same GPL V2 license terms.  
