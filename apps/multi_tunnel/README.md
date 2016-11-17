Multi Tunnel Application
========================

This application is sets up a tunnel able to tunnel traffic from a UART device or a TCP socket into a Nabto stream. Additionally it has echo functionallity to tunnel Nabto stream inputs back to the Nabto stream output.

Located in:
	unabto/apps/multi_tunnel

This application uses CMake and make to build to most unix platforms. The tunnel simply runs the tunnel and waits for incoming streams.

Requirements:
	- PC or Device running a unix distribution.
	- CMake, make and gcc/clang.

How to run:
	- make a build directory: mkdir build
	- run CMake from the build directory: cmake PATH_TO_NABTO/unabto/
	- run make from the build directory:  make -j
	- go to directory: cd unabto/apps/multi_tunnel
	- run the tunnel:
	  - Only using the TCP and Echo functionalities: ./unabto_multi_tunnel -d my_device.nabto.net -s
	  - Using the UART part requires it to know the device name: ./unabto_multi_tunnel --uart_device /dev/MY_TTY_1 -d my_device.nabto.net -s

The multi tunnel is now running, and can be used. The Nabto uart_tester_app and simple_client_app can be used to verify.
	- set up a pseudo terminal in linux: socat -d -d pty,raw,echo=0 pty,raw,echo=0
	- note the two pseudo terminals socat outputs, and use those as UART devices.
	- To verify bi-directional communication using UART run: ./nabto/linux64/bin/uart_tester_app -q my_device.nabto.net -t rwData --terminal MY_TTY_2 --datasize 10
	- To show Echo functionality:
	  	 - run: ./nabto/linux64/bin/uart_tester_app -q my_device.nabto.net -t raw --terminal MY_TTY_2
		 - To start Echo mode type: echo
		 - Anything that is typed should now echo back to the shell.
	- To show TCP tunnel functionallity:
	  	 - make sure openssh-server is installed and running
	  	 - run: ./nabto/linux64/bin/simpleclient_app/simpleclient_app -q my_device.nabto.net --tunnel 4243:127.0.0.1:22
		 - ssh access to the localhost should now be available on port 4243, run: ssh localhost -p 4243
		 
