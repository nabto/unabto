/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Logging for uNabto RTX4100 - Implementation.
 */

#include "unabto/unabto_env_base.h"

#include <Protothreads/Protothreads.h>
#include <PtApps/AppCommon.h>

#include <Drivers/DrvLeuart.h>
#include <stdarg.h>

static char print_buffer[128];

void rtx4100_log(const char* fmt, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, fmt);
  vsnprintf(print_buffer, sizeof(print_buffer), fmt, arg_ptr);
  va_end(arg_ptr);
  DrvLeuartTxBuf(print_buffer, strlen(print_buffer));
}

void rtx4100_shell_log(const char* fmt, ...) {
  AppShellPrint(fmt);
  //AppShellPrint("\n");
}
