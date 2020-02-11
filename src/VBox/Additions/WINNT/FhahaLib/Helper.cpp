/* $Id: Helper.cpp $ */
/** @file
 * FhahaLib - Windows Logon DLL for VirtualBox, Helper Functions.
 */

/*
 * Copyright (C) 2006-2012 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#include <windows.h>

#include <iprt/semaphore.h>
#include <iprt/string.h>
#include <iprt/thread.h>

#include "winwlx.h"
#include "Helper.h"
#include "FhahaLib.h"

#include <VBox/log.h>
#include <VBox/VBoxGuestLib.h>

/** Flag indicating whether remote sessions (over MSRDP) should be
 *  handled or not. Default is disabled. */
static DWORD g_dwHandleRemoteSessions = 0;
/** Verbosity flag for guest logging. */
static DWORD g_dwVerbosity = 0;

/**
 * Displays a verbose message.
 *
 * @param   iLevel      Minimum log level required to display this message.
 * @param   pszFormat   The message text.
 * @param   ...         Format arguments.
 */
void FhahaLibVerbose(DWORD dwLevel, const char *pszFormat, ...)
{
    if (dwLevel <= g_dwVerbosity)
    {
        va_list args;
        va_start(args, pszFormat);
        char *psz = NULL;
        RTStrAPrintfV(&psz, pszFormat, args);
        va_end(args);

        AssertPtr(psz);
        LogRel(("%s", psz));

        RTStrFree(psz);
    }
}

/**
 * Loads the global configuration from registry.
 *
 * @return  IPRT status code.
 */
int FhahaLibLoadConfiguration(void)
{
    HKEY hKey;
    /** @todo Add some registry wrapper function(s) as soon as we got more values to retrieve. */
    DWORD dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Oracle\\VirtualBox Guest Additions\\AutoLogon",
                               0L, KEY_QUERY_VALUE, &hKey);
    if (dwRet == ERROR_SUCCESS)
    {
        DWORD dwValue;
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);

        dwRet = RegQueryValueEx(hKey, L"HandleRemoteSessions", NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
        if (   dwRet  == ERROR_SUCCESS
            && dwType == REG_DWORD
            && dwSize == sizeof(DWORD))
        {
            g_dwHandleRemoteSessions = dwValue;
        }

        dwRet = RegQueryValueEx(hKey, L"LoggingEnabled", NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
        if (   dwRet  == ERROR_SUCCESS
            && dwType == REG_DWORD
            && dwSize == sizeof(DWORD))
        {
            g_dwVerbosity = 1; /* Default logging level. */
        }

        if (g_dwVerbosity) /* Do we want logging at all? */
        {
            dwRet = RegQueryValueEx(hKey, L"LoggingLevel", NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
            if (   dwRet  == ERROR_SUCCESS
                && dwType == REG_DWORD
                && dwSize == sizeof(DWORD))
            {
                g_dwVerbosity = dwValue;
            }
        }

        RegCloseKey(hKey);
    }
    /* Do not report back an error here yet. */
    return VINF_SUCCESS;
}

/**
 * Determines whether we should handle the current session or not.
 *
 * @return  bool        true if we should handle this session, false if not.
 */
bool FhahaLibHandleCurrentSession(void)
{
    /* Load global configuration from registry. */
    int rc = FhahaLibLoadConfiguration();
    if (RT_FAILURE(rc))
        FhahaLibVerbose(0, "FhahaLib::handleCurrentSession: Error loading global configuration, rc=%Rrc\n",
                        rc);

    bool fHandle = false;
    if (VbglR3AutoLogonIsRemoteSession())
    {
        if (g_dwHandleRemoteSessions) /* Force remote session handling. */
            fHandle = true;
    }
    else /* No remote session. */
        fHandle = true;

#ifdef DEBUG
    FhahaLibVerbose(3, "FhahaLib::handleCurrentSession: Handling current session=%RTbool\n", fHandle);
#endif
    return fHandle;
}

/* handle of the poller thread */
RTTHREAD gThreadPoller = NIL_RTTHREAD;

/**
 * Poller thread. Checks periodically whether there are credentials.
 */
static DECLCALLBACK(int) credentialsPoller(RTTHREAD ThreadSelf, void *pvUser)
{
    FhahaLibVerbose(0, "FhahaLib::credentialsPoller\n");

    do
    {
        int rc = VbglR3CredentialsQueryAvailability();
        if (RT_SUCCESS(rc))
        {
            FhahaLibVerbose(0, "FhahaLib::credentialsPoller: got credentials, simulating C-A-D\n");
            /* tell WinLogon to start the attestation process */
            pWlxFuncs->WlxSasNotify(hGinaWlx, WLX_SAS_TYPE_CTRL_ALT_DEL);
            /* time to say goodbye */
            return 0;
        }

        if (   RT_FAILURE(rc)
            && rc != VERR_NOT_FOUND)
        {
            static int s_cBitchedQueryAvail = 0;
            if (s_cBitchedQueryAvail++ < 5)
                FhahaLibVerbose(0, "FhahaLib::credentialsPoller: querying for credentials failed with rc=%Rrc\n", rc);
        }

        /* wait a bit */
        if (RTThreadUserWait(ThreadSelf, 500) == VINF_SUCCESS)
        {
            FhahaLibVerbose(0, "FhahaLib::credentialsPoller: we were asked to terminate\n");
            /* we were asked to terminate, do that instantly! */
            return 0;
        }
    }
    while (1);

    return 0;
}

int FhahaLibCredentialsPollerCreate(void)
{
    if (!FhahaLibHandleCurrentSession())
        return VINF_SUCCESS;

    FhahaLibVerbose(0, "FhahaLib::credentialsPollerCreate\n");

    /* don't create more than one of them */
    if (gThreadPoller != NIL_RTTHREAD)
    {
        FhahaLibVerbose(0, "FhahaLib::credentialsPollerCreate: thread already running, returning!\n");
        return VINF_SUCCESS;
    }

    /* create the poller thread */
    int rc = RTThreadCreate(&gThreadPoller, credentialsPoller, NULL, 0, RTTHREADTYPE_INFREQUENT_POLLER,
                            RTTHREADFLAGS_WAITABLE, "creds");
    if (RT_FAILURE(rc))
        FhahaLibVerbose(0, "FhahaLib::credentialsPollerCreate: failed to create thread, rc = %Rrc\n", rc);

    return rc;
}

int FhahaLibCredentialsPollerTerminate(void)
{
    if (gThreadPoller == NIL_RTTHREAD)
        return VINF_SUCCESS;

    FhahaLibVerbose(0, "FhahaLib::credentialsPollerTerminate\n");

    /* post termination event semaphore */
    int rc = RTThreadUserSignal(gThreadPoller);
    if (RT_SUCCESS(rc))
    {
        FhahaLibVerbose(0, "FhahaLib::credentialsPollerTerminate: waiting for thread to terminate\n");
        rc = RTThreadWait(gThreadPoller, RT_INDEFINITE_WAIT, NULL);
    }
    else
        FhahaLibVerbose(0, "FhahaLib::credentialsPollerTerminate: thread has terminated? wait rc = %Rrc\n",     rc);

    if (RT_SUCCESS(rc))
    {
        gThreadPoller = NIL_RTTHREAD;
    }

    FhahaLibVerbose(0, "FhahaLib::credentialsPollerTerminate: returned with rc=%Rrc)\n", rc);
    return rc;
}

/**
 * Reports FhahaLib's status to the host (treated as a guest facility).
 *
 * @return  IPRT status code.
 * @param   enmStatus               Status to report to the host.
 */
int FhahaLibReportStatus(VBoxGuestFacilityStatus enmStatus)
{
    FhahaLibVerbose(0, "FhahaLib: reporting status %d\n", enmStatus);

    int rc = VbglR3AutoLogonReportStatus(enmStatus);
    if (RT_FAILURE(rc))
        FhahaLibVerbose(0, "FhahaLib: failed to report status %d, rc=%Rrc\n", enmStatus, rc);
    return rc;
}

