#ifndef _FP_ACL_H_
#define _FP_ACL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_types.h>

#define FP_ACL_FP_LENGTH FINGERPRINT_LENGTH
#define FP_ACL_USERNAME_MAX_LENGTH 64
#define FP_ACL_PSK_ID_LENGTH 16
#define FP_ACL_PSK_KEY_LENGTH 16


// ACL permissions they are used on a per connection basis bit in the
// bits for general usage.
#define FP_ACL_PERMISSION_MASK_GENERAL                              0xffff0000ul
// Bits for custom application specific usage.
#define FP_ACL_PERMISSION_MASK_CUSTOM                               0x0000fffful

// Empty permission value.
#define FP_ACL_PERMISSION_NONE                                      0x00000000ul
// All permissions value.
#define FP_ACL_PERMISSION_ALL                                       0xfffffffful
// The LOCAL ACCESS bit gives a user access to the device locally.
#define FP_ACL_PERMISSION_LOCAL_ACCESS                              0x80000000ul
// The REMOTE ACCESS bit gives a user access to the device remotely
// via the internet.
#define FP_ACL_PERMISSION_REMOTE_ACCESS                             0x40000000ul
// The ADMIN bit gives a user administrative access to a device. This
// will normally atleast imply that the user can add, remove, change
// users and system permissions.
#define FP_ACL_PERMISSION_ADMIN                                     0x20000000ul

// SYSTEM permissions they are used to tell what the system can do
#define FP_ACL_SYSTEM_PERMISSION_NONE                               0x00000000ul
#define FP_ACL_SYSTEM_PERMISSION_ALL                                0xfffffffful
// The LOCAL ACCESS bit tells if the system is allowed to accept local
// connections.
#define FP_ACL_SYSTEM_PERMISSION_LOCAL_ACCESS                       0x80000000ul
// The REMOTE ACCESS bit tells if the system is allowed to accept
// remote connections.
#define FP_ACL_SYSTEM_PERMISSION_REMOTE_ACCESS                      0x40000000ul
// The PAIRING bit tells if the system is allowed to make pairings.
#define FP_ACL_SYSTEM_PERMISSION_PAIRING                            0x20000000ul

typedef char username[FP_ACL_USERNAME_MAX_LENGTH];

struct fp_acl_user {
    struct unabto_optional_fingerprint fp;
    struct unabto_optional_psk_id pskId;
    struct unabto_optional_psk psk;
#if NABTO_ENABLE_FCM_TOKEN_STORAGE
    struct unabto_optional_fcm_token fcmTok;
#endif
    username name;
    uint32_t permissions;
};

struct fp_acl_settings {
    uint32_t systemPermissions;      ///< permission bits controlling the system
    uint32_t defaultUserPermissions; ///< default permissions for new users
    uint32_t firstUserPermissions;   ///< permissions to give the first user of the system
};

typedef enum {
    FP_ACL_DB_OK,
    FP_ACL_DB_FULL,
    FP_ACL_DB_SAVE_FAILED,
    FP_ACL_DB_LOAD_FAILED,
    FP_ACL_DB_FAILED
} fp_acl_db_status;


/**
 * This is a structure of function pointer which describes the
 * interface to the underlying acl database. This interface has been
 * engineereed such that it in theory is possible to have an
 * underlying datasctructure where the users can e.g. be saved in
 * eeprom or ram.
 */
struct fp_acl_db {
    // Iterate through the users. First returns the first user
    // iterator.
    void* (*first)();
    // Get the next user iterator from the current User iterator. When
    // next returns NULL it's the last user.
    void* (*next)(void* it);
    // Get an iterator to a user with the given fingerprint. Return
    // NULL if the user does not exists.
    void* (*find)(const struct unabto_fingerprint* fp);
    // Create or overwrite user
    fp_acl_db_status (*save)(struct fp_acl_user* user);
    // Load a user given a valid iterator.
    fp_acl_db_status (*load)(void* it, struct fp_acl_user* user);
    // remove user
    fp_acl_db_status (*remove)(void* it);
    // remove all users in db
    fp_acl_db_status (*clear)();

    // Load system settings.
    fp_acl_db_status (*load_settings)(struct fp_acl_settings* settings);
    // Save system settings.
    fp_acl_db_status (*save_settings)(struct fp_acl_settings* settings);
};

void fp_acl_init_user(struct fp_acl_user* user);

// The following functions are helper functions and does not
// manipulate the acl database. They are just doing the simple bitwise
// operations correctly.

/**
 * Check if the system is allowed to do what the permission bits
 * requests. It could be asking if the system is allowed to accept
 * remote connections.
 */ 
bool fp_acl_check_system_permissions(struct fp_acl_settings* settings, uint32_t requiredPermissions);

/**
 * Given a non null user, check if that user has the required
 * permission bits set. isLocal refers to the type of the connection,
 * set it to true if the connection is local.
 */
bool fp_acl_check_user_permissions(struct fp_acl_user* user, bool isLocal, uint32_t requiredPermissions);

/**
 * Add permissions to a user.
 */
void fp_acl_user_add_permissions(struct fp_acl_user* user, uint32_t permissions);

/**
 * remove permission bits from a user
 */
void fp_acl_user_remove_permissions(struct fp_acl_user* user, uint32_t permissions);

/**
 * set permission bits on a user.
 */
void fp_acl_user_set_permissions(struct fp_acl_user* user, uint32_t permissions);

#endif
