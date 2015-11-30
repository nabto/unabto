#include <stdlib.h>
#include "urrdtool.h"
#include <math.h>
#include <string.h>
#include <stddef.h>
#include "urrdtool_backend.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif


#if UNABTO_PLATFORM_PIC18
#pragma udata big_mem
#endif

static urrd_table urrd_table_list[MAX_NUMBER_OF_URRD];

#if UNABTO_PLATFORM_PIC18
#pragma udata
#endif

const unsigned char CRC7_POLY = 0x91;

void urrd_renew_pointer(urrd_pointer* old_ptr);
void urrd_save_ptr(urrd_table* table);
urrd_table* urrd_get_table(uint8_t id);
bool fill_undefined_values(urrd_table* table, urrd_size number_of_records);
bool urrd_value_written(urrd_table* table);
bool reset_urrd(urrd_table* table);

uint8_t calc_pointer_checksum(urrd_pointer* pointer) {
    return calc_checksum((uint8_t*)pointer, offsetof(urrd_pointer, checksum));
}


void urrd_save_ptr(urrd_table* table)
{
    URRD_LOG_INFO(("uRRDtool: Saving uRRD_ptr (ptrId=%i, index=%i, datetime=%i)\n", table->id, table->pointer.index, table->pointer.date_time));
    table->pointer.checksum = calc_pointer_checksum(&table->pointer);
    
}

urrd_table* urrd_get_table(uint8_t id)
{
    return &urrd_table_list[id];
}

bool readAndFindNewestPointer(urrd_table* table) {
    uint32_t maxTimeStamp = 0;
    int maxPointerNo = -1;
    uint8_t i;
    for (i = 0; i < NUMBER_OF_URRD_POINTERS_PER_URRD; i++) {
        urrd_pointer p;
        if (urrd_read_header(table, i, &p)) {
            uint8_t checkSum = calc_pointer_checksum(&p);
            if (p.checksum != checkSum) {
                continue;
            }
            if (p.date_time > maxTimeStamp) {
                maxTimeStamp = p.date_time;
                maxPointerNo = i;
            }
        }
    }
    
    if (maxPointerNo == -1) {
        // No valid pointers found, start from zero.
        memset(&table->pointer, 0, sizeof(urrd_pointer));
        return 0;
    }
    
    if (!urrd_read_header(table, maxPointerNo, &table->pointer)) {
        return false;
    }

    return true;
}

bool check_urrd_args(urrd_table* table, urrd_size size, uint8_t record_length, uint32_t write_frequency) {
    if (table->staticData.size != size) {
        return false;
    }

    if (table->staticData.data_length != record_length) {
        return false;
    }

    if (table->staticData.write_frequency != write_frequency) {
        return false;
    }
    return true;
}

urrdStatus urrd_create(uint8_t id, urrd_size size, uint8_t record_length, uint32_t write_frequency, consolidation_function consolidation_func, uint32_t consolidation_frequency, uint8_t bounded_urrd_id)
{
    urrd_table* table = urrd_get_table(id);

    table->staticData.size = size;
    table->staticData.data_length = record_length;
    table->staticData.write_frequency = write_frequency;
    table->id = id;

    if (urrd_open_table(table)) {
        //if (!check_urrd_args(&table, size, record_length, write_frequency)) {
        //    return URRD_ARGS_DIFFERENT;
        //}
        if (!readAndFindNewestPointer(table)) {
            return URRD_COULD_NOT_LOAD_TABLE;
        }
    } else {
        urrdStatus st = urrd_create_table(id, table);
        if (st != URRD_OK) {
            return st;
        }
        reset_urrd(table);
    }
    return URRD_OK;
}

bool reset_urrd(urrd_table* table) {
    uint8_t i;

    table->pointer.index = 0;
    table->pointer.count = 0;
    table->pointer.date_time = platform_get_time_stamp();
    table->pointer.checksum = calc_pointer_checksum(&table->pointer);
    
    for (i = 0; i < NUMBER_OF_URRD_POINTERS_PER_URRD; i++) {
        if (!urrd_write_header(table, i, &table->pointer)) {
            return false;
        }
    }
    return true;
}

bool urrd_write(uint8_t id, const void* data) {
    urrd_table* table = urrd_get_table(id);
     // First check if we need to write some missing entries
    int missingRecordsDiff = ((platform_get_time_stamp() - table->pointer.date_time - (table->staticData.write_frequency/2)));
  
    if (platform_get_time_stamp() < table->pointer.date_time) {
        // TIME is in the past
        return false;
    }
    
    if (missingRecordsDiff > (int)table->staticData.write_frequency) {
        urrd_size number_of_missing_records;
        number_of_missing_records = missingRecordsDiff/table->staticData.write_frequency;
        if (number_of_missing_records > table->staticData.size) {
            // Time is hugely in the future, start a completely new dataset
            // TODO support aggregation.
            if (!reset_urrd(table)) {
                return false;
            }
        } else {
            if (!fill_undefined_values(table, number_of_missing_records)) {
                return false;
            }
        }
    }

    if (!urrd_write_data(table, table->pointer.index * table->staticData.data_length, data, table->staticData.data_length)) {
        return false;
    }

    return urrd_value_written(table);
}

bool urrd_value_written(urrd_table* table)
{
    URRD_LOG_INFO(("uRRDtool: Writing in uRRD (index=%i, timestamp=%i)\n", table->pointer.index, (int)platform_get_time_stamp()));

    if (table->consolidation_frequency > 0) {
        if ((table->pointer.index % table->consolidation_frequency) == 0) {
            // consolidate
            if (table->consolidation_func != NULL) {
                table->consolidation_func(table->id, table->bounded_urrd_id, table->consolidation_frequency);
            }
        }
    }

    table->pointer.index = (table->pointer.index + 1) % table->staticData.size;
    table->pointer.date_time = platform_get_time_stamp();

    if (table->pointer.count < table->staticData.size) {
        table->pointer.count++;
    }

    

    table->pointer.checksum = calc_pointer_checksum(&table->pointer);
    urrd_write_header(table, (table->pointer.index % NUMBER_OF_URRD_POINTERS_PER_URRD), &table->pointer);

    return true;
}

urrd_size urrd_read_by_timestamp(uint8_t id, uint32_t time_stamp, void* buf, urrd_size length, uint32_t* to)
{
    urrd_table* db_ptr = urrd_get_table(id);
    urrd_size read_index;
    uint32_t timeSeriesStart;
    uint32_t timeSeriesEnd;
    int32_t timeSeriesLength;
    uint32_t timeSeriesDataLength;
    urrd_size back_steps;
    urrd_size dataLength;

    if (db_ptr->staticData.write_frequency != 0)
    {
        urrd_size maxEntries = length/db_ptr->staticData.data_length;
        uint32_t startTime = time_stamp - maxEntries * db_ptr->staticData.write_frequency;
        int32_t rrdStartTime = db_ptr->pointer.date_time - (db_ptr->staticData.write_frequency * db_ptr->pointer.count);

        if (rrdStartTime < 0) {
            return 0;
        }

        timeSeriesStart = MAX(rrdStartTime, startTime);
        
        timeSeriesEnd = MIN(db_ptr->pointer.date_time, time_stamp);

        timeSeriesLength = timeSeriesEnd - timeSeriesStart;

        if (timeSeriesLength < 0) {
            return 0;
        } 
        
        timeSeriesDataLength = (timeSeriesLength+(db_ptr->staticData.write_frequency-1))/db_ptr->staticData.write_frequency;
        
        // Handle the case where we read data which is all in front of the data we have.
        if (timeSeriesStart > db_ptr->pointer.date_time) {
            // We are just reading undefined here.
            memset(buf, NULL_RECORDS, length);
            return length/db_ptr->staticData.data_length;
        }

        if (timeSeriesEnd < rrdStartTime) {
            return 0;
        }

        if (timeSeriesStart > timeSeriesEnd) {
            return 0;
        }

        back_steps = ((db_ptr->pointer.date_time - timeSeriesStart) + (db_ptr->staticData.write_frequency-1))  / db_ptr->staticData.write_frequency;
        read_index = ((db_ptr->staticData.size + db_ptr->pointer.index) - back_steps) % db_ptr->staticData.size;

        URRD_LOG_INFO(("uRRDtool: Data readed (read_index=%i, back_steps=%i, length=%u)\n", read_index, back_steps, length));

        dataLength = timeSeriesDataLength*db_ptr->staticData.data_length;

        if (!urrd_read_data(db_ptr, read_index*db_ptr->staticData.data_length, buf, dataLength)) {
            return 0;
        }
        *to = timeSeriesEnd;
        return timeSeriesDataLength;
    }
    else {
        URRD_LOG_INFO(("uRRDtool: The time_stamp is newer than data (ptr_datetime=%i, date_time=%i)\n", db_ptr->pointer.date_time, time_stamp));
        return 0;
    }
}

bool fill_undefined_values(urrd_table* table, urrd_size number_of_records)
{
    int i,j;
    
    uint8_t nullbyte = NULL_RECORDS;

    for (i = 0; i < number_of_records; i++) {
        for (j = 0; j < table->staticData.data_length; j++) {
            if (!urrd_write_data(table, (table->pointer.index * table->staticData.data_length)+j, &nullbyte , sizeof(nullbyte))) {
                return false;
            }
        }
        urrd_value_written(table);
    }
    return true;

}

uint8_t calc_checksum(uint8_t* ptr, uint8_t length)
{
    uint8_t data = 0;
    uint8_t i;
    uint8_t crc_sum = 0;


    for (i = 0; i < length; i++)
    {
        data = *ptr;
        data ^= CRC7_POLY;
        crc_sum += data;
        ptr++;
    }
    return crc_sum;
}

urrd_size get_urrd_count(uint8_t id)
{
    urrd_table* db_ptr = urrd_get_table(id);
    return db_ptr->pointer.count;
}

bool urrd_get_status(uint8_t id)
{
    urrd_table* dp_ptr = urrd_get_table(id);
    return dp_ptr->isNew;
}
