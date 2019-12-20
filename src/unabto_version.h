/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#ifndef _U_NABTO_VERSION_H_
#define _U_NABTO_VERSION_H_
/**
 * @file
 *
 * The uNabto Version Description holding the application version
 * numbers.
 *
 * There are two version numbers in uNabto. The one is the platform
 * version number that's the verison number described below. The other
 * version number is a string residing in the nabto_main_setup
 * struct. This string can be used for application version numbers
 * such that it is possible to describe the combined application
 * number of the unabto framework and the custom application to a
 * consuming client.
 */

/** 
 * Major, minor, patch release number (uint32_t), The version numbers
 * can be grepped out into bash with these commands, remove the extra
 * underscore which is inserted such that this file remains bash
 * friendly.
 *
 * grep U_NABTO_VERSION src/unabto_version.h | awk '{print $2 "=" $3}'
 * 
 * export `grep U_NABTO_VERSION src/unabto_version.h | awk '{print $2 "=" $3}'`
 */
#define UNABTO_VERSION_MAJOR            4
#define UNABTO_VERSION_MINOR            4
#define UNABTO_VERSION_PATCH            4

/**
 * Prerelease identifiers are used to distinguish real releases from
 * development, rc and beta releases. Set this string to "" for a
 * final release. Remember to include the first dash.
 */
#define UNABTO_VERSION_PRERELEASE ""

/**
 * Build info is used to distinguish builds, it could for example be
 * the jenkins build id. Remember to include a plus (+) in start of the
 * build version.
 */
#define UNABTO_VERSION_BUILD ""

#endif
