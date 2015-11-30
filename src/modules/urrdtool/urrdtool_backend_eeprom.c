#include "urrdtool.h"
#include "urrdtool_backend.h"
#include "urrdtool_backend_eeprom.h"

#include <string.h>
#include <stddef.h>

uint32_t init_magic_value = 0xAAAAAAAA;

uint8_t calc_layout_checksum(eeprom_layout* layout) {
    return calc_checksum((uint8_t*)layout, offsetof(eeprom_layout, checksum));
}


bool urrd_init(bool force_clean)
{
    uint32_t init_check;
    uint8_t init_ff[4];
    uint8_t init_value = 255;
    memset (init_ff, init_value, sizeof(init_ff));
    urrdtool_eeprom_read(0, &init_check, sizeof(init_check));

    if (init_check != init_magic_value || force_clean == true)
    {
        int i;
        URRD_LOG_INFO(("uRRDtool: Initialising...\n"));
        for (i=URRD_POINTER_BUFFER_START_ADDR; i<URRD_POINTER_BUFFER_END_ADDR+sizeof(init_ff); i+=sizeof(init_ff))
        {
            urrdtool_eeprom_write(i, (const void*)init_ff, sizeof(init_ff));
        }

        urrdtool_eeprom_write(0, (const void*)&init_magic_value, 4);
        URRD_LOG_INFO(("uRRDtool: Initialising done, the uRRD is cleaned\n"));
    }

    return true;
}

bool urrd_read_layout(uint8_t id, eeprom_layout* layout) {
    urrd_size layoutIndex = URRD_POINTER_BUFFER_START_ADDR + (EEPROM_LAYOUT_ENTRY_SIZE*id);
    if (!urrdtool_eeprom_read(layoutIndex, layout, sizeof(eeprom_layout))) {
        return false;
    }

    if (layout->checksum != calc_layout_checksum(layout)) {
        return false;
    }
 

    return true;
}

urrdStatus urrd_create_table(uint8_t id, urrd_table* table) {
    urrd_size startAddr;
    eeprom_layout layout;
    urrd_size layoutIndex;
    
    if (urrd_read_layout(id, &layout)) {
        if (layout.id != 255) {
            return URRD_TABLE_ALREADY_CREATED;
        }
    }
    
    if (id > 0) {
        if (!urrd_read_layout(id-1, &layout)) {
            return URRDS_SHOULD_BE_CREATED_SEQUENTIALLY;
        }
        startAddr = layout.data_end_addr;
    } else {
        startAddr = DATABASE_START_ADDR;
    }
    
    table->layout.id = id;
    table->layout.data_start_addr = startAddr;
    table->layout.data_end_addr = startAddr + (sizeof(urrd_pointer) * NUMBER_OF_URRD_POINTERS_PER_URRD) + table->staticData.size;
    table->layout.checksum = calc_layout_checksum(&table->layout);

    layoutIndex = URRD_POINTER_BUFFER_START_ADDR + (EEPROM_LAYOUT_ENTRY_SIZE*id);

    if (!urrdtool_eeprom_write(layoutIndex, &table->layout, sizeof(eeprom_layout))) {
        return URRD_WRITE_ERROR;
    }

    return URRD_OK;
} 

bool urrd_open_table(urrd_table* table) {
    if (!urrd_read_layout(table->id, &table->layout)) {
        return false;
    }
    
    if (table->layout.id != table->id) {
        return false;
    }
    
    return true;
}

bool urrd_write_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data) {
    urrd_size index = table->layout.data_start_addr + (pointerNo * URRD_POINTER_SIZE);
    
    urrdtool_eeprom_write(index, (void*)data, URRD_POINTER_SIZE);
    return true;
}

bool urrd_read_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data) {
    urrd_size index = table->layout.data_start_addr + (pointerNo * URRD_POINTER_SIZE);
    urrdtool_eeprom_read(index, (void*)data, URRD_POINTER_SIZE);
    return true;
}

bool urrd_write_data(urrd_table* table, urrd_size offset, const void* data, urrd_size dataLength) {
    urrd_size index = table->layout.data_start_addr + URRD_POINTER_BUFFER_SIZE + offset;
    
    urrdtool_eeprom_write(index, data, dataLength);
    return true;
}

bool urrd_read_data(urrd_table* table, urrd_size offset, void* data, urrd_size dataLength) {
    urrd_size index;
    
    urrd_size readBegin = offset;
    urrd_size readEnd = (offset + dataLength);

    urrd_size dataEnd = table->staticData.size * table->staticData.data_length;

    if (dataEnd < readEnd) {
        urrd_size toRead = dataEnd - readBegin;
    
        index = table->layout.data_start_addr + URRD_POINTER_BUFFER_SIZE + offset;
        urrdtool_eeprom_read(index, data, toRead);
        
        data += toRead;
        offset = 0;
        dataLength -= toRead;
    }
    
    index = table->layout.data_start_addr + URRD_POINTER_BUFFER_SIZE + offset;

    urrdtool_eeprom_read(index, data, dataLength);
    return true;
}
