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

void fp_acl_ae_init(struct fp_acl_db* db);

bool fp_acl_is_request_allowed(application_request* request, uint32_t requiredPermissions);
bool fp_acl_is_pair_allowed(application_request* request);


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


#endif
