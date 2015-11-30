/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _NABTO_H_
#define _NABTO_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <unabto/unabto.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_app_adapter.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_logging.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_include_platform.h>
#include <unabto/unabto_common_main.h>
#include <unabto_version.h>
#include <device_drivers/w5100/w5100.h>
#include <platforms/arduino/spi.h>

class NabtoClass
{
 public:
    /**
     * @param mac, 6 bytes for the mac address
     * @param localIp, the local ip address
     * @param name, the namer of the device
     */
    void begin(uint8_t* mac, char* name);

    /**
     * @param v, char array to get nabto version
     */
    void version(char* v);

    /**
     * Nabto main tick
     */
    void tick();
 private:
    nabto_main_context nmc;
	uint32_t* ip;
};

/**
 * Create a default instantiation of nabto.
 */
extern NabtoClass Nabto;

#endif
