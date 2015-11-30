#include <inttypes.h>
#include <stdbool.h>

#define NUMBER_OF_URRD_POINTERS_PER_URRD    4
#define MAX_NUMBER_OF_URRD                  8
#define NULL_RECORDS                        0xFF

#ifdef URRD_LOG
#include <stdio.h>
#include "unabto/unabto.h"
#endif


#ifdef URRD_LOG
#define URRD_LOG_INFO(msg) NABTO_LOG_INFO(msg)
#else
#define URRD_LOG_INFO(msg)
#endif

#define platform_eeprom_read(addr, value, length) platform_read(addr, value, length)
#define platform_eeprom_write(addr, value, length) platform_write(addr, value, length)

#define platform_get_time_stamp() urrd_time()


typedef uint32_t urrd_size;
typedef uint32_t urrd_time_stamp;
typedef uint32_t urrd_eeprom_size;
