# uNabto Application Testing by the Integrator

The uNabto SDK can be used in a vast variety of combinations. Nabto
thoroughly tests the general capabilities and functionality of the
platform-independent, core platform.

In the specific, individual combination of hardware, operating system,
IP stack, toolchains, Nabto configuration and other applications on
the system at the specific integrator (uNabto SDK user), the
integrator must perform tests of the specific resulting application in
the target environment. This can be done in many ways, this document
describes a typical approach often used by developers and integrators.

The various examples of integration to specific platforms as found in
the uNabto SDK are to be considered a starting point for the vendor's
integration, not part of the core Nabto platform - meaning that the
vendor must follow the recommendations for testing as described in
this document, even if using software directly from the SDK.

Each module contains a README.md file describing the quality assurance
level provided by Nabto and can hence be used as a guide towards the
required testing effort need by the integrator of the target solution.

Various models also exist for assistance with integration and testing
\- Nabto can take on the full responsibility as systems integrator or
assist an integrator on a consultancy basis. Under all circumstances,
feel free to write to support@nabto.com for assistance, we will always
help on a best effort basis - and with a support agreement, also to an
agreed SLA.


# Integration test objectives

The most important Nabto-related aspects to test for the integrator
are the platform specific parts of the system. This basically means
network communication and timing related functionality - the
functionality outlined in section 12 of TEN023 "Writing a uNabto
Device Application".

The simplest way to exercise this functionality is through a long
running test where the client continuously connects and invokes an RPC
function on the device. This is outlined in section "Basic network and
timing tests" below. 

A suite of unit tests that exercises the most important core
functionality is provided, it is recommended to run on the
target platform built with the same toolchain as the production binary.

Ideally, different network configurations are also tested to exercise
the different connection types (p2p, relay, local), the network
quality can also be varied, e.g. through simulators and the different
communication patterns (RPC, Streaming, Push) are exercised. This
platform independent functionality is exercised thoroughly by Nabto -
but for completeness on the target platform, it may still be
performed.


## Basic network and timing tests

To invoke the Nabto device periodically, either a simple commandline
tool from Nabto can be used. Or for greater flexibility, the Nabto
Client SDK can be used to build a custom test application, e.g. for closer
integration with existing tools.

The tests basically perform these operations from the Nabto Client SDK to invoke the device:

```bash
   for test_count do:
      session = nabtoOpenSession()
      for rep_count do:
         nabtoFetchUrl(session, url)
         sleep(sleep_time)
      done
      nabtoCloseSession(session)
   done
```

### Prepare Nabto device for test

The target Nabto device in question must be started and expose at least one
RPC function for remote invocation, ie. with a request defined in
unabto_queries.xml used by the client and a corresponding
`application_event()` handler in the device.

It does not matter which function this is. In the below example, the
weather station demo app is used for testing. Adjust the client
request URLs accordingly in the actual scenario at hand.


### Prepare Nabto command line tool for test

The simpleclient_app command allows repeated execution of a request
towards a device with a delay between each execution. The same Nabto
session is used (see TEN025 "Writing a Nabto Client API Application"
for a description of Nabto sessions), meaning that connections are
reused. So the test tool must be restarted to exercise opening
and closing connections - this is shown below.

1. From https://developer.nabto.com, download the "Simple Client App"
commandline utility. Additionally, download the Nabto Client SDK
Resource bundle.

2. Unzip both and move content to a common root directory with the structure seen below.

3. On non-development Windows workstations (i.e., without Visual Studio installed), install the MSVC 2013 Runtime Redistributable (https://www.microsoft.com/en-GB/download/details.aspx?id=40784)

Dir structure:

```
<root>/share
<root>/share/nabto
<root>/share/nabto/schemas
<root>/share/nabto/schemas/query_model.xsd
<root>/share/nabto/users
<root>/share/nabto/users/guest.key
<root>/share/nabto/users/guest.crt
<root>/share/nabto/roots
<root>/share/nabto/roots/ca.crt
<root>/share/nabto/roots/cacert.pem
<root>/share/nabto/configuration
<root>/share/nabto/configuration/mimetypes.ini
<root>/bin/simpleclient_app
```


### Long running test with continuous requests and connects

The tests below should ideally run for at least as long time it takes for the tick counter to overflow in tick timer based configurations on the specific platform. Or 24 hours, whichever is larger.

After the time has passed (measured manually) and the test is interrupted, the file out.txt contains the output from the invocations. Inspect it and the device console output (if available) for any anomalies. That is, device invocations that fail for non-obvious reasons.

#### Executing tests on Windows

In a command prompt enter the following (or invoke through a `.cmd` script):


```cmd
@echo off
for /l %%x in (1, 1, 999999) do (
  echo "%date% %time%" >> out.txt
  .\bin\simpleclient_app.exe -q nabto://demo.nabto.net/wind_speed.json? -t 3 -S 2000 >> out.txt 2>&1 | type out.txt
  timeout 1
)
```

#### Executing tests on Linux / Mac OS X

In a command prompt enter the following (or invoke through a bash script):

```bash
while true;
  do date | tee -a out.txt 
  simpleclient_app -q nabto://demo.nabto.net/wind_speed.json? -S 2000 -t 3 | tee -a out.txt 2>&1
  sleep 1
done
```

### Long running test with passive device

It should be tested that it is possible to invoke the device after it
has been sitting passively (with no connect requests) for the same
time as in the above long running request and connect test. After
waiting this period, invoke the device using any already used means of
invoking a remote function on it - or see the above examples of using
simpleclient_app.

### Connection ressource exhaustion test

The purpose of this test is to verify that connection ressources are
correctly released on the device after a while if the client uncleanly
closes the connection. This test is best run with the standalone Nabto
test client mentioned above as an unclean exit is known to work as intended.

The idea is to start a client that will make repeated requests for a
while, meaning the client will be keep running with an open connection
with plenty of time to uncleanly shutting it down. After a few such
repetitions, the client will see an error message. After waiting a minute, it should then
be possible again.

As the purpose of the test is to exhaust connection ressources on the
device and see they are reclaimed after a garbage collection period,
the Nabto configuration parameter `NABTO_CONNECTIONS_SIZE` should not
be set too high. That is, if you want to test the exact production
system and it is not feasible to change this parameter, it is still
possible to verify this, albeit a bit less explicit, by watching the
console output on the device (if available - otherwise lowering the
number of connection slots or executing many test clients in parallel
are the only options).

#### Steps to execute

In a command prompt, perform the following:

1. `./bin/simpleclient_app -q nabto://demo.nabto.net/wind_speed.json? -S 5000 -t 3`

2. observe a succesful invocation of the device in the client console

3. within 15 seconds (5000 ms x 3), press Ctrl-C to stop the client

4. repeat from step 1 until the error "1000021 - too many connections to micro server" is seen in the client console.

5. wait a minute

6. repeat step 1 again and observe the device is now invoked successfully again

If not feasible to repeat the test until complete ressource exhaustion, an alternative is to observe the device console instead, if possible. Each invocation in step 1 should after a minute be followed by a line like the following in the console:

```
17:31:46:359 unabto_connection.c(155) (0.3948615778.0) Release connection (record 0)
17:31:48:026 unabto_connection.c(155) (0.3948615779.0) Release connection (record 1)
...
```

## Unit tests on the target platform

A suite of unit tests are supplied to verify that basic operations such as cryptographic calculations and buffer manipulation are performed as expected with the vendor's chosen toolchain on the target architecture.

To execute the unit test suite, include the unit test source files as listed in the `unabto_server_common_src_test` definition in `./build/cmake/unabto_files.cmake` in your project or a dedicated test application. In the latter case, it is of utmost importance that the separate application is compiled with exactly the same tools, options and `unabto_config.h` as the target application. On high level platforms with cmake support, an example of the latter standalone test runner is provided in in `unabto/test/runners`. 


## Advanced connection type tests

Three different types of connections can be established to the
device. From the client side, four different connection types are
possible (two different types of relay through the basestation). To
verify just the device functionality, using any of the two relay
connection types from the client is sufficient. To verify full system
end-to-end functionality all four types should be tested.

Most of the functionality involved in establishing the different types
of connections resides in the core uNabto SDK and is exercised
thoroughly by Nabto. Still, the overall system including the vendor
specific platform integration should be verified to work with each
connection type.

The following sections suggest in detail how to setup configurations
to trigger the specific connection types.

### Testing local connections

The caveat in this test is that the client must have communicated with the device prior to the isolated test to have a cached interface definition available (or have it injected directly, but for simplicity it suffices to use the caching approach).

1. in a normal connected network setting, perform a client test, e.g. using simpleclient_app as suggested above

2. put client and device on the same local network

3. disconnect the local network from the Internet

4. perform a client invocation and verify the device can be invoked


### Testing P2P connections

This test requires control of the firewall on both the device and
client networks. The firewall must not block UDP traffic and must not
be of the symmetric NAT type (unless extendend rendezvous is enabled -
test of this is currently not described).

1. put client and device on two different networks

2. make sure nothing blocks the client and device's outbound UDP network communcation towards ports 5566 and 5562

3. perform a client invocation and verify the device can be invoked

4. observe the device's console log (or otherwise observe the peer type in `unabto_connection.c:nabto_connection_end_connecting()`):

```17:06:23:875 unabto_connection.c(656) (0.3948617925.0) Direct UDP connection```


### Testing relay connections (client HTTP relay)

This test requires control of the firewall on both the device and
client networks. The client firewall must block all UDP traffic above
port 5000 and most TCP traffic, but allow outbound HTTP traffic. The
device's firewall must allow outbound UDP traffic.

1. put client and device on two different networks

2. block client firewall's outbound UDP traffic for ports 5000 and block TCP ports except port 80

3. allow device firewall's outbound UDP traffic towards ports 5566 and 5562

3. perform a client invocation and verify the device can be invoked

4. observe the device's console log (or otherwise observe the peer type in `unabto_connection.c:nabto_connection_end_connecting()`):

```17:05:13:198 unabto_connection.c(654) (0.3948617921.0) UDP Fallback through the GSP```

The test can be simplified by forcing relay connections instead of
manipulating firewalls by setting `disableDirectConnection=1` in the
`nabto_config.ini` file. This is somewhat artificial as the handshake
is simplified, but exercises most of the code as above without the
likely hassle of configuring the firewall.

### Testing relay connections (client non-HTTP relay)

This is only relevant if planning on having clients deployed in
mainland China using the global Nabto Cloud platform where websockets
are not supported. Or if planning on using legacy Nabto Basestations
with only non-websocket relay support.

The test is exactly as the client HTTP relay test above, except for a
different port open in the client: Instead of allowing client outbound
HTTP traffic, allow only TCP traffic towards port 5568. All other
steps identical as the other testcase.


## Security test objectives

Overall security aspects (framework level authentication,
confidentiality, integrity) are tested thoroughly by Nabto as part of
core QA and need not much attention by the integrator: Through code
reviews and inspection the vendor must verify that handling of
cryptographic keys is sound. And similarly verify that a
cryptographically secure random function is supplied to the uNabto SDK
(as described in section 12 of TEN023).

The vendor must thoroughly test the application specific authorization /
access control logic as outlined in section 10.3 of TEN036 "Security
in Nabto Solutions", available from www.nabto.com.

If the vendor supplies self-implemented application level
authentication of clients as outlined in section 10.3 of TEN036, the
integrator must additionally thoroughly test and verify this crucial
application specific functionality. For instance, this means verifying
the vendor specific username/password database is implemented
correctly.
