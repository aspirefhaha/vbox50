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

#ifndef ___H_FHAHALIBHELPER
#define ___H_FHAHALIBHELPER

#include <VBox/VBoxGuestLib.h>

void FhahaLibVerbose(DWORD dwLevel, const char *pszFormat, ...);
int  FhahaLibLoadConfiguration();

#endif /* !___H_FHAHALIBHELPER */

