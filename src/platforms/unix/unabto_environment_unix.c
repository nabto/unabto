/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * The environment for the Nabto Micro Device Server (PC Device), Implementation.
 *
 */

#define NABTO_DECLARED_MODULE NABTO_LOG_NETWORK

#include "unabto/unabto_external_environment.h"
#include "unabto/unabto_logging.h"
#include "unabto/unabto_context.h"
#include "unabto/unabto_environment.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>


/**
 * Set a socket in non blocking mode.
 * @param sd  the socket
 */
void nabto_bsd_set_nonblocking(nabto_socket_t* sd)
{
    int flags = fcntl(sd->sock, F_GETFL, 0);
    if (flags == -1) flags = 0;
    fcntl(sd->sock, F_SETFL, flags | O_NONBLOCK);
}

bool nabto_init_platform() {
    return true;
}

void nabto_close_platform() {
}
