/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * The argument parsing for pc and boost versions of unabto
 */
 
#ifndef _UNABTO_ARGS_H_
#define _UNABTO_ARGS_H_

#include "unabto/unabto_env_base.h"
#include "unabto/unabto_main_contexts.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check and convert arguments.
 * @param argc         the number of arguments
 * @param argv         the arguments
 * @param nmc          the main context
 * @return             true if successfull
 */
bool check_args(int argc, char* argv[], nabto_main_setup *nms);

/**
 * Try to convert the hostname given in nmc->id
 * to a ipv4 controller address
 * @param nms the nabto main setup to get device id and set hostname in.
 * @return true if resolution succeded
 */
bool resolve_hostname(nabto_main_setup *nms);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
