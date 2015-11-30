#include "cfs.h"
#include "cfs-coffee-arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "cfs-coffee-unix.h"
#include <stdbool.h>


const char* filename = "fs.bin";
static FILE* fd = 0;

static bool shouldExit = false;

void exit_at_next_operation() {
    shouldExit = 1;
}


void init_fs() {
    if (fd == 0) {
        fd = fopen("fs.bin", "r+");
        if (fd == NULL) {
            printf("could not open raw filesystem\n");
            exit(1);
        }
    }
}


// Underlying flash implementation:
void my_write(const void* data, int datalen, cfs_offset_t offset) {
    
    if (offset < 0) {
        return;
    }

    if (shouldExit) {
        exit(0);
    }
    init_fs();
    uint8_t data2[datalen];
    if (fseek(fd, offset, SEEK_SET) != 0) {
        printf("Write seek failed\n");
        exit(1);
    }

    if (fread(data2, 1, datalen, fd) != datalen) {
        printf("verify fread failed\n");
        exit(1);
    }
    
    int i;
    for (i = 0; i < datalen; i++) {
        if(( (((char*)data2)[i]) & ~(((char*)data)[i])) > 0) {
            printf("flipping 1 to 0 not allowed data %i, olddata %i\n", ((char*)data)[i], ((char*)data2)[i]);
            
            ((uint8_t*)data)[i] |= data2[i];

            //exit(1);
        } 
    }

    if (fseek(fd, offset, SEEK_SET) != 0) {
        printf("Write seek failed\n");
        exit(1);
    }
    if (fwrite(data, 1, datalen, fd) != datalen) {
        printf("Could not write data\n");
        exit(1);
    }
}

void my_read(const void* data, int datalen, cfs_offset_t offset) {

    if (shouldExit) {
        exit(0);
    }

    init_fs();
    if (datalen == 0) {
        return;
    }
    if (fseek(fd, offset, SEEK_SET) != 0) {
        printf("Read seek failed\n");
        exit(1);
    }
    size_t readlen = fread((void*)data, 1, datalen, fd);
    if (readlen != datalen) {
        printf("fread failed read: %i, datalen: %i, offset %i\n",readlen, datalen, offset);
        exit(1);
    }
}

void my_erase(int sector) {
    if (shouldExit) {
        exit(0);
    }

    init_fs();
    printf("Erasing sector %i\n", sector);
    int offset = COFFEE_SECTOR_SIZE * sector;
    if (fseek(fd,offset, SEEK_SET) != 0) {
        printf("erase seek failed\n");
        exit(1);
    }
    uint8_t empty[COFFEE_SECTOR_SIZE];
    memset(empty, 0, sizeof(empty));
    if (fwrite(empty, 1, sizeof(empty), fd) != sizeof(empty)) {
        printf("Write failed\n");
        exit(1);
    }
}
