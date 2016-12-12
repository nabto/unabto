#ifndef _FP_ACL_H_
#define _FP_ACL_H_

#include <unabto_platform_types.h>

#define FP_ACL_FP_LENGTH 16
#define FP_ACL_USERNAME_MAX_LENGTH 64


// ACL permissions they are used on a per connection basis
#define FP_ACL_PERMISSION_NONE                                      0x00000000ul
#define FP_ACL_PERMISSION_ALL                                       0xfffffffful
#define FP_ACL_PERMISSION_LOCAL_ACCESS                              0x80000000ul
#define FP_ACL_PERMISSION_REMOTE_ACCESS                             0x40000000ul
#define FP_ACL_PERMISSION_ACCESS_CONTROL                            0x20000000ul

// SYSTEM permissions they are used to tell what the system can do
#define FP_ACL_SYSTEM_PERMISSION_NONE                               0x00000000ul
#define FP_ACL_SYSTEM_PERMISSION_ALL                                0xfffffffful
#define FP_ACL_SYSTEM_PERMISSION_LOCAL_ACCESS                       0x80000000ul
#define FP_ACL_SYSTEM_PERMISSION_REMOTE_ACCESS                      0x40000000ul
#define FP_ACL_SYSTEM_PERMISSION_PAIRING                            0x20000000ul


typedef uint8_t fingerprint[FP_ACL_FP_LENGTH];
typedef char username[FP_ACL_USERNAME_MAX_LENGTH];

struct fp_acl_user {
    fingerprint fp;
    username name;
    uint32_t permissions;
};

struct fp_acl_settings {
    uint32_t systemPermissions;   ///< permission bits controlling the system
    uint32_t defaultPermissions;  ///< default permissions for new users
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

// helper functions
bool fp_acl_check_system_permissions(struct fp_acl_settings* settings, uint32_t requiredPermissions);
bool fp_acl_check_user_permissions(struct fp_acl_user* user, bool isLocal, uint32_t requiredPermissions);

void fp_acl_user_add_permissions(struct fp_acl_user* user, uint32_t permissions);
void fp_acl_user_remove_permissions(struct fp_acl_user* user, uint32_t permissions);
void fp_acl_user_set_permissions(struct fp_acl_user* user, uint32_t permissions);

#endif
