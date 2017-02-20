/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#ifndef _UNABTO_VERSION_H_
#define _UNABTO_VERSION_H_
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
 * Major, minor, patch release number (uint32_t),
 * Labels are used to distinguish real releases from development, rc
 * and beta releases. Set this string to "" for a final release.
 */
#define UNABTO_VERSION_MAJOR            3
#define UNABTO_VERSION_MINOR            0
#define UNABTO_VERSION_PATCH            16

/**
 * Labels are used to distinguish real releases from development, rc
 * and beta releases. Set this string to "" for a final
 * release. Remember to include the first dash.
 */
#define UNABTO_VERSION_PRERELEASE "-alpha.0"

/**
 * Build info is used to distinguish builds, it could for example be
 * the jenkins build id. Remember to include a plus in start of the
 * build version.
 */
#define UNABTO_VERSION_BUILD ""


/**
 * use the following helper macros as
 * printf("Version " PRIversion "\n", MAKE_VERSION_PRINTABLE());
 */

#ifndef PRIversion
#define PRIversion "%" PRIu32 ".%" PRIu32 ".%" PRIu32 "%s%s"
#endif

#ifndef MAKE_VERSION_PRINTABLE
#define MAKE_VERSION_PRINTABLE() (uint32_t)UNABTO_VERSION_MAJOR, (uint32_t)UNABTO_VERSION_MINOR, (uint32_t)UNABTO_VERSION_PATCH, UNABTO_VERSION_PRERELEASE, UNABTO_VERSION_BUILD
#endif

#endif
