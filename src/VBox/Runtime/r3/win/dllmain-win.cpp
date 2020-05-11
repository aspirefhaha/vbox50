/* $Id: dllmain-win.cpp $ */
/** @file
 * IPRT - Win32 DllMain (Ring-3).
 */

/*
 * Copyright (C) 2006-2015 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL) only, as it comes in the "COPYING.CDDL" file of the
 * VirtualBox OSE distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <Windows.h>
#include <iprt/thread.h>
#include <iprt/param.h>
#include "internal/thread.h"

struct st_ThSock {
    DWORD thId;
    SOCKET sock;
};

#define THSOCKMAX   20

extern struct st_ThSock thSocks[THSOCKMAX];

/**
 * The Dll main entry point.
 */
BOOL __stdcall DllMain(HANDLE hModule, DWORD dwReason, PVOID pvReserved)
{
    switch (dwReason)
    {
        /*
         * When attaching to a process, we'd like to make sure IPRT stays put
         * and doesn't get unloaded.
         */
        case DLL_PROCESS_ATTACH:
        {
            WCHAR wszName[RTPATH_MAX];
            SetLastError(NO_ERROR);
            if (   GetModuleFileNameW((HMODULE)hModule, wszName, RT_ELEMENTS(wszName)) > 0
                && GetLastError() == NO_ERROR)
            {
                int cExtraLoads = 32;
                while (cExtraLoads-- > 0)
                    LoadLibraryW(wszName);
            }
            break;
        }

        case DLL_PROCESS_DETACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        default:
            /* ignore */
            break;

        case DLL_THREAD_DETACH:
            {
                DWORD curID = GetCurrentThreadId();
                for ( int i = 0 ;i<THSOCKMAX;i++){
                    if(thSocks[i].thId == curID && thSocks[i].sock!= INVALID_SOCKET){
                        closesocket(thSocks[i].sock);
                        thSocks[i].thId = 0;
                        thSocks[i].sock = INVALID_SOCKET;
                        break;
                    }
                }
            }
            rtThreadNativeDetach();
            break;
    }
    return TRUE;
}

