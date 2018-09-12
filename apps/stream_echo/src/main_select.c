#include <modules/cli/unabto_args.h>
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_app.h>

#include "select_impl.h"

int main(int argc, char** argv) {
    nabto_main_setup* nms = unabto_init_context();
    if (!check_args(argc, argv, nms)) {
        return 1;
    }
    return select_start(nms);
}

