/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Logging for uNabto Renesas RL78 - Implementation.
 */

#include <system\console.h>
#include "unabto/unabto_env_base.h"

/**
 * Log Nabto messages to Renesas UART.
 * @param message  the message to print
 */
void nabto_renesas_rl78_log(char *message) {
  // printf(message)
  ConsolePrintf(message);
}