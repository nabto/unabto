/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "mcf52255.h"
#include <devices/coldfire/unabto_env_base_coldfire.h>
#include <devices/coldfire/iic.h>
#include <devices/coldfire/eeprom_24xx1025.h>
#include <application.h>

#define PAGE_SIZE                                                                       128
#define WRITE_TIMEOUT                                                                   1000 

static bool eeprom_page_write(uint32_t address, void* data, uint8_t length);

bool eeprom_read(uint32_t address, void* data, uint16_t length)
{
    uint8_t control[3];
    control[0] = (uint8_t)(0xa0 | 0 | 0); // clear read/write bit
    control[1] = (uint8_t)(address >> 8);
    control[2] = (uint8_t)address;
    
    if (address > 0xffff){
        control[0] |= 0x08;
    }

    iic_start(EEPROM_IIC_CHANNEL);
    
    if (iic_write(EEPROM_IIC_CHANNEL, control, 3) == false){
        iic_stop(EEPROM_IIC_CHANNEL);
        return false;
    }

    iic_repeated_start(EEPROM_IIC_CHANNEL);
    
    control[0] |= 1; // set read/write bit
    
    if (iic_write(EEPROM_IIC_CHANNEL, control, 1) == false){
        iic_stop(EEPROM_IIC_CHANNEL);
        return false;
    }

    iic_read(EEPROM_IIC_CHANNEL, data, length);
    
    iic_stop(EEPROM_IIC_CHANNEL);
    
    return true;
}

bool eeprom_write(uint32_t address, void* data, uint16_t length)
{
    uint8_t* p = (uint8_t*)data;
    uint8_t currentLength;
    uint8_t currentPageAddress;
    
    while (length > 0){
        currentPageAddress = (uint8_t)(address & (PAGE_SIZE - 1));
        currentLength = (uint8_t)(PAGE_SIZE - currentPageAddress); // calculate maximum byte that can be written to this page from the current offset
        if (length < currentLength){
            currentLength = (uint8_t)length;
        }

        if (eeprom_page_write(address, p, currentLength) == false){
            return false;
        }
        address += currentLength;
        p += currentLength;
        length -= currentLength;
    }
    
    return true;
}

static bool eeprom_page_write(uint32_t address, void* data, uint8_t length)
{
    uint8_t control[3];
    uint16_t timeout = WRITE_TIMEOUT;
    control[0] = (uint8_t)(0xa0 | 0 | 0);
    control[1] = (uint8_t)(address >> 8);
    control[2] = (uint8_t)address;
    
    if (address > 0xffff){
        control[0] |= 0x08;
    }

    iic_start(EEPROM_IIC_CHANNEL);
    
    if (iic_write(EEPROM_IIC_CHANNEL, control, 3) == false){
        iic_stop(EEPROM_IIC_CHANNEL);
        return false;
    }

    if (iic_write(EEPROM_IIC_CHANNEL, data, length) == false){
        iic_stop(EEPROM_IIC_CHANNEL);
        return false;
    }

    iic_stop(EEPROM_IIC_CHANNEL);
    
    while (timeout--){
        iic_start(EEPROM_IIC_CHANNEL);
        if (iic_write(EEPROM_IIC_CHANNEL, control, 1)){
            iic_stop(EEPROM_IIC_CHANNEL);
            return true;
        }
        iic_stop(EEPROM_IIC_CHANNEL);
    }
    
    return false;
}
