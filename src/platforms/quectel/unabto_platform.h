/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_H_
#define _UNABTO_PLATFORM_H_

#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_pin.h"
#include "Ql_multitask.h"
#include "Ql_tcpip.h"
#include "Ql_fcm.h"

#include <unabto_platform_types.h>
#include <platforms/unabto_common_types.h>

#ifdef __cplusplus
extern "C" {
#endif


#define NABTO_LOG_BASIC_PRINT(Severity,msg) Ql_DebugTrace msg

//extern APP_CONFIG AppConfig; // expose the TCP/IP configuration structure so the application can access it.


#ifdef __cplusplus
} //extern "C"
#endif

#endif
