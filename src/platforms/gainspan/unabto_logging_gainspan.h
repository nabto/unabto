/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_LOGGING_GAINSPAN_H_
#define _UNABTO_LOGGING_GAINSPAN_H_

#define NABTO_DEFAULT_LOG_MODULE(level, cmsg) do { nabto_gainspan_log cmsg; nabto_gainspan_log("\n"); } while(0)


#endif
