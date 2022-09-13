#ifndef _FP_ACL_FILE_H_
#define _FP_ACL_FILE_H_

#include "fp_acl.h"
#include "fp_acl_memory.h"

#include <stdio.h>

/**
 * The fp acl file saves the fingerprint config in the following format
 *
 * saving rewrites the full file.
 */

/**
 * Version 1 format:
 *
 * uint32_t version == 1
 * settings { uint32_t systemPermissions, uint32_t defaultPermissions}
 * number of users: uint32_t,
 * [users] user = {fingerprint: uint8_t[16], name: char[64], permissions: uint32_t}
 *
 */

/**
 * Version 2 format:
 *
 * uint32_t version == 2
 * settings { uint32_t systemPermissions, uint32_t defaultUserPermissions, uint32_t firstUserPermissions}
 * number of users: uint32_t,
 * [users] user = {fingerprint: uint8_t[16], name: char[64], permissions: uint32_t}
 */

/**
 * Version 3 format:
 *
 * uint32_t version == 3
 * settings { uint32_t systemPermissions, uint32_t defaultUserPermissions, uint32_t firstUserPermissions}
 * number of users: uint32_t,
 * [users] user = {
 *    fingerprint: uint8_t[16]
 *    pskId: {
 *         hasValue: uint8_t,
 *         value: uint8_t[16]
 *    }
 *    psk: {
 *         hasValue: uint8_t,
 *         value: uint8_t[16]
 *    }
 *    name: char[64],
 *    permissions: uint32_t
 * }
 *
 */

/*
 * Version 4 of the file format is as follows:
 *
 * uint32_t version == 4
 * settings { uint32_t systemPermissions, uint32_t defaultUserPermissions, uint32_t firstUserPermissions}
 * number of users: uint32_t,
 * [users] user = {
 *    fingerprint: uint8_t[16]
 *    pskId: {
 *         hasValue: uint8_t,
 *         value: uint8_t[16]
 *    }
 *    psk: {
 *         hasValue: uint8_t,
 *         value: uint8_t[16]
 *    }
 *    fcmTok: char[256]
 *    name: char[64],
 *    permissions: uint32_t
 * }
 */

#define FP_ACL_FILE_USERNAME_LENGTH 64

#define FP_ACL_FILE_FCM_TOKEN_LENGTH 256

#define FP_ACL_FILE_VERSION 4

#define FP_ACL_RECORD_SIZE   FINGERPRINT_LENGTH +          \
                             1 + PSK_ID_LENGTH +           \
                             1 + PSK_LENGTH +              \
                             FP_ACL_USERNAME_MAX_LENGTH +  \
                             FP_ACL_FCM_TOKEN_MAX_LENGTH + \
                             sizeof(uint32_t)

#if FP_ACL_USERNAME_MAX_LENGTH != FP_ACL_FILE_USERNAME_LENGTH
#error incompatible user name length with current acl file format
#endif

#if FP_ACL_FCM_TOKEN_MAX_LENGTH != FP_ACL_FILE_FCM_TOKEN_LENGTH
#error incompatible FCM token length with current acl file format
#endif

fp_acl_db_status fp_acl_file_save_file(struct fp_mem_state* acl);

fp_acl_db_status fp_acl_file_load_file(struct fp_mem_state* acl);

/**
 * Inititalize the file persisting backend. tempFile is used to try to
 * save the state to it, if it succeedes tempFile will be renamed to
 * file.
 */
fp_acl_db_status fp_acl_file_init(const char* file, const char* tempFile, struct fp_mem_persistence* p);


#endif
