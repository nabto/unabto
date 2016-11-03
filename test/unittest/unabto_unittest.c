#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_test.h>

#include <modules/util/read_hex_test.h>

nabto_main_context nmc;

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

    if (ret) {
        NABTO_LOG_INFO(("All tests passed"));
        exit(0);
    } else {
        NABTO_LOG_INFO(("Some tests failed"));
        exit(1);
    }
}
