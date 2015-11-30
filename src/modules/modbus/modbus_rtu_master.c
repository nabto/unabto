/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_MODBUS

#include "modbus_rtu_master.h"
#include "modbus_rtu_crc.h"
#include <modules/util/list_malloc.h>
#include <modules/util/memory_allocation.h>
#include <uart.h>
#include <application.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_util.h>

#define DEFAULT_RESPONSE_TIMEOUT                                    500 // the maximum time allowed to pass before the response must be initiated (time from the last byte sent from the master to the first byte received at the master)
#define DEFAULT_MAXIMUM_TRANSMISSION_ATTEMPTS                       3

#define BITS_PER_CHARACTER          (1 + 8 + 1 + 1)
#define FRAME_SILENT_DURATION       MAX(1, (((BITS_PER_CHARACTER * 3500000) / baudrate) / 1000))

typedef enum
{
    BUS_STATE_IDLE,
    BUS_STATE_BUSY
} modbus_state;

typedef struct
{
    int identifier;
    list messageQueue;
    uint8_t uartChannel;
    modbus_state state;
    uint8_t response[MODBUS_MAXIMUM_FRAME_SIZE];
    uint16_t responseSize;
    bool responseOverrun;
    nabto_stamp_t frameCommitTimeout; // the timestamp where the frame is considered completed
    nabto_stamp_t responseTimeout; // a response should be received within this limit
} modbus;

static void retransmit_current_message(modbus* bus);
static uint32_t calculate_transmission_time(uint32_t numberOfBytes);

const char* modbusMessageStateNames[] = 
{
    "Allocated",
    "Queued",
    "Transferring",
    "Completed",
    "Failed",
    "Discarded"
};

static const char* modbusStateNames[] = 
{
    "Idle",
    "Busy"
};

static modbus busses[MODBUS_NUMBER_OF_BUSSES];
static list completedList;

void modbus_rtu_master_initialize(void)
{
    uint8_t i;
    modbus* bus;
    
    // reset all busses
    for(i = 0; i < MODBUS_NUMBER_OF_BUSSES; i++)
    {
        bus = &busses[i];
        bus->identifier = i;
        bus->uartChannel = i;
        list_initialize(&bus->messageQueue);
        bus->state = BUS_STATE_IDLE;
    }

    list_initialize(&completedList);
    
    NABTO_LOG_INFO(("Initialized."));
}

void modbus_rtu_master_tick(void)
{
    modbus* bus;

    // tick all busses
    for(bus = busses; bus < (busses + MODBUS_NUMBER_OF_BUSSES); bus++)
    {
        modbus_state oldState;

        oldState = bus->state;

        switch(bus->state)
        {
        case BUS_STATE_IDLE:
            {
                modbus_message* message;
                if(list_peek_first(&bus->messageQueue, (void**)&message)) // is there a message waiting to be sent
                {
                    if(message->state == MODBUS_MESSAGE_STATE_DISCARDED) // has message been discared by the application while waiting in the queue?
                    {
                        list_remove(&bus->messageQueue, message);

                        modbus_rtu_master_release_message(message);

                        NABTO_LOG_TRACE(("Completing discard request (bus=%u, query=%u).", bus->identifier, (int)message));
                    }
                    else // start transferring the message
                    {
                        uint32_t compensatedReponseTime = message->maximumResponsetime + calculate_transmission_time(message->frameSize);

                        NABTO_LOG_TRACE(("Sending query (bus=%u, timeout=%u, compensated timeout=%u).", bus->identifier, (int)message->maximumResponsetime, (int)compensatedReponseTime));

                        uart_flush_receiver(bus->uartChannel);
                        uart_write_buffer(bus->uartChannel, message->frame, message->frameSize);

                        message->state = MODBUS_MESSAGE_STATE_TRANSFERRING;

                        nabtoSetFutureStamp(&bus->responseTimeout, compensatedReponseTime); // time to wait for start of response: transmission time of outgoing frame + maximum response time
                        bus->responseSize = 0;
                        bus->responseOverrun = false;

                        bus->state = BUS_STATE_BUSY;
                        
                        NABTO_LOG_TRACE(("Query sent (bus=%u, timeout=%u, compensated timeout=%u).", bus->identifier, (int)message->maximumResponsetime, (int)compensatedReponseTime));
                        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_LEVEL_TRACE, ("Query (length=%u): (shown with CRC)", (int)message->frameSize), message->frame, message->frameSize);
                    }
                }
            }
            break;

        case BUS_STATE_BUSY:
            {
                uint16_t length;
                
                length = uart_can_read(bus->uartChannel);

                if(length > 0) // has data been received?
                {
                    if(bus->responseOverrun == false) // ignore all data after an overrun has occurred
                    {
                        if((bus->responseSize + length) <= MODBUS_MAXIMUM_FRAME_SIZE) // is there room for the data?
                        {
                            // add data to response buffer
                            uart_read_buffer(bus->uartChannel, bus->response + bus->responseSize, length);
                            bus->responseSize += length;
                        }
                        else // response buffer overrun
                        {
                            uart_flush_receiver(bus->uartChannel); // discard data
                            bus->responseOverrun = true; // mark response as over size
                            NABTO_LOG_TRACE(("Receiving oversize response (bus=%u, oversize length=%u)!", bus->identifier, (int)((bus->responseSize + length) - MODBUS_MAXIMUM_FRAME_SIZE)));
                        }
                    }
                    else
                    {
                        uart_flush_receiver(bus->uartChannel); // discard data
                        NABTO_LOG_TRACE(("Dumping overrun data (bus=%u, length=%u)!", bus->identifier, (int)length));
                    }

                    nabtoSetFutureStamp(&bus->frameCommitTimeout, FRAME_SILENT_DURATION); // reset packet commit timer
                }
                else if(bus->responseSize > 0) // has the response begun
                {
                    if(nabtoIsStampPassed(&bus->frameCommitTimeout)) // has the frame ended
                    {
                        modbus_message* message;
                        list_peek_first(&bus->messageQueue, (void**)&message);

                        if(message->state == MODBUS_MESSAGE_STATE_DISCARDED) // has the message been discarded
                        {
                            list_remove(&bus->messageQueue, message);

                            modbus_rtu_master_release_message(message); // perform actual release of discarded message

                            bus->state = BUS_STATE_IDLE;

                            NABTO_LOG_TRACE(("Received response for dumped query (bus=%u, query=%u, response length=%u).", bus->identifier, (int)message, (int)bus->responseSize));
                        }
                        else
                        {
                            if(modbus_rtu_crc_verify_crc_field(bus->response, bus->responseSize)) // does reponse pass CRC check?
                            {
                                NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_LEVEL_TRACE, ("Received response (bus=%u, length=%u): (shown with CRC)", bus->identifier, (int)bus->responseSize), bus->response, bus->responseSize);

                                // replace the query in the message with the response (removing CRC)
                                memcpy(message->frame, bus->response, bus->responseSize - 2);
                                message->frameSize = bus->responseSize - 2;

                                message->state = MODBUS_MESSAGE_STATE_COMPLETED; // mark message as completed

                                // move message from transfer queue to completed list
                                list_remove(&bus->messageQueue, message);
                                list_append(&completedList, message);

                                bus->state = BUS_STATE_IDLE;

                                NABTO_LOG_TRACE(("Received response for query (bus=%u, query=%u).", bus->identifier, (int)message));
                            }
                            else
                            {
                                NABTO_LOG_TRACE(("Received bad response for query (bus=%u, query=%u)!", bus->identifier, (int)message));
                                retransmit_current_message(bus);
                            }
                        }
                    }
                }
                else if(nabtoIsStampPassed(&bus->responseTimeout)) // the response has not begun within the time limit
                {
                    modbus_message* message;
                    list_peek_first(&bus->messageQueue, (void**)&message);

                    NABTO_LOG_TRACE(("No response received (bus=%u, message=%u)!", bus->identifier, (int)message));

                    if(message->state == MODBUS_MESSAGE_STATE_DISCARDED) // has the application discarded the message?
                    {
                        list_remove(&bus->messageQueue, message);

                        modbus_rtu_master_release_message(message); // perform actual release of discarded message

                        bus->state = BUS_STATE_IDLE;
                        
                        NABTO_LOG_TRACE(("Completing discard request (bus=%u, query=%u).", bus->identifier, (int)message));
                    }
                    else // no - just continue and retransmit the message
                    {
                        retransmit_current_message(bus);
                    }
                }
            }
            break;
        }

        if(oldState != bus->state)
        {
            NABTO_LOG_TRACE(("State change in bus %u: %s -> %s", bus->identifier, modbusStateNames[oldState], modbusStateNames[bus->state]));
        }
    }
}

modbus_message* modbus_rtu_master_allocate_message(void)
{
    modbus_message* message = (modbus_message*) checked_malloc(sizeof(modbus_message));

    if(message != NULL)
    {
        message->state = MODBUS_MESSAGE_STATE_ALLOCATED;
        message->remainingTransmissionAttempts = DEFAULT_MAXIMUM_TRANSMISSION_ATTEMPTS; // set default maximum number of transmission attempts
        message->maximumResponsetime = DEFAULT_RESPONSE_TIMEOUT;
        message->deferredRetransmissions = false;
        NABTO_LOG_TRACE(("Allocated message (message=%u).", (int)message));
    }
    else
    {
        NABTO_LOG_TRACE(("Message allocation failed (no free messages)!"));
    }

    return message;
}

void modbus_rtu_master_release_message(modbus_message* message)
{
    switch(message->state)
    {
    case MODBUS_MESSAGE_STATE_ALLOCATED:
        // message is held solely by the application
        break;

    case MODBUS_MESSAGE_STATE_QUEUED:
    case MODBUS_MESSAGE_STATE_TRANSFERRING:
        message->state = MODBUS_MESSAGE_STATE_DISCARDED; // can't remove from the transfer queue once inserted - signal pending release
        NABTO_LOG_TRACE(("Query discard requested (query=%u).", (int)message));
        return;

    case MODBUS_MESSAGE_STATE_COMPLETED: // completed and failed messages are both placed in the completed list
    case MODBUS_MESSAGE_STATE_FAILED:
        list_remove(&completedList, message);
        break;

    case MODBUS_MESSAGE_STATE_DISCARDED:
        break;
    }

    checked_free(message);

    NABTO_LOG_TRACE(("Released query (query=%u).", (int)message));
    NABTO_LOG_TRACE(("--------------------------------------"));
}

bool modbus_rtu_master_transfer_message(modbus_message* message)
{
    modbus* bus;

    if(message->bus >= MODBUS_NUMBER_OF_BUSSES) // invalid bus
    {
        NABTO_LOG_TRACE(("Attempted to enqueue a message on an invalid bus (query=%u)!", (int)message));
        return false;
    }
    
    bus = &busses[message->bus];

    if((message->frameSize + 2) > MODBUS_MAXIMUM_FRAME_SIZE) // not enough space to add CRC
    {
        NABTO_LOG_TRACE(("Attempted to enqueue an oversize message (bus=%u, query=%u)!", (int)bus->identifier, (int)message));
        return false;
    }

    // add CRC
    message->frameSize += 2;
    modbus_rtu_crc_update_crc_field(message->frame, message->frameSize);

    if(list_append(&bus->messageQueue, message)) // add to the end of the transfer queue
    {
        message->state = MODBUS_MESSAGE_STATE_QUEUED;
        NABTO_LOG_TRACE(("Query has been queued (bus=%u, query=%u, length=%u).", bus->identifier, (int)message, (int)message->frameSize));

        // eager tick when queue was empty
        if(list_count(&bus->messageQueue) == 1)
        {
            NABTO_LOG_TRACE(("Performing eager tick as message queue was empty (bus=%u).", bus->identifier));
            modbus_rtu_master_tick();
        }

        return true;
    }
    else
    {
        NABTO_LOG_ERROR(("Unable to enqueued query (bus=%u, query=%u, length=%u)!", bus->identifier, (int)message, (int)message->frameSize));
        return false;
    }
}

// function creation helper functions

void modbus_function_read_holding_register(modbus_message* message, uint8_t address, uint16_t startingAddress, uint16_t quantityOfRegisters)
{
    message->address = address;
    message->frame[0] = address;
    message->frame[1] = MODBUS_FUNCTION_READ_HOLDING_REGISTER;
    message->frame[2] = startingAddress >> 8;
    message->frame[3] = (uint8_t)startingAddress;
    message->frame[4] = quantityOfRegisters >> 8;
    message->frame[5] = (uint8_t)quantityOfRegisters;
    message->frameSize = 6;
}

// privates

static void retransmit_current_message(modbus* bus)
{
    modbus_message* message;
    list_peek_first(&bus->messageQueue, (void**)&message);

    if(--message->remainingTransmissionAttempts > 0) // retransmit message or drop it?
    {
        uint32_t compensatedReponseTimeout;

        if(message->deferredRetransmissions)
        {
            if(list_count(&bus->messageQueue) > 1) // should a deferred retransmission be attempted and does it make any sense to defer (only makes sense if more than one packet is in the queue)
            {
                if(list_append(&bus->messageQueue, message)) // add to end of queue
                {
                    list_remove(&bus->messageQueue, message); // remove current/first instance of message in transfer queue
                    message->state = MODBUS_MESSAGE_STATE_QUEUED;
                    bus->state = BUS_STATE_IDLE;
                    NABTO_LOG_TRACE(("Deferring retransmission of query (bus=%u, query=%u).", bus->identifier, (int)message));
                    return;
                }
            
                bus->state = BUS_STATE_IDLE;
                NABTO_LOG_TRACE(("Unable to defer query - retransmitting immediately (bus=%u, query=%u)!", bus->identifier, (int)message));
            }
            else
            {
                NABTO_LOG_TRACE(("Query allowed deferrence but transfer queue is empty (bus=%u, query=%u).", bus->identifier, (int)message));
            }
        }

        // retransmit immediately (also used as fallback if deferred retransmission fails)
        compensatedReponseTimeout = (uint32_t)message->maximumResponsetime + calculate_transmission_time(message->frameSize);
        
        uart_flush_receiver(bus->uartChannel);
        uart_write_buffer(bus->uartChannel, message->frame, message->frameSize);

        nabtoSetFutureStamp(&bus->responseTimeout, compensatedReponseTimeout); // time to wait for start of response: transmission time of outgoing frame + maximum response time
        bus->responseSize = 0;
        bus->responseOverrun = false;

        NABTO_LOG_TRACE(("Retransmitting query (bus=%u, remaining attempts=%u).", bus->identifier, (int)message->remainingTransmissionAttempts));
    }
    else
    {
        // mark message as failed
        message->state = MODBUS_MESSAGE_STATE_FAILED;

        // move message from transfer queue to completed list
        list_remove(&bus->messageQueue, message);
        list_append(&completedList, message);

        bus->state = BUS_STATE_IDLE;

        NABTO_LOG_TRACE(("Dropped query due too many retransmissions (bus=%u, message=%u).", bus->identifier, (int)message));
    }
}

// Calculate the number of milliseconds it takes to send the specified number of bytes at the current baudrate (actual wire time).
static uint32_t calculate_transmission_time(uint32_t numberOfBytes)
{
    return ((numberOfBytes * BITS_PER_CHARACTER * 1000000) / baudrate) / 1000;
}
