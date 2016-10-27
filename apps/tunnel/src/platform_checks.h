#ifndef _TUNNEL_PLATFORM_CHECKS_H_
#define _TUNNEL_PLATFORM_CHECKS_H_

#include <unabto_platform_types.h>

bool platform_checks();

/**
 * return number of filedescriptors which can be made.
 * return -1 if the feature is not supported.
 */ 
int check_ulimit_files(int desired);

#endif
