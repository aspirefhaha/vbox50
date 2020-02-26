/* $Id */
/** @file
 * tstFhahaLib.cpp - Simple testcase for invoking FhahaLib.dll.
 */

/*
 * Copyright (C) 2012 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#define UNICODE
#include <windows.h>
#include <stdio.h>

int main(int argc, TCHAR* argv[])
{
    DWORD dwErr;

    /**
     * Be sure that:
     * - the debug FhahaLib gets loaded instead of a maybe installed
     *   release version in "C:\Windows\system32".
     */

    HMODULE hMod = LoadLibrary(L"FhahaLib.dll");
    if (!hMod)
    {
        dwErr = GetLastError();
        wprintf(L"FhahaLib.dll not found, error=%ld\n", dwErr);
    }
    else
    {
        wprintf(L"FhahaLib found\n");

        FARPROC pfnDebug = GetProcAddress(hMod, "FhahaLibDebug");
        if (!pfnDebug)
        {
            dwErr = GetLastError();
            wprintf(L"Could not load FhahaLibDebug, error=%ld\n", dwErr);
        }
        else
        {
            wprintf(L"Calling FhahaLib ...\n");
            dwErr = pfnDebug();
        }

        FreeLibrary(hMod);
    }

    wprintf(L"Test returned: %ld\n", dwErr);

    return dwErr == ERROR_SUCCESS ? 0 : 1;
}