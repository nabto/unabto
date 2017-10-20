# uNabto SDK 
[![Build Status] [1]][2]
[![Build Status: Windows] [3]][4]
[1]: https://travis-ci.org/nabto/unabto.svg?branch=master
[2]: https://travis-ci.org/nabto/unabto
[3]: https://ci.appveyor.com/api/projects/status/github/nabto/unabto?svg=true&branch=master
[4]: https://ci.appveyor.com/project/nabto/unabto

Nabto provides a full communication infrastructure to allow direct, encrypted communication between clients and IoT devices - the Nabto  communication platform. The platform supports direct peer-to-peer connectivity through NAT traversal.

uNabto (pronounced 'micro nabto') is an open source C framework supplied by Nabto, which can be integrated with your existing device application.

### Documentation

A general introduction to the Nabto platform can be found on [www.nabto.com](https://www.nabto.com). More detailed documentation and tutorials are available on [developer.nabto.com](https://developer.nabto.com). Also see our blog [blog.nabto.com](https://blog.nabto.com) with examples and projects.

The document [TEN036 "Security in Nabto Solutions"](https://www.nabto.com/downloads/docs/TEN036%20Security%20in%20Nabto%20Solutions.pdf) is mandatory read when considering Nabto for use in production solutions.

The TESTING.md document in this directory describes how integrators may test their uNabto SDK based applications and what their responsibilities are.

### Example applications and demos

This repository contains example applications for the three major OS's
Windows, Linux and Mac. Demos for these platforms is located in the
`apps` folder.

We have repositories for other embedded platforms as well.

  * [PIC32 Ethernet starterkit II (Ethernet)](https://github.com/nabto/unabto-pic32-sdk)
  * [STM32 Discovery (Ethernet)](https://github.com/nabto/unabto-stm32-sdk)
  * [CC3200 (WiFi)](https://github.com/nabto/unabto-cc3200)
  * [ESP8266 (WiFi)](https://github.com/nabto/unabto-esp8266-sdk)
  * [Arduino (Ethernet)](https://github.com/nabto/unabto-arduino-sdk)



## Source code

### Core 

The unabto core is located in `src/unabto/` this source is needed by
all unabto instances.

### Modules

In the folder `src/modules` there are modules
for encryption, random, timers, utilities, etc. These modules makes it
easier to implement unabto on a platform.

### Platforms

The `src/platforms` folder includes various modules and implementation
which together forms as a platform adapter for some specific
platforms.

### build/cmake/unabto_files.cmake

The build/cmake/unabto_files.cmake is a file which lists all the source code in the repository which is used together with cmake projects. The file does not have any logic included it is only a list of variables assigned to specific files or definitions.

### build/cmake/unabto_project.cmake

The build/cmake/unabto_project.cmake is a cmake project definition file which is used to simply create unabto projects for Windows, Linux and Mac.

A simple unabto static library project based on unabto_project.cmake
is created as follows:

```
cmake_minimum_required(VERSION 2.8)
project(example)
include(${CMAKE_CURRENT_SOURCE_DIR}/ ... /build/cmake/unabto_project.cmake)

add_definitions(${unabto_definitions})
add_library(example ${unabto_src})
target_link_libraries(example ${unabto_link_libraries})
```

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
