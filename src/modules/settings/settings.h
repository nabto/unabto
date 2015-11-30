/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

bool settings_read_string(const char* key, char* value);
bool settings_write_string(const char* key, const char* value);

bool settings_read_int(const char* key, int* value);
bool settings_write_int(const char* key, int value);

bool settings_read_bool(const char* key, bool* value);
bool settings_write_bool(const char* key, bool value);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
