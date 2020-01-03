/* $Id: VBoxGuestR3LibSwitch.cpp $ */
/** @file
 * VBoxGuestR3Lib - Ring-3 Support Library for VirtualBox guest additions, Switch.
 */

/*
 * Copyright (C) 2007-2015 Oracle Corporation
 *
 * 
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <iprt/time.h>
#include "VBGLR3Internal.h"

VBGLR3DECL(int) VbglR3Switch(void)
{
    VMMDevRequestHeader *ppReq;
    //vmmdevInitRequest(&Req.header, VMMDevReq_SafeEnvSwitch);
    vbglR3GRAlloc(&ppReq,sizeof(VMMDev_SafeEnvSwitchReq), VMMDevReq_SafeEnvSwitch);
    //Req.time = UINT64_MAX;
    int rc = vbglR3GRPerform(ppReq);
    vbglR3GRFree(ppReq);
       
    return rc;
}

