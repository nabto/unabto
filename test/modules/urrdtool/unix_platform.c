#include <modules/urrdtool/urrdtool.h>


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define IMAGE_FILE "flash.img"

#define PREALLOC_BLOCK_SIZE 1024
#define PREALLOC_BLOCKS     (400)

static FILE *fd = NULL;

static bool urrd_eeprom_init()
{
    int i;
    char *buf;
    fd = fopen(IMAGE_FILE, "r+"); /* simple stat */

    if (NULL != fd)
        return true;

    fd = fopen(IMAGE_FILE, "w+");

    if (NULL == fd)
    {
        //         NABTO_LOG_ERROR(("Unable to open image file: %s", IMAGE_FILE));
        return false;
    }

    buf = malloc(PREALLOC_BLOCK_SIZE);
    memset(buf, 0x00, PREALLOC_BLOCK_SIZE);
    for (i = 0; i < PREALLOC_BLOCKS; i++) {
        fwrite(buf, 1, PREALLOC_BLOCK_SIZE, fd);
    }

    free(buf);
    return true;
}

bool urrdtool_eeprom_write(uint32_t index, void* data, uint32_t dataLength)
{
    if (NULL == fd && !urrd_eeprom_init()) {
        return false;
    }
    if (fseek(fd, index, SEEK_SET) != 0) {
        return false;
    }

    if (dataLength != fwrite(data, 1, dataLength, fd)) {
        return false;
    }

    return true;
}
bool urrdtool_eeprom_read(uint32_t index, void* data, uint32_t readLength)
{
    int len;
    if (NULL == fd && !urrd_eeprom_init()) {
        return false;
    }
    if (fseek(fd, (long)index, SEEK_SET) != 0) {
        return false;
    }

    if (-1 == (len = fread(data, 1, readLength, fd))) {
        perror("READ ERROR\n");
        return false;
    } else {
        printf("Len : %d\n", len);
    }
    return true;
}
