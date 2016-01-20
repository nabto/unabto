#ifndef _UNABTO_PROVISION_FILE_H
#define _UNABTO_PROVISION_FILE_H

#define UNABTO_PROVISION_FILE_DELIMITER     ':'

/**
 * Definition of read and write function handles
 * @param text        fill with characters / write content to persistent storage
 * @param size        max size / size of text
 * return             true if successful
 */
typedef bool (*unabtoPersistentFunction)(char* text, size_t size);

bool unabto_provision_parse_data(nabto_main_setup *nms, char *data, char *key);
bool unabto_provision_read_file(const char* path, char *text, size_t size);
bool unabto_provision_test_create_file(const char* path);
bool unabto_provision_write_file(const char* path, nabto_main_setup* nms);

#endif

