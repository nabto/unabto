#include "fp_acl_ae_test.h"
#include <modules/fingerprint_acl/fp_acl_ae.h>

bool fp_acl_ae_test() {
    application_request req;
    req.connection = NULL;
    buffer_read_t read_buffer;
    buffer_write_t write_buffer;
    fp_acl_ae_user_get(&req,
                       &read_buffer,
                       &write_buffer);
    return true;
}
