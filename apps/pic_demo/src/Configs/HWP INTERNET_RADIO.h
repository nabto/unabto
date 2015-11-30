/*********************************************************************
 *
 *	Hardware specific definitions for:
 *    - Internet Radio
 *    - PIC18F67J60
 *    - Internal 10BaseT Ethernet
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:    Compiler.h
 * Processor:       PIC18
 * Compiler:        Microchip C18 v3.36 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		09/16/2010	Regenerated for specific boards
 ********************************************************************/
#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include "Compiler.h"

// Define a macro describing this hardware set up (used in other files)
#define INTERNET_RADIO

// Set configuration fuses (but only in MainDemo.c where THIS_IS_STACK_APPLICATION is defined)
#if defined(THIS_IS_STACK_APPLICATION)
	#pragma config WDT=OFF, FOSC2=ON, FOSC=HSPLL, ETHLED=ON

	// Automatically set Extended Instruction Set fuse based on compiler setting
	#if defined(__EXTENDED18__)
		#pragma config XINST=ON
	#else
		#pragma config XINST=OFF
	#endif
#endif


// Clock frequency values
// These directly influence timed events using the Tick module.  They also are used for UART and SPI baud rate generation.
#define GetSystemClock()		(41666667ul)			// Hz
#define GetInstructionClock()	(GetSystemClock()/4)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Might need changing if using Doze modes.
#define GetPeripheralClock()	(GetSystemClock()/4)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Divisor may be different if using a PIC32 since it's configurable.


// Hardware I/O pin mappings

// LEDs
#define LED0_TRIS			(TRISCbits.TRISC2)	// Ref D3
#define LED0_IO				(LATCbits.LATC2)
#define LED1_TRIS			(TRISCbits.TRISC2)	// No LED1 on this board
#define LED1_IO				(LATCbits.LATC2)
#define LED2_TRIS			(TRISCbits.TRISC2)	// No LED2 on this board
#define LED2_IO				(LATCbits.LATC2)
#define LED3_TRIS			(TRISCbits.TRISC2)	// No LED3 on this board
#define LED3_IO				(LATCbits.LATC2)
#define LED4_TRIS			(TRISCbits.TRISC2)	// No LED4 on this board
#define LED4_IO				(LATCbits.LATC2)
#define LED5_TRIS			(TRISCbits.TRISC2)	// No LED5 on this board
#define LED5_IO				(LATCbits.LATC2)
#define LED6_TRIS			(TRISCbits.TRISC2)	// No LED6 on this board
#define LED6_IO				(LATCbits.LATC2)
#define LED7_TRIS			(TRISCbits.TRISC2)	// No LED7 on this board
#define LED7_IO				(LATCbits.LATC2)
#define LED_GET()			(LED0_IO)
#define LED_PUT(a)			(LED0_IO = (a))

// Momentary push buttons
#define BUTTON0_TRIS		(TRISBbits.TRISB5)
#define	BUTTON0_IO			(PORTBbits.RB5)
#define BUTTON1_TRIS		(TRISFbits.TRISF1)
#define	BUTTON1_IO			(PORTFbits.RF1)
#define BUTTON2_TRIS		(TRISBbits.TRISB4)
#define	BUTTON2_IO			(PORTBbits.RB4)
#define BUTTON3_TRIS		(TRISBbits.TRISB4)	// No BUTTON3 on this board
#define	BUTTON3_IO			(1)
/*
// Serial SRAM
#define SPIRAM_CS_TRIS		(TRISEbits.TRISE4)
#define SPIRAM_CS_IO		(LATEbits.LATE4)
#define SPIRAM_SCK_TRIS		(TRISCbits.TRISC3)
#define SPIRAM_SDI_TRIS		(TRISCbits.TRISC4)
#define SPIRAM_SDO_TRIS		(TRISCbits.TRISC5)
#define SPIRAM_SPI_IF		(PIR1bits.SSPIF)
#define SPIRAM_SSPBUF		(SSP1BUF)
#define SPIRAM_SPICON1		(SSP1CON1)
#define SPIRAM_SPICON1bits	(SSP1CON1bits)
#define SPIRAM_SPICON2		(SSP1CON2)
#define SPIRAM_SPISTAT		(SSP1STAT)
#define SPIRAM_SPISTATbits	(SSP1STATbits)
#define SPIRAM2_CS_TRIS		(TRISEbits.TRISE5)
#define SPIRAM2_CS_IO		(LATEbits.LATE5)
#define SPIRAM2_SCK_TRIS	(TRISCbits.TRISC3)
#define SPIRAM2_SDI_TRIS	(TRISCbits.TRISC4)
#define SPIRAM2_SDO_TRIS	(TRISCbits.TRISC5)
#define SPIRAM2_SPI_IF		(PIR1bits.SSPIF)
#define SPIRAM2_SSPBUF		(SSP1BUF)
#define SPIRAM2_SPICON1		(SSP1CON1)
#define SPIRAM2_SPICON1bits	(SSP1CON1bits)
#define SPIRAM2_SPICON2		(SSP1CON2)
#define SPIRAM2_SPISTAT		(SSP1STAT)
#define SPIRAM2_SPISTATbits	(SSP1STATbits)
*/
// VLSI VS1011/VS1053 audio encoder/decoder and DAC
#define MP3_DREQ_TRIS		(TRISBbits.TRISB0)	// Data Request
#define MP3_DREQ_IO 		(PORTBbits.RB0)
#define MP3_XRESET_TRIS		(TRISDbits.TRISD0)	// Reset, active low
#define MP3_XRESET_IO		(LATDbits.LATD0)
#define MP3_XDCS_TRIS		(TRISBbits.TRISB1)	// Data Chip Select
#define MP3_XDCS_IO			(LATBbits.LATB1)
#define MP3_XCS_TRIS		(TRISBbits.TRISB2)	// Control Chip Select
#define MP3_XCS_IO			(LATBbits.LATB2)
#define MP3_SCK_TRIS		(TRISCbits.TRISC3)
#define MP3_SDI_TRIS		(TRISCbits.TRISC4)
#define MP3_SDO_TRIS		(TRISCbits.TRISC5)
#define MP3_SPI_IF			(PIR1bits.SSP1IF)
#define MP3_SSPBUF			(SSP1BUF)
#define MP3_SPICON1			(SSP1CON1)
#define MP3_SPICON1bits		(SSP1CON1bits)
#define MP3_SPICON2			(SSP1CON2)
#define MP3_SPISTAT			(SSP1STAT)
#define MP3_SPISTATbits		(SSP1STATbits)

// OLED Display
#define oledWR				(LATAbits.LATA3)
#define oledWR_TRIS			(TRISAbits.TRISA3)
#define oledRD				(LATAbits.LATA4)
#define oledRD_TRIS			(TRISAbits.TRISA4)
#define oledCS				(LATAbits.LATA5)
#define oledCS_TRIS			(TRISAbits.TRISA5)
#define oledRESET			(LATDbits.LATD1)
#define oledRESET_TRIS		(TRISDbits.TRISD1)
#define oledD_C				(LATGbits.LATG4)
#define oledD_C_TRIS		(TRISGbits.TRISG4)

// UART mapping functions for consistent API names across 8-bit and 16 or 
// 32 bit compilers.  For simplicity, everything will use "UART" instead 
// of USART/EUSART/etc.
#define BusyUART()			BusyUSART()
#define CloseUART()			CloseUSART()
#define ConfigIntUART(a)	ConfigIntUSART(a)
#define DataRdyUART()		DataRdyUSART()
#define OpenUART(a,b,c)		OpenUSART(a,b,c)
#define ReadUART()			ReadUSART()
#define WriteUART(a)		WriteUSART(a)
#define getsUART(a,b,c)		getsUSART(b,a)
#define putsUART(a)			putsUSART(a)
#define getcUART()			ReadUSART()
#define putcUART(a)			WriteUSART(a)
#define putrsUART(a)		putrsUSART((far rom char*)a)

#endif // #ifndef HARDWARE_PROFILE_H
