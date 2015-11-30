#ifndef _APPLICATION_CONFIGURATION_H_
#define _APPLICATION_CONFIGURATION_H_

#include <unabto/unabto.h>

typedef struct {
#if NABTO_ACL_ENABLE
    acl_user acl[NABTO_ACL_SIZE];
#endif
    uint8_t dummy; // the struct can not be empty so add a dummy field if needed.
} application_configuration;

#endif
