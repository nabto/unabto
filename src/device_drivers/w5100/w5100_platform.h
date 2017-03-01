#ifndef _W5100_PLATFORM_H_
#define _W5100_PLATFORM_H_

/**
 * This defines the external functions this module needs to function work
 */

/**
 * spi read/write.  This function needs to set the w5100 chip select output physically low make
 * the transfer and then set the chip select to physically high again.
 */

void w5100_spi_transfer_buffer(uint8_t* data, uint16_t size);

#endif
