#include "unabto/unabto_env_base.h"
#include "unabto/unabto_test.h"

#include "unabto/unabto_memory.h"

nabto_main_context nmc;

int main(int argc, char** argv) {
    bool ret;
    NABTO_LOG_INFO(("Running the uNabto test suites"));

    ret = unabto_test_all();
    exit(ret?0:1);
}
