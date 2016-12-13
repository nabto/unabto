#ifndef _FP_ACL_MEMORY_H_
#define _FP_ACL_MEMORY_H_

#include <unabto_platform_types.h>

#include "fp_acl.h"

#define FP_MEM_ACL_ENTRIES 32

struct fp_mem_state {
    struct fp_acl_settings settings;
    struct fp_acl_user users[FP_MEM_ACL_ENTRIES];    
};

struct fp_mem_persistence {
    fp_acl_db_status (*load)(struct fp_mem_state* state);
    fp_acl_db_status (*save)(struct fp_mem_state* state);
};

/**
 * In memory fingerprint acl database
 */

/**
 * constructor if persistence is NULL it will not be persisted.
 * defaultSettings is required and will take effect if the user database is empty.
 */
fp_acl_db_status fp_mem_init(struct fp_acl_db* db,
                             struct fp_acl_settings* defaultSettings,
                             struct fp_mem_persistence* persistence);

// iterator to iterate through all the users

// get iterator to first user
void* fp_mem_get_first_user();

// iterator++
void* fp_mem_next(void* current);

// find a user given the fingerprint
void* fp_mem_find(fingerprint fp);

fp_acl_db_status fp_mem_save_user(struct fp_acl_user* user);
fp_acl_db_status fp_mem_load_user(void* it, struct fp_acl_user* user);
fp_acl_db_status fp_mem_remove_user(void* it);
fp_acl_db_status fp_mem_clear();
fp_acl_db_status fp_mem_load_settings(struct fp_acl_settings* settings);
fp_acl_db_status fp_mem_save_settings(struct fp_acl_settings* settings);


bool fp_mem_is_slot_free(struct fp_acl_user* ix);

#endif
