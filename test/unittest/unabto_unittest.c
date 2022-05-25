#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_test.h>

#include <modules/util/read_hex_test.h>
#include <modules/fingerprint_acl/fp_acl_mem_test.h>
#include <modules/fingerprint_acl/fp_acl_ae_test.h>

#include <unabto/unabto_payload_test.h>

int main() {
    bool ret = true;
    bool r;
    NABTO_LOG_INFO(("Running the uNabto test suites"));

    r = unabto_test_all();
    if (!r) {
        NABTO_LOG_ERROR(("Test all failed"));
        ret = false;
    }

    r = read_hex_test();
    if (!r) {
        NABTO_LOG_ERROR(("Test of hex function failed"));
        ret = false;
    }

    r = test_read_payload();
    if (!r) {
        NABTO_LOG_ERROR(("test read payload failed"));
        ret = false;
    }

    r = fp_acl_mem_test();
    if (!r) {
        NABTO_LOG_ERROR(("test of acl memory state failed"));
        ret = false;
    }

    r = fp_acl_ae_test();
    if (!r) {
        NABTO_LOG_ERROR(("test of acl failed"));
        ret = false;
    }

    if (ret) {
        NABTO_LOG_INFO(("All tests passed"));
        exit(0);
    } else {
        NABTO_LOG_INFO(("Some tests failed"));
        exit(1);
    }


}
