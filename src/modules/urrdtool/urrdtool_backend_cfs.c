#include <cfs.h>
#include "urrdtool_backend.h"

#include <stdio.h>
#include <string.h>

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

    cfs_remove(filename);
    int i;
    for (i = 0; i < SPLIT_IN_FILES; i++) {
        sprintf(filename, "urrd%i_%i.bin", id, i);
        cfs_remove(filename);
    }
    return true;
}

int urrd_header_file_init(uint8_t id) {
    char filename[16];
    memset(filename,0,16);
    int fd;
    sprintf(filename, "urrd%i.bin", id);
    fd = cfs_open(filename, CFS_READ | CFS_WRITE);

    return fd;
}

void urrd_delete_file(uint8_t id, uint8_t sector) {
    char filename[16];
    sprintf(filename, "urrd%i_%i.bin", id, sector);
    cfs_remove(filename);
}

void urrd_header_file_close(int fd) {
    cfs_close(fd);
}

int urrd_data_file_init(uint8_t id, uint8_t sector) {
    char filename[16];
    memset(filename,0,16);
    int fd;
    sprintf(filename, "urrd%i_%i.bin", id, sector);
    
    fd = cfs_open(filename, CFS_READ|CFS_WRITE);

    return fd;
}

void urrd_data_file_close(int fd) {
    cfs_close(fd);
}


bool urrd_open_table(urrd_table* table) {
    urrd_pointer pointer;
    return urrd_read_header(table, 0, &pointer);
}

urrdStatus urrd_create_table(uint8_t id, urrd_table* table) {
    // ensure the size is ok

    uint32_t sectorSize = (table->staticData.size / SPLIT_IN_FILES);
    if (sectorSize % table->staticData.data_length != 0) {
        return URRD_TABLE_UNALIGNED;
    }

    if (table->staticData.size != sectorSize * SPLIT_IN_FILES) {
        return URRD_TABLE_UNALIGNED;
    }
    return URRD_OK;
}

bool urrd_read_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data) {
    int fd = urrd_header_file_init(table->id);

    if (fd == -1) {
        return false;
    }
    
    size_t r = cfs_read(fd, data, URRD_POINTER_SIZE);
    bool ret = (r == URRD_POINTER_SIZE);

    urrd_header_file_close(fd);
    return ret;
}

bool urrd_write_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data) {
    int fd = urrd_header_file_init(table->id);
    if (fd == -1) {
        return false;
    }
    size_t w = cfs_write(fd, data, URRD_POINTER_SIZE);
    bool ret = (w == URRD_POINTER_SIZE);

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

    int fd = urrd_data_file_init(metaData->id, sector);
    if (fd == -1) {
        return false;
    }

    if (cfs_seek(fd, sectorOffset, CFS_SEEK_SET) == -1) {
        retVal = false;
    } else {   
        retVal =  (cfs_write(fd, data, dataLength));
    }

    urrd_data_file_close(fd);
    return retVal;
}

bool urrd_read_data(urrd_table* table, uint32_t offset, void* data, uint32_t dataLength) {
    uint32_t sectorSize = table->staticData.size/SPLIT_IN_FILES * table->staticData.data_length;
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

        int fd = urrd_data_file_init(table->id, sector % SPLIT_IN_FILES);
        if (fd == -1) {
            return false;
        }

        if (cfs_seek(fd, sectorOffset, SEEK_SET) == -1) {
            retVal = false;
        } else {
            size_t readLength = cfs_read(fd, data, toRead);
        
            // fill rest with undefined values
            if (readLength < toRead) {
                memset(data+readLength, 0, toRead-readLength);
                readLength = toRead;
            }
            dataLength -= readLength;
            offset += readLength;
            data += readLength;
        }
        urrd_data_file_close(fd);
    }



    return retVal;
}
