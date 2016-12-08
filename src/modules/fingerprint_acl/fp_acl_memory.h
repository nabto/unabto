#ifndef _FP_ACL_MEMORY_H_
#define _FP_ACL_MEMORY_H_

#include "fp_acl.h"

/**
 * In memory fingerprint acl database
 */

void fp_mem_init();

// iterator to iterate through all the users

// get iterator to first user
void* fp_mem_get_first_user();

// iterator++
void* fp_mem_next(void* current);

// find a user given the fingerprint
void* fp_mem_find(fingerprint* fp);

fp_acl_db_status fp_mem_save_user(struct fp_acl_user* user);
fp_acl_db_status fp_mem_load_user(void* it, struct fp_acl_user* user);
fp_acl_db_status fp_mem_remove_user(void* it);
fp_acl_db_status fp_mem_load_settings(struct fp_acl_settings* settings);
fp_acl_db_status fp_mem_save_settings(struct fp_acl_settings* settings);

#endif
