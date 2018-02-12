/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "unabto_env_base.h"
#include "gsn_includes.h"


static char print_buffer[1024];
GSN_UART_HANDLE_T nabtoUartHandle;

void nabto_gainspan_log(const char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(print_buffer, sizeof(print_buffer), fmt, arg_ptr);
    va_end(arg_ptr);

    GsnUart_Write(&nabtoUartHandle, (UINT8*)print_buffer, strlen(print_buffer), NULL, NULL);
}

void init_nabto_gainspan_logging() {
    GSN_UART_CONFIG_T nabtoUartConf;

    nabtoUartConf.eBaudRate = GSN_UART_BAUDRATE_115K2;
    nabtoUartConf.eCharFormat = GSN_UART_CHARFORMAT_8;
    nabtoUartConf.eParityBits = GSN_UART_PARITYBIT_NONE;
    nabtoUartConf.eStopBits = GSN_UART_STOPBITS_1;
    nabtoUartConf.HwFlowSupport = GSN_UART_HW_NO_SUPPORT;
    GsnUart_Open(GSN_UART_1, &nabtoUartHandle ,&nabtoUartConf);

    NABTO_LOG_TRACE(("Gainspan logging initialized"));
}
