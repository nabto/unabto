/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_DIAG_H_
#define _UNABTO_DIAG_H_

#include <unabto/unabto_env_base.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void unabto_printf_unabto_config(FILE * f, const char *progname);
extern void unabto_printf_memory_sizes(FILE * f, const char *progname);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
