#ifndef __BSP_DRIVER_H__
#define __BSP_DRIVER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Nano1X2Series.h"
#include "rak_config.h"
#include "rak_global.h"

uint8_t  rak_UART_send(uint8_t *tx_buf,uint16_t buflen);

#endif
