
/*
 * File:   urrdtool.h
 * Author: Nabto
 *
 * Created on 28. november 2013, 14:37
 */

#ifndef _URRD_H_
#define _URRD_H_

#include "urrdtool_config.h"

typedef enum {
    URRD_OK,
    URRD_ARGS_DIFFERENT,
    URRD_COULD_NOT_LOAD_TABLE,
    URRD_TABLE_ALREADY_CREATED,
    URRD_TABLE_UNALIGNED,
    URRDS_SHOULD_BE_CREATED_SEQUENTIALLY,
    URRD_WRITE_ERROR
} urrdStatus;

/**
 * Declaration of consolidation function
 *
 * @param  urrd_id              the id of the uRRD which have called the consolidation
 * @param  bounded_urrd_id      the id of the bounded uRRD (where to store the processed data)
 * @param  number_of_records    number of records to process
 */
typedef void (*consolidation_function)(uint8_t urrd_id, uint8_t bounded_urrd_id, uint16_t number_of_records);



#define URRD_POINTER_LIFE_TIME              10
#define URRD_POINTER_SIZE                   sizeof(urrd_pointer)
#define URRD_POINTER_BUFFER_START_ADDR      10
#define URRD_POINTER_BUFFER_SIZE            NUMBER_OF_URRD_POINTERS_PER_URRD*URRD_POINTER_SIZE
#define URRD_POINTER_BUFFER_END_ADDR        (URRD_POINTER_BUFFER_START_ADDR + MAX_NUMBER_OF_URRD*NUMBER_OF_URRD_POINTERS_PER_URRD*URRD_POINTER_SIZE)
#define DATABASE_START_ADDR                 URRD_POINTER_BUFFER_END_ADDR + 10

#define EEPROM_LAYOUT_ENTRY_SIZE              sizeof(eeprom_layout)

typedef struct {
    uint8_t id;
    urrd_eeprom_size data_start_addr;
    urrd_eeprom_size data_end_addr;
    uint8_t checksum; // checksum of header.
} __attribute__((packed)) eeprom_layout;

typedef struct {
    urrd_size size; // the size of the urrd in records;
    uint8_t data_length;
    uint32_t write_frequency;
} __attribute__((packed)) static_urrd_header;

typedef struct {
    urrd_size index; // current index head points at. This is the next entry to write. Index is a record index.
    urrd_size count; // current number of entries in the table.
    uint32_t date_time; // last write stamp
    uint8_t checksum; // checksum of header.
} __attribute__((packed)) urrd_pointer;


typedef struct {
#if URRD_BACKEND_EEPROM
    eeprom_layout layout;
#endif
    static_urrd_header staticData;
    urrd_pointer pointer;
    uint8_t id; 
    uint32_t consolidation_frequency;
    consolidation_function consolidation_func;
    uint8_t bounded_urrd_id;
    bool isNew;
} __attribute__((packed)) urrd_table;


/**
 * Initializing of the uRRDtool, only if it was not initialized. This function have to be run as the first
 */
bool urrd_init(bool erase);

/**
 * Create a uRRD
 *
 * 
 *
 * @param  size                         the size of uRRD to create (in records)
 * @param  record_length                the length of record
 * @param  write_frequency              how often is data stored in the uRRD (in second)
 * @param  consolidation_func           a function reference to the data processing function for consolidation
 * @param  consolidation_frequency      how often is the consolidation called (in records)
 * @param  bounded_urrd_id              the id of the bounded uRRD (where to store the processed data)
 * @return                              the id of the created uRRD
 */
urrdStatus urrd_create(
    uint8_t urrdId,
    urrd_size size,
    uint8_t record_length,
    uint32_t write_frequency,
    consolidation_function
    consolidation_func,
    uint32_t consolidation_frequency,
    uint8_t bounded_urrd_id);

/**
 * Write to a specific uRRD
 *
 * @param  id                   the id of the uRRD
 * @param  data                 data to write
 */
bool urrd_write(uint8_t id, const void* data);

/**
 * Read from a specifik uRRD by a timestamp,
 * 
 * If timestamp is in the future: return undefined until defined records occurs
 *
 * If timestamp is in the past: return no entries.
 *
 *
 * @param  id                   the id of the uRRD
 * @param  buf                  buffer to the readed data
 * @param  time_stamp           the time_stamp of the record to read
 * @param  length               size of buffer
 * @param  from                 The start timestamp of the data.
 * @param  to                   The end timestamp of the data.
 * @return                      returns number of records read
 */
urrd_size urrd_read_by_timestamp(uint8_t id, uint32_t time_stamp, void* buf, urrd_size length, uint32_t* dataTimestamp);

bool urrd_get_status(uint8_t id);
urrd_size get_urrd_count(uint8_t id);

uint8_t calc_checksum(uint8_t* ptr, uint8_t length);


#endif
