#ifndef _FP_ACL_H_
#define _FP_ACL_H_

#include <unabto_platform_types.h>

#define FP_LENGTH 16
#define USERNAME_MAX_LENGTH 64

typedef uint8_t fingerprint[FP_LENGTH];
typedef char username[USERNAME_MAX_LENGTH];

struct fp_acl_user {
    fingerprint fp;
    username un;
    uint32_t perm;
};

struct fp_acl_settings {
    bool openForPairing;
    uint32_t defaultPairingPermissions;
};

typedef enum {
    FP_ACL_DB_OK,
    FP_ACL_DB_FULL,
    FP_ACL_DB_FAILED
} fp_acl_db_status;

struct fp_acl_db {
    void* (*first)();
    void* (*next)(void* it);
    void* (*find)(fingerprint fp);
    // create or overwrite user
    fp_acl_db_status (*save)(struct fp_acl_user* user);
    fp_acl_db_status (*load)(void* it, struct fp_acl_user* user);
    // remove user
    fp_acl_db_status (*remove)(void* it);

    fp_acl_db_status (*load_settings)(struct fp_acl_settings* settings);
    fp_acl_db_status (*save_settings)(struct fp_acl_settings* settings);
};


void fp_acl_initialize(struct fp_acl_db* db);
/* fp_acl_status fp_acl_get_user(fingerprint* fp, struct fp_acl_user* user); */

/* fp_acl_status fp_acl_set_local_pairing_mode(bool enabled); */
/* fp_acl_status fp_acl_get_local_pairing_mode(bool* enabled); */
/* fp_acl_status fp_acl_set_local_pairing_permissions(uint32_t permissions); */
/* fp_acl_status fp_acl_get_local_pairing_permissions(uint32_t* permissions); */
/* // get the next user if fingerprint is empty then return the first user. */
/* fp_acl_status fp_acl_get_next_user(fingerprint* start, struct fp_acl_user* users); */
/* fp_acl_status fp_acl_remove_user(fingerprint* fp); */
/* // TODO */
/* fp_acl_status fp_acl_pair_with_device(fingerprint* fp, const char* name); */

/* fp_acl_status fp_acl_add_permissions(fingerprint* fp, uint32_t permissions); */
/* fp_acl_status fp_acl_remove_permissions(fingerprint* fp, uint32_t permissions); */
/* fp_acl_status fp_acl_set_user_name(fingerprint* fp, const char* name); */

#endif
