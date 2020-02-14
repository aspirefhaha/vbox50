/** @file
 * out of safe env filesystem service - Common header for host service and guest clients.
 */

/*
 * Copyright (C) 2011-2020
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

#ifndef ___VBox_HostService_OutEnvFSService_h
#define ___VBox_HostService_OutEnvFSService_h

#include <VBox/types.h>
#include <VBox/VMMDev.h>
#include <VBox/VBoxGuest2.h>
#include <VBox/hgcmsvc.h>
#include <VBox/log.h>
#include <iprt/assert.h>
#include <iprt/string.h>

/* Everything defined in this file lives in this namespace. */
//namespace outenvfs {
#define GDLSIM_FN_OPENDISK								(0x00000001)
#define GDLSIM_FN_CLOSEDISK							(0x00000002)
#define GDLSIM_FN_GETDISKINFO							(0x00000003)
#define GDLSIM_FN_GETDISKSN							(0x00000004)
#define GDLSIM_FN_WRITEHIDEDISK						(0x00000005)
#define GDLSIM_FN_READHIDEDISK						(0x00000006)
#define GDLSIM_FN_OPENCDROMDEVICE					(0x00000007)
#define GDLSIM_FN_CLOSESDISKDEVICE					(0x00000008)
#define GDLSIM_FN_VENDORREADFLASHDISK				(0x00000009)
#define GDLSIM_FN_VENDORREADFLASHDISKEX				(0x0000000a)
#define GDLSIM_FN_VENDORWRITEFLASHDISK				(0x0000000b)
#define GDLSIM_FN_VENDORWRITEFLASHDISKEX			(0x0000000c)
#define GDLSIM_FN_GETLOGICALDRIVES					(0x0000000d)
#define GDLSIM_FN_SETCURRENTDIRECTORY				(0x0000000e)
#define GDLSIM_FN_GETCURRENTDIRECTORY				(0x0000000f)
#define GDLSIM_FN_FINDFIRSTFILE						(0x00000010)
#define GDLSIM_FN_FINDNEXTFILE						(0x00000011)
#define GDLSIM_FN_FINDCLOSE							(0x00000012)
#define GDLSIM_FN_CREATEFILE							(0x00000013)
#define GDLSIM_FN_DELETEFILE							(0x00000014)
#define GDLSIM_FN_GETFILEINFORMATIONBYHANDLE		(0x00000015)
#define GDLSIM_FN_GETFILESIZE							(0x00000016)
#define GDLSIM_FN_GETFFILESIZEEX						(0x00000017)
#define GDLSIM_FN_GETTEMPPATH							(0x00000018)
#define GDLSIM_FN_MOVEFILE								(0x00000019)
#define GDLSIM_FN_COPYFILE								(0x0000001a)
#define GDLSIM_FN_SETFILETIME							(0x0000001b)
#define GDLSIM_FN_READFILE								(0x0000001c)
#define GDLSIM_FN_SETENDOFFILE						(0x0000001d)
#define GDLSIM_FN_SETFILEPOINTER						(0x0000001e)
#define GDLSIM_FN_WRITEFILE							(0x0000001f)
#define GDLSIM_FN_FLUSHFILEBUFFERS					(0x00000020)
#define GDLSIM_FN_CLOSEHANDLE							(0x00000021)
#define GDLSIM_FN_CREATEDIRECTORY						(0x00000022)
#define GDLSIM_FN_REMOVEDIRECTORY					(0x00000023)
#define GDLSIM_FN_GETLASTERROR						(0x00000024)
#define GDLSIM_FN_GETDISKFREESPACEEX					(0x00000025)
#define GDLSIM_FN_GETDRIVETYPE						(0x00000026)
#define GDLSIM_FN_GETUSERNAME						(0x00000027)
#define GDLSIM_FN_GETPASSWD							(0x00000028)
#define GDLSIM_FN_GETMACHINECODE						(0x00000029)
#define GDLSIM_FN_GETCURRENTMODULEDIR				(0x00000030)
#define GDLSIM_FN_PATHFILEEXISTS						(0x00000031)
#define GDLSIM_FN_GETDISKTYPE							(0x00000032)
#define GDLSIM_FN_SHGETFOLDERPATH					(0x00000033)
#define GDLSIM_FN_GETFILEATTRIBUTESEX					(0x00000034)

#define GDLSIM_CPARMS_OPENDISK						(3)
#define GDLSIM_CPARMS_CLOSEDISK						(2)
#define GDLSIM_CPARMS_GETDISKINFO					(4)
#define GDLSIM_CPARMS_GETDISKSN						(3)
#define GDLSIM_CPARMS_WRITEHIDEDISK					(5)
#define GDLSIM_CPARMS_READHIDEDISK					(5)
#define GDLSIM_CPARMS_OPENCDROMDEVICE				(4)
#define GDLSIM_CPARMS_CLOSESDISKDEVICE				(2)
#define GDLSIM_CPARMS_VENDORREADFLASHDISK			(6)
#define GDLSIM_CPARMS_VENDORREADFLASHDISKEX		(6)
#define GDLSIM_CPARMS_VENDORWRITEFLASHDISK			(6)
#define GDLSIM_CPARMS_VENDORWRITEFLASHDISKEX		(6)
#define GDLSIM_CPARMS_GETLOGICALDRIVES				(1)
#define GDLSIM_CPARMS_SETCURRENTDIRECTORY			(2)
#define GDLSIM_CPARMS_GETCURRENTDIRECTORY			(2)
#define GDLSIM_CPARMS_FINDFIRSTFILE					(3)
#define GDLSIM_CPARMS_FINDNEXTFILE					(3)
#define GDLSIM_CPARMS_FINDCLOSE						(2)
#define GDLSIM_CPARMS_CREATEFILE						(8)
#define GDLSIM_CPARMS_DELETEFILE						(2)
#define GDLSIM_CPARMS_GETFILEINFORMATIONBYHANDLE	(3)
#define GDLSIM_CPARMS_GETFILESIZE						(3)
#define GDLSIM_CPARMS_GETFFILESIZEEX					(3)
#define GDLSIM_CPARMS_GETTEMPPATH					(2)
#define GDLSIM_CPARMS_MOVEFILE						(3)
#define GDLSIM_CPARMS_COPYFILE						(4)
#define GDLSIM_CPARMS_SETFILETIME						(5)
#define GDLSIM_CPARMS_READFILE						(5)
#define GDLSIM_CPARMS_SETENDOFFILE					(2)
#define GDLSIM_CPARMS_SETFILEPOINTER					(5)
#define GDLSIM_CPARMS_WRITEFILE						(5)
#define GDLSIM_CPARMS_FLUSHFILEBUFFERS				(2)
#define GDLSIM_CPARMS_CLOSEHANDLE					(2)
#define GDLSIM_CPARMS_CREATEDIRECTORY				(3)
#define GDLSIM_CPARMS_REMOVEDIRECTORY				(2)
#define GDLSIM_CPARMS_GETLASTERROR					(1)
#define GDLSIM_CPARMS_GETDISKFREESPACEEX			(5)
#define GDLSIM_CPARMS_GETDRIVETYPE					(2)
#define GDLSIM_CPARMS_GETUSERNAME					(2)
#define GDLSIM_CPARMS_GETPASSWD						(2)
#define GDLSIM_CPARMS_GETMACHINECODE				(3)
#define GDLSIM_CPARMS_GETCURRENTMODULEDIR			(2)
#define GDLSIM_CPARMS_PATHFILEEXISTS					(2)
#define GDLSIM_CPARMS_GETDISKTYPE					(2)
#define GDLSIM_CPARMS_SHGETFOLDERPATH				(6)
#define GDLSIM_CPARMS_GETFILEATTRIBUTESEX				(4)

#define GDLSIM_FN_SETUSERNAME							1
#define GDLSIM_FN_SETPASSWD							2

#define GDLSIM_CPARMS_SETUSERNAME					1
#define GDLSIM_CPARMS_SETPASSWD						1
/******************************************************************************
* Typedefs, constants and inlines                                             *
******************************************************************************/

//#define HGCMSERVICE_NAME "OutEnvFSSvc"

//} /* namespace outenvfs */

#endif  /* !___VBox_HostService_OutEnvFSService_h */

