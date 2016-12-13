#ifndef _FP_ACL_AE_H_
#define _FP_ACL_AE_H_

#include "fp_acl.h"

#include <unabto/unabto_app.h>

enum fp_acl_response_status {
    FP_ACL_STATUS_OK = 0,
    FP_ACL_STATUS_NO_SUCH_USER = 1,
    FP_ACL_STATUS_USER_DB_FULL = 2,
    FP_ACL_STATUS_SAVE_FAILED = 3,
    FP_ACL_STATUS_REMOVE_FAILED = 4
};

/**
 * Initialize the fingerprint acl module.
 */
void fp_acl_ae_init(struct fp_acl_db* db);

/**
 * call this function as part of every application request to check
 * that sufficient permissions is present for the application request.
 */
bool fp_acl_is_request_allowed(application_request* request, uint32_t requiredPermissions);

/**
 * call this function to see if the given application
 * requets/connection is allowed to pair with the device.
 */
bool fp_acl_is_pair_allowed(application_request* request);

/**
 * This function should be called from the allow_client_access(...)
 * function. This will reject connections early in the connect phase.
 */
bool fp_acl_is_connection_allowed(nabto_connect* connection);

// request getUsers.json?count=nn&start=nn
// response list of users, next
application_event_result fp_acl_ae_users_get(application_request* request,
                                             unabto_query_request* read_buffer,
                                             unabto_query_response* write_buffer);


// request getUser.json?fingerprint=<hex>
// response status, fingerprint, username, permissions
application_event_result fp_acl_ae_user_get(application_request* request,
                                            unabto_query_request* read_buffer,
                                            unabto_query_response* write_buffer);

// request getMe.json
// response status, fingerprint, username, permissions
application_event_result fp_acl_ae_user_me(application_request* request,
                                           unabto_query_request* read_buffer,
                                           unabto_query_response* write_buffer);

// request removeUser.json?fingerprint=<hex>
// response status
application_event_result fp_acl_ae_user_remove(application_request* request,
                                               unabto_query_request* read_buffer,
                                               unabto_query_response* write_buffer);

// request pairWithDevice.json?userName=<string>
// response status, fingerprint, username, permissions
application_event_result fp_acl_ae_pair_with_device(application_request* request,
                                                    unabto_query_request* read_buffer,
                                                    unabto_query_response* write_buffer);

// request setUserName.json?fingerprint=<hex>
// response status, fingerprint, username, permissions
application_event_result fp_acl_ae_user_set_name(application_request* request,
                                                 unabto_query_request* read_buffer,
                                                 unabto_query_response* write_buffer);

// request addPermissions.json?fingerprint=<hex>&permissions=<mask>
// response status, fingerprint, username, permissions
application_event_result fp_acl_ae_user_add_permissions(application_request* request,
                                                        unabto_query_request* read_buffer,
                                                        unabto_query_response* write_buffer);

// request removePermissions.json?fingerprint=<hex>&permissions=<mask>
// response status, fingerprint, username, permissions
application_event_result fp_acl_ae_user_remove_permissions(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer);


// request setPermissions.json?fingerprint=<hex>&permissions=<mask>
// response status, fingerprint, username, permissions
application_event_result fp_acl_ae_user_set_permissions(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer);


// request getAclSettings.json?
// response status, systemPermissions, defaultUserPermissions
application_event_result fp_acl_ae_system_get_acl_settings(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer);


// request getAclSettings.json?systemPermisions<uint32_t>&defaultUserPermissions=<uint32_t>
// response status, systemPermissions, defaultUserPermissions
application_event_result fp_acl_ae_system_set_acl_settings(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer);

#endif
