/*********************************************************************
 *
 *  OLED display driver
 *  Module for Microchip TCP/IP Stack
 *   -Provides an API to access an OSD 2864ASWAG01 128x64 pixel OLED  
 *	  display, using the Sino Wealth SH1101A controller chip
 *	 -Reference: None
 *
 *********************************************************************
 * FileName:        OLED.h
 * Dependencies:    GenericTypeDefs.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F
 * Compiler:        Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
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
 * Author           	Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rodger Richey		03/10/07	Original
 ********************************************************************/
#ifndef OLED_H
#define OLED_H

#include "HardwareProfile.h"

#if defined(oledWR_TRIS)
	#include <delays.h>
	#include "GenericTypeDefs.h"
	
	
	#define SSON	1u
	#define	SSOFF	0u
	
	extern ROM BYTE g_pucFont[95][5];
	extern ROM BYTE Microchip[1024];
	
	void oledInitDisplay(void);
	void oledFillDisplay(unsigned char data);
	void oledFillLine(unsigned char data, unsigned char line);
	void oledWriteCommand(unsigned char cmd);
	void oledPutPixel(unsigned char x,unsigned char y,unsigned char color);
	void oledWriteChar1x(char letter, unsigned char page, unsigned char column);
	void oledWriteChar2x(char letter,unsigned char page, unsigned char column);
	unsigned char Convert1xto2x(unsigned char uplow, unsigned char byte);
	void oledPutImage(rom unsigned char *ptr, unsigned char sizex, unsigned char sizey, unsigned char startx, unsigned char starty);
	void oledScreenSaver(unsigned char enable);
	void oledPutString(unsigned char *ptr,unsigned char page, unsigned char col, unsigned char height);
	void oledPutROMString(rom unsigned char *ptr,unsigned char page, unsigned char col, unsigned char height);
#else
	// Bunch of empty preprocessor macros to make the function calls go away
	#define oledInitDisplay()
	#define oledFillDisplay(data)
	#define oledFillLine(data,line)
	#define oledWriteCommand(cmd)
	#define oledPutPixel(x,y,color)
	#define oledWriteChar1x(letter,page,column)
	#define oledWriteChar2x(letter,page,column)
	#define Convert1xto2x(uplow,byte)			(0u)
	#define oledPutImage(ptr,sizex,sizey,startx,starty)
	#define oledScreenSaver(enable)
	#define oledPutString(ptr,page,col,height)
	#define oledPutROMString(ptr,page,col,height)
#endif //#if defined(oled_WR)

#endif
