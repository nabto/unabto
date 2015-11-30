/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "mcf52255.h"
#include <devices/coldfire/unabto_env_base_coldfire.h>
#include <devices/coldfire/iic.h>

#define NUMBER_OF_CHANNELS                                  2

void iic_initialize(uint8_t channel, uint8_t frequencyDivider)
{
    uint8_t dummy;
    
    MCF_I2C_I2FDR(channel) = MCF_I2C_I2FDR_IC(frequencyDivider);
    MCF_I2C_I2CR(channel) = MCF_I2C_I2CR_IEN;
    
    if (MCF_I2C_I2SR(channel) & MCF_I2C_I2SR_IBB){
        MCF_I2C_I2CR(channel) = 0; // clear control register
        MCF_I2C_I2CR(channel) = MCF_I2C_I2CR_IEN | MCF_I2C_I2CR_MSTA; // enable module, send a START condition
        dummy = MCF_I2C0_I2DR; // dummy read
        MCF_I2C_I2SR(channel) = 0; // clear status register
        MCF_I2C_I2CR(channel) = 0; // clear control register
        MCF_I2C_I2CR(channel) = MCF_I2C_I2CR_IEN; // enable the module again
    }
}

void iic_start(uint8_t channel)
{
    MCF_I2C_I2CR(channel) |= MCF_I2C_I2CR_MTX | MCF_I2C_I2CR_MSTA;
    
    while ((MCF_I2C_I2SR(channel) & MCF_I2C_I2SR_IBB) == false)
        ;
}

void iic_repeated_start(uint8_t channel)
{
    MCF_I2C_I2CR(channel) |= MCF_I2C_I2CR_RSTA;
    
    while ((MCF_I2C_I2SR(channel) & MCF_I2C_I2SR_IBB) == false)
        ;
}

void iic_stop(uint8_t channel)
{
    MCF_I2C_I2CR(channel) &= ~MCF_I2C_I2CR_MSTA;
    
    while ((MCF_I2C_I2SR(channel) & MCF_I2C_I2SR_IBB))
        ;
}

bool iic_write(uint8_t channel, void* data, uint16_t length)
{
    uint8_t* p = (uint8_t*)data;
    
    MCF_I2C_I2CR(channel) |= MCF_I2C_I2CR_MTX;
    
    while (length--){
        MCF_I2C_I2DR(channel) = *p++;
        
        while ((MCF_I2C_I2SR(channel) & MCF_I2C_I2SR_IIF) == false)
            ;
        
        MCF_I2C_I2SR(channel) &= ~MCF_I2C_I2SR_IIF;
        
        if (MCF_I2C_I2SR(channel) & MCF_I2C_I2SR_RXAK){
            return false;
        }
    }
    
    return true;
}

void iic_read(uint8_t channel, void* data, uint16_t length)
{
    uint8_t dummy;
    uint8_t* p = (uint8_t*)data;
    
    MCF_I2C_I2CR(channel) &= ~MCF_I2C_I2CR_MTX;
    
    if (length > 1){
        MCF_I2C_I2CR(channel) &= ~MCF_I2C_I2CR_TXAK;
    }else{
        MCF_I2C_I2CR(channel) |= MCF_I2C_I2CR_TXAK;
    }
    
    MCF_I2C_I2SR(channel) &= ~MCF_I2C_I2SR_IIF;
    
    dummy = MCF_I2C_I2DR(channel);
    
    while (length--){
        while ((MCF_I2C_I2SR(channel) & MCF_I2C_I2SR_IIF) == false)
            ;

        if (length == 1){
            MCF_I2C_I2CR(channel) |= MCF_I2C_I2CR_TXAK;
        }else if (length == 0){
            MCF_I2C_I2CR(channel) |= MCF_I2C_I2CR_MTX;
        }
        
        MCF_I2C_I2SR(channel) &= ~MCF_I2C_I2SR_IIF;
        
        *p++ = MCF_I2C_I2DR(channel);
    }
}
