#include <modules/urrdtool/urrdtool_backend.h>
#include "urrd_file_backend.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define HEADER_FILE "header.bin"

#define SPLIT_IN_FILES 4

bool urrd_erase_table(uint8_t id);

bool urrd_init(bool erase) {
    if (erase) {
        int i;
        for(i = 0; i < MAX_NUMBER_OF_URRD; i++) {
            urrd_erase_table(i);
        }
    }
    return true;
}

bool urrd_erase_table(uint8_t id) {
    char filename[16];
    sprintf(filename, "urrd%i.bin", id);

    unlink(filename);
    int i;
    for (i = 0; i < SPLIT_IN_FILES; i++) {
        sprintf(filename, "urrd%i_%i.bin", id, i);
        unlink(filename);
    }
    return true;
}

FILE* urrd_data_file_init(uint8_t id, uint8_t sector) {
    char filename[16];
    FILE* fd;
    sprintf(filename, "urrd%i_%i.bin", id, sector);
    
    fd = fopen(filename, "r+"); /* simple stat */

    if (NULL != fd) {
        return fd;
    }

    fd = fopen(filename, "w+");
    
    if (NULL == fd) {
        return false;
    }

    return fd;
}

void urrd_delete_file(uint8_t id, uint8_t sector) {
    char filename[16];
    sprintf(filename, "urrd%i_%i.bin", id, sector);
    unlink(filename);
}


void urrd_data_file_close(FILE* fd) {
    fclose(fd);
}


FILE* urrd_header_file_init(uint8_t id) {
    char filename[16];
    FILE* fd;
    sprintf(filename, "urrd%i.bin", id);
    fd = fopen(filename, "r+"); /* simple stat */

    if (NULL != fd) {
        return fd;
    }

    fd = fopen(filename, "w+");
    
    if (NULL == fd) {
        return false;
    }

    return fd;
}

void urrd_header_file_close(FILE* fd) {
    fclose(fd);
}

bool urrd_open_table(urrd_table* table) {
    char filename[16];
    FILE* fd;
    sprintf(filename, "urrd%i.bin", table->id);
    fd = fopen(filename, "r+"); /* simple stat */

    if (NULL != fd) {
        fclose(fd);
        return true;
    }
    return false;
}

urrdStatus urrd_create_table(uint8_t id, urrd_table* table) {
    // ensure the size is ok

    uint32_t sectorSize = (table->staticData.size / SPLIT_IN_FILES);
    
    if (table->staticData.size != sectorSize * SPLIT_IN_FILES) {
        return URRD_TABLE_UNALIGNED;
    }
    return URRD_OK;
}

bool urrd_write_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data) {
    FILE* fd = urrd_header_file_init(table->id);
    if (fd == NULL) {
        return false;
    }
    size_t w = fwrite(data,1, URRD_POINTER_SIZE, fd);
    bool ret = (w == URRD_POINTER_SIZE);

    urrd_header_file_close(fd);
    
    return ret;
}

bool urrd_read_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data) {
    FILE* fd = urrd_header_file_init(table->id);

    size_t r = fread(data,1, URRD_POINTER_SIZE, fd);
    bool ret = (r == URRD_POINTER_SIZE);

    urrd_header_file_close(fd);
    
    return ret;
}

bool urrd_write_data(urrd_table* metaData, uint32_t offset, const void* data, uint32_t dataLength) {
    uint32_t sectorSize = (metaData->staticData.size/SPLIT_IN_FILES) * metaData->staticData.data_length;

    uint8_t sector = offset/sectorSize;
    uint32_t sectorOffset = offset - (sector*sectorSize);
    bool retVal;

    if (sectorOffset == 0) {
        urrd_delete_file(metaData->id, sector);

        uint32_t splitDeleteCount = (metaData->staticData.size/SPLIT_IN_FILES)*(SPLIT_IN_FILES-1);

        if (metaData->pointer.count > splitDeleteCount) {
            metaData->pointer.count = splitDeleteCount;
        }
    }

    FILE* fd = urrd_data_file_init(metaData->id, sector);
    if (fd == NULL) {
        return false;
    }

    if (fseek(fd, sectorOffset, SEEK_SET) == -1) {
        retVal = false;
    } else {   
        retVal =  (fwrite(data, 1, dataLength, fd) == dataLength);
    }

    urrd_data_file_close(fd);
    return retVal;
}

bool urrd_read_data(urrd_table* table, uint32_t offset, void* data, uint32_t dataLength) {
    uint32_t sectorSize = (table->staticData.size/SPLIT_IN_FILES) * table->staticData.data_length;
    bool retVal = true;

    while(dataLength > 0 && retVal == true) {
        uint32_t toRead;
        uint8_t sector = offset/sectorSize;
        uint32_t sectorOffset = offset - (sector*sectorSize);

        uint32_t sectorBegin = offset;
        uint32_t sectorEnd = (sector+1)*sectorSize;

        if (sectorBegin + dataLength > sectorEnd) {
            toRead = sectorEnd - sectorBegin;
        } else {
            toRead = dataLength;
        }

        FILE* fd = urrd_data_file_init(table->id, sector % SPLIT_IN_FILES);
        if (fd == NULL) {
            return false;
        }

        if (fseek(fd, sectorOffset, SEEK_SET) == -1) {
            retVal = false; 
        } else {
            size_t readLength = fread(data, 1, toRead, fd);
        
            if (readLength == 0) {
                if (feof(fd)) {
                    retVal = false;
                }
            }

            // fill rest with undefined values
            if (readLength < toRead) {
                if (ferror(fd)) {
                    retVal = false;
                } 
            }
            dataLength -= readLength;
            offset += readLength;
            data += readLength;
        }
        urrd_data_file_close(fd);
    }



    return retVal;
}

