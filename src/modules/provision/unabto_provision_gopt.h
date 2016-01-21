#ifndef _UNABTO_PROVISION_GOPT_H
#define _UNABTO_PROVISION_GOPT_H

/**
 * Simplify using provisioning with cli/gopt module. See
 * weather_station demo for example of usage.
 */

#include <unabto/unabto_env_base.h>
#include "unabto_config.h"

#ifndef UNABTO_PROVISION_GOPT_START_ENUM
#define UNABTO_PROVISION_GOPT_START_ENUM     255
#endif

enum {
    UNABTO_PROVISION_GOPT_HOST = UNABTO_PROVISION_GOPT_START_ENUM,
    UNABTO_PROVISION_GOPT_API_KEY,
    UNABTO_PROVISION_GOPT_ID_INPUT,
    UNABTO_PROVISION_GOPT_USER_TOKEN,
    UNABTO_PROVISION_GOPT_FILE
};

#define UNABTO_PROVISION_GOPT_NAME_HOST        "provision-host"
#define UNABTO_PROVISION_GOPT_NAME_API_KEY     "provision-api-key"
#define UNABTO_PROVISION_GOPT_NAME_ID_INPUT    "provision-id-input"
#define UNABTO_PROVISION_GOPT_NAME_USER_TOKEN  "provision-user-token"
#define UNABTO_PROVISION_GOPT_NAME_FILE        "provision-file"

#define UNABTO_PROVISION_GOPT_ARGS() \
    { UNABTO_PROVISION_GOPT_HOST,      GOPT_ARG, "", gopt_longs(UNABTO_PROVISION_GOPT_NAME_HOST) } ,     \
    { UNABTO_PROVISION_GOPT_API_KEY,   GOPT_ARG, "", gopt_longs(UNABTO_PROVISION_GOPT_NAME_API_KEY) },   \
    { UNABTO_PROVISION_GOPT_ID_INPUT,  GOPT_ARG, "", gopt_longs(UNABTO_PROVISION_GOPT_NAME_ID_INPUT) },  \
    { UNABTO_PROVISION_GOPT_USER_TOKEN,GOPT_ARG, "", gopt_longs(UNABTO_PROVISION_GOPT_NAME_USER_TOKEN) },\
    { UNABTO_PROVISION_GOPT_FILE,      GOPT_ARG, "", gopt_longs(UNABTO_PROVISION_GOPT_NAME_FILE) },

void unabto_provision_gopt_help(const char* progname);

bool unabto_provision_gopt_apply(nabto_main_setup* nms, const char* progname, void* options);

#endif
