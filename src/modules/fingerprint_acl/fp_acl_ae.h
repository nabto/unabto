#ifndef _FP_ACL_AE_H_
#define _FP_ACL_AE_H_

enum fp_acl_response_status {
    FP_ACL_STATUS_OK = 0,
    FP_ACL_STATUS_NO_SUCH_USER = 1,
    FP_ACL_STATUS_USER_DB_FULL = 2
};

// request getUser.json?fingerprint=<hex>
// response status, userName, fingerprint, permissions
application_event_result fp_acl_ae_get_user(pplication_request* request,
                                            buffer_read_t* read_buffer,
                                            buffer_write_t* write_buffer);

// request pairWithDevice.json?userName=<string>
// response status, userName, fingerprint, permissions
application_event_result fp_acl_ae_get_user(pplication_request* request,
                                            buffer_read_t* read_buffer,
                                            buffer_write_t* write_buffer);


#endif
