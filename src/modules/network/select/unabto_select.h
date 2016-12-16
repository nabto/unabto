#ifndef _UNABTO_SELECT_H_
#define _UNABTO_SELECT_H_

#if defined(WIN32) || defined(WINCE)
#include "Winsock2.h"
#else
#include <sys/select.h>
#endif

#endif
