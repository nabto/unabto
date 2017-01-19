Make directory junctions to local copies of unabto externals here.

Links you might need to make:
    Microchip TCP/IP stack
    LWIP
    FreeRTOS

On Windows:
    mklink /J Microchip "C:\Microchip Solutions v2012-10-15\Microchip"
    mklink /J lwip "c:\lwip-1.4.0"
    mklink /J FreeRTOS "c:\FreeRTOSV7.2.0\FreeRTOS"

On Unix:
    ln -s "~/Documents/Microchip Solutions v2012-10-15/Microchip" Microchip
    ln -s "~/Documents/lwip-1.4.0" lwip
    ln -s "~/Documents/FreeRTOSV7.2.0/FreeRTOS" FreeRTOS