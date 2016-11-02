#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_test.h>

#include <modules/util/read_hex_test.h>

nabto_main_context nmc;

int main() {
    bool ret;
    NABTO_LOG_INFO(("Running the uNabto test suites"));

    ret = unabto_test_all();

    ret &= read_hex_test();
    
    exit(ret?0:1);
}
