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

/** Major, minor, patch release number (uint32_t) */
#define RELEASE_MAJOR            3
#define RELEASE_MINOR            0
#define RELEASE_PATCH            16

/**
 * Labels are used to distinguish real releases from development, rc
 * and beta releases. Set this string to "" for a final release.
 */
#define RELEASE_LABEL "pre.0"

#ifndef PRI_RELEASE_LABEL
#define PRI_RELEASE_LABEL "%s%s"
#endif

#ifndef PRI_RELEASE
#define PRI_RELEASE "%" PRIu32 ".%" PRIu32 ".%" PRIu32 PRI_RELEASE_LABEL
#endif

#ifndef PRI_RELEASE_LABEL_FORMAT
// only print a - if the length of the label is not empty
#define PRI_RELEASE_LABEL_FORMAT (strlen(RELEASE_LABEL) > 0 ? "-" : ""), (strlen(RELEASE_LABEL) > 0 ? RELEASE_LABEL : "")
#endif

#ifndef PRI_RELEASE_FORMAT
#define PRI_RELEASE_FORMAT (uint32_t)RELEASE_MAJOR, (uint32_t)RELEASE_MINOR, (uint32_t)RELEASE_PATCH, PRI_RELEASE_LABEL_FORMAT
#endif

#endif
