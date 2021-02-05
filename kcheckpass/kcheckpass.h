/*****************************************************************
 *      SPDX-License-Identifier: GPL-2.0-or-later
 *·     SPDX-FileCopyrightText: 1998 Caldera Inc.
 *·     SPDX-FileCopyrightText: Olaf Kirch <okir@caldera.de>
 *·     SPDX-FileCopyrightText: Christian Esken <esken@kde.org>
 *·     SPDX-FileCopyrightText: Oswald Buddenhagen <ossi@kde.org>
 *
 *	kcheckpass
 *
 *	Simple password checker. Just invoke and send it
 *	the password on stdin.
 *
 *	If the password was accepted, the program exits with 0;
 *	if it was rejected, it exits with 1. Any other exit
 *	code signals an error.
 *
 *      Other parts were taken from kscreensaver's passwd.cpp
 *****************************************************************/

#ifndef KCHECKPASS_H_
#define KCHECKPASS_H_

#include <config-workspace.h>
#include <config-unix.h>
#include <config-kcheckpass.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#include <pwd.h>
#include <sys/types.h>

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif


#ifdef ultrix
#include <auth.h>
#endif

#include <unistd.h>

/* Make sure there is only one! */
#if defined(HAVE_PAM)
#else
#define HAVE_SHADOW
#endif

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
# define ATTR_UNUSED __attribute__((unused))
# define ATTR_NORETURN __attribute__((noreturn))
# define ATTR_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
# define ATTR_UNUSED
# define ATTR_NORETURN
# define ATTR_PRINTFLIKE(fmt,var)
#endif

#include "kcheckpass-enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************
 * Authenticates user
 *****************************************************************/
AuthReturn Authenticate(
        const char *method,
        const char *user,
        char *(*conv) (ConvRequest, const char *));

/*****************************************************************
 * Output a message to stderr
 *****************************************************************/
void message(const char *, ...) ATTR_PRINTFLIKE(1, 2);

/*****************************************************************
 * Overwrite and free the passed string
 *****************************************************************/
void dispose(char *);

#ifdef __cplusplus
}
#endif
#endif
