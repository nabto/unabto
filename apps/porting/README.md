= Porting uNabto - creating  a new uNabto platform adapter =

In this section a new uNabto platform adapter will be created for the
linux platform. In each step we will describe what is neccessary.

A uNabto platform adapter is the code which the uNabto Core uses for
network communication, time handling, dns resolving, logging,
randomness etc. We already provide modules for many platforms for
network, dns, timing, encryption, random etc. they are located in the
modules folder in the source tree. All the functionality in this
description is also available as uNabto modules and the implementation
using modules can be found under the official unix platform in the
platforms/unix folder in the uNabto source.

We will describe all the neccessary steps to create a new platform
here. First we will describe the overall structure of a uNabto
platform adapter and secondly we will describe de individual module
implementations which is neccessary for a complete uNabto platform
adapter.

== Overall structure. ==

The overall structure of a uNabto platform adapter consists of 2
header files and several source files. The two header files are
unabto_platform_types.h and unabto_platform.h. The first header file
describes all the types neccessary for uNabto to run while the latter
describes all the platform dependencies which isn't types this is for
example macro definitions. A uNabto platform adapter is added to a
uNabto project by adding the uNabto platform adapter to the compilers
include path, then uNabto will include the respective
unabto_platform.h and unabto_platform_types.h

== Basic code ==

First we create a main program which can run uNabto with the platform
adapter we are developing. And secondly a uNabto configuration file
which sets the outline for the implementation.

The main file below is just a simple runner which setup uNabto and
calls the tick function.

[source,c]
----
include::src/main.c[]
----

uNabto configuration file.

[source,c]
----
include::src/unabto_config.h[]
----

== Implementing unabto_platform_types.h ==

Next we create the unabto_platform_types.h file it should define all
neccessary types for unabto. Thats at least timestamps, int*_t
uint*_t, booleans and sockets.

[source,c]
----
include::src/unabto_platform_types.h[]
----

== Implementing unabto_platform.h ==

When we have all the basic types we need to make a unabto_platform.h
implementation. This can be used to implement all the adhoc functions
which isn't neccessarily specified as a link time dependency in
unabto_external_environment.h

[source,c]
----
include::src/unabto_platform.h[]
----

== Implementing a network adapter ==

The network adapter should implement the functions which is neccessary
for creating communication sockets. This is init, close, read and
write functionality for network data. A simple unix implementation is
like

[source,c]
----
include::src/network_adapter.c[]
----

== Implementing a time adapter ==

The uNabto core has several timers which handle timed events, like
reconnecting or retransmitting a packet. In order to facilitate this
some time functions must be available. The timing needed does not need
to be absolute, it should just be monotonically increasing in some
arbitrary time period.

[source,c]
----
include::src/time_adapter.c[]
----

== Implementing a dns adapter ==

The dns adapter should be able to do an asynchronous dns resolving.

[source,c]
----
include::src/dns_adapter.c[]
----

== Implementing a random adapter ==

[source,c]
----
include::src/random_adapter.c[]
----