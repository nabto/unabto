/**
 *   This project demonstrates the uNabto framework running the
 *   uNabto SDK demo on Raspberry Pi (Linux).
 *   There are to implementation:
 *
 *   Remote Light Switch Demo:
 *   1) Install CMake (see www.cmake.org).
 *   2) Run "cmake ." in the "raspberry_pi" directory.
 *   3) Run "make" to make the executable.
 *   4) And run the newly made unabto_raspberrypi by e.g. "./unabto_raspberrypi -d [ yourName ].sdk.u.nabto.net".
 *
 *   Remote the Pi-Face (see www.pi.cs.man.ac.uk/interface.htm):
 *   1) Install CMake (see www.cmake.org).
 *   2) Install PiFace library (see www.github.com/thomasmacpherson/piface).
 *   3) Set USING_PIFACE to '1' in the unabto_config.h.
 *   4) Run "cmake . -Dusing_piface=ON" in the "raspberry_pi" directory 
 *   5) Run "make" to make the executable.
 *   6) And run the newly made unabto_raspberrypi by e.g. "./unabto_raspberrypi -d [ yourName ].piface.sdk.u.nabto.net".
 *
 *
 */