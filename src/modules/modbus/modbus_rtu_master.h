/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _MODBUS_RTU_MASTER_H_
#define _MODBUS_RTU_MASTER_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum size of a Modbus frame is Address (1) + Function code (1) + Data (0..256) + CRC (2).
#define MODBUS_MAXIMUM_DATA_SIZE                                    256
#define MODBUS_MAXIMUM_FRAME_SIZE                                   (1 + 1 + MODBUS_MAXIMUM_DATA_SIZE + 2)

typedef enum
{
    MODBUS_FUNCTION_READ_COILS = 1,
    MODBUS_FUNCTION_READ_HOLDING_REGISTER = 3
} modbus_function;

typedef enum
{
    MODBUS_MESSAGE_STATE_ALLOCATED,
    MODBUS_MESSAGE_STATE_QUEUED,
    MODBUS_MESSAGE_STATE_TRANSFERRING,
    MODBUS_MESSAGE_STATE_COMPLETED,
    MODBUS_MESSAGE_STATE_FAILED,
    MODBUS_MESSAGE_STATE_DISCARDED
} modbus_message_state;

typedef struct
{
    uint8_t bus;
    uint8_t address;
    uint8_t frame[MODBUS_MAXIMUM_FRAME_SIZE];
    uint16_t frameSize;
    uint8_t remainingTransmissionAttempts;
    uint32_t maximumResponsetime;
    bool deferredRetransmissions;
    modbus_message_state state;
} modbus_message;

void modbus_rtu_master_initialize(void);
void modbus_rtu_master_tick(void);
modbus_message* modbus_rtu_master_allocate_message(void);
void modbus_rtu_master_release_message(modbus_message* message);
bool modbus_rtu_master_transfer_message(modbus_message* message);

void modbus_function_read_holding_register(modbus_message* message, uint8_t address, uint16_t startingAddress, uint16_t quantityOfRegisters);

extern const char* modbusMessageStateNames[];


#ifdef __cplusplus
} //extern "C"
#endif

#endif
