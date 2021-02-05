/*****************************************************************
 *      SPDX-License-Identifier: GPL-2.0-or-later
 *·     SPDX-FileCopyrightText: 1998 Caldera Inc.
 *·     SPDX-FileCopyrightText: Olaf Kirch <okir@caldera.de>
 *·     SPDX-FileCopyrightText: Christian Esken <esken@kde.org>
 *·     SPDX-FileCopyrightText: Oswald Buddenhagen <ossi@kde.org>
 *·     kcheckpass
 *
 *·     Simple password checker. Just invoke and send it
 *·     the password on stdin.
 *
 *·     If the password was accepted, the program exits with 0;
 *·     if it was rejected, it exits with 1. Any other exit
 *
 *      Other parts were taken from kscreensaver's passwd.cpp
 *****************************************************************/

#ifndef KCHECKPASS_ENUMS_H
#define KCHECKPASS_ENUMS_H

#ifdef __cplusplus
extern "C" {
#endif

/* these must match kcheckpass' exit codes */
typedef enum {
    AuthOk = 0,
    AuthBad = 1,
    AuthError = 2,
    AuthAbort = 3
} AuthReturn;

typedef enum {
    ConvGetBinary,
    ConvGetNormal,
    ConvGetHidden,
    ConvPutInfo,
    ConvPutError,
    ConvPutAuthSucceeded,
    ConvPutAuthFailed,
    ConvPutAuthError,
    ConvPutAuthAbort,
    ConvPutReadyForAuthentication
} ConvRequest;

/* these must match the defs in kgreeterplugin.h */
typedef enum {
    IsUser = 1, /* unused in kcheckpass */
    IsPassword = 2
} DataTag;

#ifdef __cplusplus
}
#endif

#endif /* KCHECKPASS_ENUMS_H */
