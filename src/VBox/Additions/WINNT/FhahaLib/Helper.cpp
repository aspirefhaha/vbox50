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

#include "Helper.h"
#include "FhahaLib.h"

#include <VBox/log.h>
#include <VBox/VBoxGuestLib.h>


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

    /* Do not report back an error here yet. */
    return VINF_SUCCESS;
}


