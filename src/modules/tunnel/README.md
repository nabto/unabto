Tunnel module
=============

The tunnelling module is used such that the device can be used as a
transparent tcp tunnel together with the nabto client. See TEN030 for
more information.

The tunnel module has support for tcp,uart and echo tunneling. The
different tunnel modes can be enabled by setting cmake variables
before the tunnel library is added to a project.

Example cmake tunnel project where only a main method needs to be
added which initializes unabto and calls void
```tunnel_loop_select()``` or void ```tunnel_loop_epoll()```

```
cmake_minimum_required(VERSION 2.8)
project(tunnel C)
set(UNABTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(ENABLE_TUNNEL_TCP 1)
#set(ENABLE_TUNNEL_UART 1) 
#set(ENABLE_TUNNEL_ECHO 1)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/tunnel ${CMAKE_CURRENT_BINARY_DIR}/tunnel_library)
add_definitions(${unabto_definitions})
include_directories(${unabto_include_directories})
add_executable(tunnel ${unabto_src} your_custom_main.c)
target_link_libraries(tunnel unabto_tunnel_library)
```

Test Level
----------

This code is tested for each release.
