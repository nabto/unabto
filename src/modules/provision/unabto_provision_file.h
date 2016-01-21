#ifndef _UNABTO_PROVISION_FILE_H
#define _UNABTO_PROVISION_FILE_H

/**
 * Storing and parsing of provisioning data.
 */

#define UNABTO_PROVISION_FILE_DELIMITER     ':'

/**
 * Parse provision data, write id to nms struct and crypto key to key parameter.
 */
bool unabto_provision_parse_data(nabto_main_setup *nms, char *data, char *key);

/**
 * Test that it is possible to create a file. 
 *
 * TODO: NABTO-1170 Provisioning file is too easily truncated and previous data lost
 */
bool unabto_provision_test_create_file(const char* path);

/**
 * Write id and key information from nms struct to specified location.
 */
bool unabto_provision_write_file(const char* path, nabto_main_setup* nms);

/**
 * Read file into text buffer.
 */
bool unabto_provision_read_file(const char* path, char *text, size_t size);

#endif

