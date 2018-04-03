/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _CONFIGURATION_STORE_H_
#define _CONFIGURATION_STORE_H_

/*
 * Usage examples:
 *
 * typedef struct {
 *     uint32_t value;
 * } application_configuration;
 *
 * Initialization of configuration store:
 *   __ROM application_configuration defaultApplicationConfiguration = { 123456 };
 *   bool success = configuration_store_initialize(&defaultApplicationConfiguration);
 *
 * Formatting the configuration store (only necessary when fields are changed, added or removed from the application_configuration type - done automatically if the configuration store is invalid):
 *   bool success = configuration_store_format(&defaultApplicationConfiguration);
 *
 * Read from configuration store:
 *   uint32_t copyOfValue;
 *   bool success = configuration_store_read(offsetof(application_configuration, value), &copyOfValue, sizeof(copyOfValue));
 *
 * Write to configuration store:
 *   uint32_t copyOfValue;
 *   bool success = configuration_store_write(offsetof(application_configuration, value), &copyOfValue, sizeof (copyOfValue));
 *
 * Set in configuration store:
 *   bool success = configuration_store_set(offsetof(application_configuration, value), 0, sizeof (copyOfValue));
 *
 */

#include <unabto/unabto_env_base.h>

#if NABTO_CONFIGURATION_STORE_ENABLE

#include <application_configuration.h>

#ifdef __cplusplus
extern "C"
{
#endif

  bool configuration_store_initialize(__ROM application_configuration* defaultApplicationConfiguration);
  bool configuration_store_format(__ROM application_configuration* defaultApplicationConfiguration);
  bool configuration_store_read(uint16_t offset, void* data, uint16_t length);
  bool configuration_store_write(uint16_t offset, const void* data, uint16_t length);
  bool configuration_store_set(uint16_t offset, uint8_t value, uint16_t length);
  bool configuration_store_compare(uint16_t offset, const void* data, uint16_t length, bool* match);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

#endif
