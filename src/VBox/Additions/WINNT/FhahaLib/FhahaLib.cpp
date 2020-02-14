/** @file
 *
 * FhahaLib -- Windows Logon DLL for VirtualBox
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <iprt/buildconfig.h>
#include <iprt/initterm.h>
#include <iprt/ldr.h>

#include <VBox/VBoxGuestLib.h>
#include <VBox/VBoxGuest2.h>
#include <VBoxGuestLib/VBGLR3Internal.h>
#include <VBox/HostServices/OutEnvFSSVC.h>

#include "FhahaLib.h"
#include "Helper.h"
uint32_t u32ClientId=0;
/*
 * Global variables.
 */

/** DLL instance handle. */
HINSTANCE hDllInstance;

#pragma pack(1)


typedef struct _TJSimGetLogicalDrives
{

    VBoxGuestHGCMCallInfo hdr;

    HGCMFunctionParameter ret;   /* OUT DWORD */

} TJSimGetLogicalDrives;
static DWORD SimGetLogicalDrives()
{
    TJSimGetLogicalDrives Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETLOGICALDRIVES;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETLOGICALDRIVES;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        DWORD ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}

typedef struct _TJSimSetCurrentDirectory
{
    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter dir;  /*  IN LPCTSTR */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimSetCurrentDirectory;
static BOOL SimSetCurrentDirectory(LPCTSTR lpPathName)
{
    TJSimSetCurrentDirectory Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_SETCURRENTDIRECTORY;
    Msg.hdr.cParms = GDLSIM_CPARMS_SETCURRENTDIRECTORY;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    char tmpstr[1024] = {0,};
    if(lpPathName && strlen(lpPathName)){
        strcpy(tmpstr,lpPathName);
        VbglHGCMParmPtrSet(&Msg.dir, (void *)tmpstr, strlen((const char *)tmpstr)+1);
    }
    else
        VbglHGCMParmPtrSet(&Msg.dir, NULL, 0);
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimGetCurrentDirectory
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pBuffer;  /*  IN LPTSTR */
    HGCMFunctionParameter ret;   /* OUT DWORD */
} TJSimGetCurrentDirectory;
static DWORD SimGetCurrentDirectory(DWORD nBufferLength, LPTSTR lpBuffer)
{
    TJSimGetCurrentDirectory Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETCURRENTDIRECTORY;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETCURRENTDIRECTORY;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmPtrSet(&Msg.pBuffer, (void *)lpBuffer, nBufferLength);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        DWORD ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimGetFileAttributesEx
{
    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pFileName;  /*  IN LPTSTR */
    HGCMFunctionParameter fInfoLevelId;  /*  uint32 */
    HGCMFunctionParameter lpFileInformation; /* IN OUT LPWIN32_FIND_DATA */
    HGCMFunctionParameter ret;   /* OUT DWORD BOOL */
}TJSimGetFileAttributesEx;
static BOOL SimGetFileAttributesEx(LPCSTR  lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID  lpFileInformation)
{
    TJSimGetFileAttributesEx Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETFILEATTRIBUTESEX;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETFILEATTRIBUTESEX;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    char tmpstr[1024] = {0,};
    if(lpFileName && strlen(lpFileName)){
        strcpy(tmpstr,lpFileName);
        VbglHGCMParmPtrSet(&Msg.pFileName, (void *)tmpstr, strlen(tmpstr) + 1);
    }
    else
        VbglHGCMParmPtrSet(&Msg.pFileName, NULL, 0);
    VbglHGCMParmUInt32Set(&Msg.fInfoLevelId,(uint32_t)fInfoLevelId);
    VbglHGCMParmPtrSet(&Msg.lpFileInformation, (void *)lpFileInformation, sizeof(WIN32_FILE_ATTRIBUTE_DATA));    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return NULL;
}

typedef struct _TJSimFindFirstFile
{
    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pFileName;  /*  IN LPTSTR */
    HGCMFunctionParameter pFileData; /* IN OUT LPWIN32_FIND_DATA */
    HGCMFunctionParameter ret;   /* OUT uint64 for 64bit ptr as handle */

} TJSimFindFirstFile;
static long long SimFindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
    TJSimFindFirstFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_FINDFIRSTFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_FINDFIRSTFILE;
    VbglHGCMParmUInt64Set(&Msg.ret, (uint64_t)0);
    char tmpstr[1024] = {0,};
    if(lpFileName && strlen(lpFileName)){
        strcpy(tmpstr,lpFileName);
        VbglHGCMParmPtrSet(&Msg.pFileName, (void *)tmpstr, strlen(tmpstr) + 1);
    }
    else
        VbglHGCMParmPtrSet(&Msg.pFileName, NULL, 0);
    VbglHGCMParmPtrSet(&Msg.pFileData, (void *)lpFindFileData, sizeof(WIN32_FIND_DATA));    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        long long ret;
        rc = VbglHGCMParmUInt64Get(&Msg.ret, (uint64_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return NULL;
}

typedef struct _TJSimFindNextFile
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFindFile;  /*  IN long long => DWORD64 */
    HGCMFunctionParameter pFileData; /* IN OUT LPWIN32_FIND_DATA */
    HGCMFunctionParameter ret;   /* OUT BOOL */

} TJSimFindNextFile;
static BOOL SimFindNextFile(long long hFindFile, LPWIN32_FIND_DATA lpFindFileData)
{
    TJSimFindNextFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_FINDNEXTFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_FINDNEXTFILE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFindFile, (uint64_t)hFindFile);
    VbglHGCMParmPtrSet(&Msg.pFileData, (void *)lpFindFileData, sizeof(WIN32_FIND_DATA));    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimFindClose
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFindFile;  /*  IN long long => DWORD64 */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimFindClose;
static BOOL SimFindClose(long long hFindFile)
{
    TJSimFindClose Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_FINDCLOSE;
    Msg.hdr.cParms = GDLSIM_CPARMS_FINDCLOSE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFindFile, (uint64_t)hFindFile);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimCreateFile
{
    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pFileName;  /*  IN LPCTSTR */
    HGCMFunctionParameter dwDesireAccess;  /*  IN DWORD */
    HGCMFunctionParameter dwShareMode;  /*  IN DWORD */
    HGCMFunctionParameter lpSecurityAttributes;  /*  IN LPSECURITY_ATTRIBUTES */
    HGCMFunctionParameter dwCreationDisposition;  /*  IN DWORD */
    HGCMFunctionParameter dwFlagsAndAttributes; /* IN DWORD  */
    HGCMFunctionParameter hTemplateFile; /* IN long long */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimCreateFile;
static long long SimCreateFile(LPCTSTR lpFileName,DWORD dwDesiredAccess, DWORD dwShareMode, 
        LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, long long hTemplateFile)
{
    TJSimCreateFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_CREATEFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_CREATEFILE;
    VbglHGCMParmUInt64Set(&Msg.ret, (uint64_t)0);
    char tmpstr[1024] = {0,};
    if(lpFileName && strlen(lpFileName)){
        strcpy(tmpstr,lpFileName);
        VbglHGCMParmPtrSet(&Msg.pFileName, (void *)tmpstr, strlen(tmpstr) + 1);
    }
    else
        VbglHGCMParmPtrSet(&Msg.pFileName, NULL, 0);
   
    VbglHGCMParmUInt32Set(&Msg.dwDesireAccess, (uint32_t)dwDesiredAccess);
    VbglHGCMParmUInt32Set(&Msg.dwShareMode, (uint32_t)dwShareMode);
    if(lpSecurityAttributes){
        VbglHGCMParmPtrSet(&Msg.lpSecurityAttributes, (void *)lpSecurityAttributes,sizeof(SECURITY_ATTRIBUTES));
    }
    else
        VbglHGCMParmPtrSet(&Msg.lpSecurityAttributes, (void *)NULL,0);
    VbglHGCMParmUInt32Set(&Msg.dwCreationDisposition, (uint32_t)dwCreationDisposition);
    VbglHGCMParmUInt32Set(&Msg.dwFlagsAndAttributes, (uint32_t)dwFlagsAndAttributes);
    VbglHGCMParmUInt32Set(&Msg.hTemplateFile, (uint32_t)hTemplateFile);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        long long ret;
        rc = VbglHGCMParmUInt64Get(&Msg.ret, (uint64_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return NULL;
}


typedef struct _TJSimDeleteFile
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pFileName;  /*  IN LPCTSTR  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimDeleteFile;
static BOOL SimDeleteFile(LPCTSTR lpFileName)
{
    TJSimDeleteFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_DELETEFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_DELETEFILE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    char tmpstr[1024] = {0,};
    if(lpFileName && strlen(lpFileName)){
        strcpy(tmpstr,lpFileName);
        VbglHGCMParmPtrSet(&Msg.pFileName, (void *)tmpstr, strlen(tmpstr) + 1);
    }
    else
        VbglHGCMParmPtrSet(&Msg.pFileName, NULL, 0);
   
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimGetFileInformationByHandle
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN long long  */
    HGCMFunctionParameter pFileInformation;  /*  IN LPBY_HANDLE_FILE_INFORMATION  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJJSimGetFileInformationByHandle;
static BOOL SimGetFileInformationByHandle(long long hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation)
{
    TJJSimGetFileInformationByHandle Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETFILEINFORMATIONBYHANDLE;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETFILEINFORMATIONBYHANDLE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFile, (uint64_t)hFile);
    if(lpFileInformation){
        VbglHGCMParmPtrSet(&Msg.pFileInformation, (void *)lpFileInformation,sizeof(PBY_HANDLE_FILE_INFORMATION));
    }
    else
        VbglHGCMParmPtrSet(&Msg.pFileInformation, (void *)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimGetFileSize
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN LPCTSTR  */
    HGCMFunctionParameter pFileSizeHigh;  /*  IN LPCTSTR  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimGetFileSize;
static DWORD SimGetFileSize(long long hFile, LPDWORD lpFileSizeHigh)
{
    TJSimGetFileSize Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETFILESIZE;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETFILESIZE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmPtrSet(&Msg.pFileSizeHigh, (void *)lpFileSizeHigh,lpFileSizeHigh?sizeof(DWORD):0);
    VbglHGCMParmUInt64Set(&Msg.hFile, (uint64_t)hFile);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        DWORD ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}

typedef struct _TJSimGetFileSizeEx
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN LPCTSTR  */
    HGCMFunctionParameter pFileSize;  /*  IN LPCTSTR  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimGetFileSizeEx;
static BOOL SimGetFileSizeEx(long long hFile, PLARGE_INTEGER lpFileSize)
{
    TJSimGetFileSizeEx Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETFFILESIZEEX;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETFFILESIZEEX;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmPtrSet(&Msg.pFileSize, (void *)lpFileSize,sizeof(LARGE_INTEGER));
    VbglHGCMParmUInt32Set(&Msg.hFile, (uint32_t)hFile);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimGetTempPath
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pBuffer;  /*  IN LPCTSTR  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimGetTempPath;
static DWORD SimGetTempPath(DWORD nBufferLength, LPTSTR lpBuffer)
{
    TJSimGetTempPath Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETTEMPPATH;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETTEMPPATH;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmPtrSet(&Msg.pBuffer, lpBuffer,nBufferLength);
   
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        DWORD ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}

typedef struct _TJSimMoveFile
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pExistingFileName;  /*  IN LPCTSTR  */
    HGCMFunctionParameter pNewFileName; /* IN LPCTSTR */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimMoveFile;
static BOOL SimMoveFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName)
{
    TJSimMoveFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_MOVEFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_MOVEFILE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    char tmpstr1[1024] = {0,};
    char tmpstr2[1024] = {0,};   
    if(lpExistingFileName){
        strcpy(tmpstr1,lpExistingFileName);
        VbglHGCMParmPtrSet(&Msg.pExistingFileName,(void *) tmpstr1,strlen(tmpstr1)+1);
    }
    else
        VbglHGCMParmPtrSet(&Msg.pExistingFileName, (void *)0,0);
    if(lpNewFileName){
        strcpy(tmpstr2,lpNewFileName);
        VbglHGCMParmPtrSet(&Msg.pNewFileName, (void *)tmpstr2,strlen(tmpstr2)+1);
    }
    else
        VbglHGCMParmPtrSet(&Msg.pNewFileName, (void *)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimCopyFile
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pExistingFileName;  /*  IN LPCTSTR  */
    HGCMFunctionParameter pNewFileName; /* IN LPCTSTR */
    HGCMFunctionParameter bFailIfExists; /* IN BOOL */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimCopyFile;
static BOOL SimCopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists)
{
    TJSimCopyFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_COPYFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_COPYFILE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    char tmpstr1[1024] = {0,};
    char tmpstr2[1024] = {0,};   
    VbglHGCMParmUInt32Set(&Msg.bFailIfExists, (uint32_t)bFailIfExists);
    if(lpExistingFileName){
        strcpy(tmpstr1,lpExistingFileName);
        VbglHGCMParmPtrSet(&Msg.pExistingFileName, (void *)tmpstr1,strlen(tmpstr1)+1);
    }
   else
        VbglHGCMParmPtrSet(&Msg.pExistingFileName, (void *)0,0);
   if(lpNewFileName){
        strcpy(tmpstr2,lpNewFileName);
        VbglHGCMParmPtrSet(&Msg.pNewFileName, (void *)tmpstr2,strlen(tmpstr2)+1);
    }
   else
        VbglHGCMParmPtrSet(&Msg.pNewFileName, (void *)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimSetFileTime
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN long long  */
    HGCMFunctionParameter lpCreationTime; /* IN FILETIME* */
    HGCMFunctionParameter lpLastAccessTime; /* IN const FILETIME**/
    HGCMFunctionParameter lpLastWriteTime; /* IN const FILETIME* */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimSetFileTime;
static BOOL SimSetFileTime(long long hFile, const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime)
{
    TJSimSetFileTime Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_SETFILETIME;
    Msg.hdr.cParms = GDLSIM_CPARMS_SETFILETIME;
    FILETIME time1,time2,time3;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFile, (uint64_t)hFile);
    if(lpCreationTime){
        memcpy(&time1,lpCreationTime,sizeof(FILETIME));
        VbglHGCMParmPtrSet(&Msg.lpCreationTime,(void*)&time1,sizeof(FILETIME));
    }
    else
        VbglHGCMParmPtrSet(&Msg.lpCreationTime,(void*)0,0);
    if(lpLastAccessTime){
        memcpy(&time2,lpLastAccessTime,sizeof(FILETIME));
        VbglHGCMParmPtrSet(&Msg.lpLastAccessTime,(void*)&time2,sizeof(FILETIME));
    }
    else
        VbglHGCMParmPtrSet(&Msg.lpLastAccessTime,(void*)0,0);
    if(lpLastWriteTime){
        memcpy(&time3,lpLastWriteTime,sizeof(FILETIME));
        VbglHGCMParmPtrSet(&Msg.lpLastWriteTime,(void*)&time3,sizeof(FILETIME));
    }
    else
        VbglHGCMParmPtrSet(&Msg.lpLastWriteTime,(void*)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimReadFile
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;
    HGCMFunctionParameter lpBuffer;  /*  IN LPVOID  */
    HGCMFunctionParameter lpNumberOfBytesRead; /* IN LPDWORD */
    HGCMFunctionParameter lpOverlapped; /* IN LPOVERLAPPED */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimReadFile;
static BOOL SimReadFile(long long hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    TJSimReadFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_READFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_READFILE;
    VbglHGCMParmUInt64Set(&Msg.hFile,(uint64_t)hFile);
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmPtrSet(&Msg.lpBuffer,(void *)lpBuffer,lpBuffer?nNumberOfBytesToRead:0);
    VbglHGCMParmPtrSet(&Msg.lpNumberOfBytesRead,(void *)lpNumberOfBytesRead,lpNumberOfBytesRead?sizeof(DWORD):0);
    VbglHGCMParmPtrSet(&Msg.lpOverlapped,(void *)lpOverlapped,lpOverlapped?sizeof(OVERLAPPED):0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimSetEndOfFile
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN long long  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimSetEndOfFile;
static BOOL SimSetEndOfFile(long long hFile)
{
    TJSimSetEndOfFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_SETENDOFFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_SETENDOFFILE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFile, (uint64_t)hFile);   
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimSetFilePointer
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN long long  */
    HGCMFunctionParameter lDistanceToMove; /* IN LONG */
    HGCMFunctionParameter lpDistanceToMoveHigh; /* IN PLONG */
    HGCMFunctionParameter dwMoveMethod; /* IN DWORD */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimSetFilePointer;
static DWORD SimSetFilePointer(long long hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
    TJSimSetFilePointer Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_SETFILEPOINTER;
    Msg.hdr.cParms = GDLSIM_CPARMS_SETFILEPOINTER;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFile, (uint64_t)hFile);  
    VbglHGCMParmUInt32Set(&Msg.lDistanceToMove, (uint32_t)lDistanceToMove);  
    VbglHGCMParmUInt32Set(&Msg.dwMoveMethod, (uint32_t)dwMoveMethod);  
    if(lpDistanceToMoveHigh)
        VbglHGCMParmPtrSet(&Msg.lpDistanceToMoveHigh, (void*)lpDistanceToMoveHigh,sizeof(LONG));
   else
        VbglHGCMParmPtrSet(&Msg.lpDistanceToMoveHigh, (void*)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        DWORD ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}


typedef struct _TJSimWriteFile
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN long long  */
    HGCMFunctionParameter lpBuffer; /* IN LPCVOID */
    HGCMFunctionParameter lpNumberOfBytesWritten; /* IN LPCVOID */
    HGCMFunctionParameter lpOverlapped; /* LPOVERLAPPED */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimWriteFile;
static BOOL SimWriteFile(long long hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    TJSimWriteFile Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_WRITEFILE;
    Msg.hdr.cParms = GDLSIM_CPARMS_WRITEFILE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFile, (uint64_t)hFile); 
    VbglHGCMParmPtrSet(&Msg.lpNumberOfBytesWritten, lpNumberOfBytesWritten,sizeof(DWORD));
    char * pTmpBuffer = NULL;
    if(nNumberOfBytesToWrite && lpBuffer){
        pTmpBuffer = (char *)malloc(nNumberOfBytesToWrite);
        if(pTmpBuffer){
            memcpy(pTmpBuffer,lpBuffer,nNumberOfBytesToWrite);
            VbglHGCMParmPtrSet(&Msg.lpBuffer, (void *)pTmpBuffer,nNumberOfBytesToWrite);
        }
        else
            VbglHGCMParmPtrSet(&Msg.lpBuffer, NULL,0);    
    }
    else{
        VbglHGCMParmPtrSet(&Msg.lpBuffer, NULL,0);
    }
    if(lpOverlapped){
        VbglHGCMParmPtrSet(&Msg.lpOverlapped, (void *)lpOverlapped,sizeof(OVERLAPPED));
    }
    else
        VbglHGCMParmPtrSet(&Msg.lpOverlapped, (void *)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            if(pTmpBuffer)
                free(pTmpBuffer);
            return ret;
        }
    }
    if(pTmpBuffer)
        free(pTmpBuffer);
    return FALSE;
}


typedef struct _TJSimFlushFileBuffers
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hFile;  /*  IN long long  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimFlushFileBuffers;
static BOOL SimFlushFileBuffers(long long hFile)
{
    TJSimFlushFileBuffers Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_FLUSHFILEBUFFERS;
    Msg.hdr.cParms = GDLSIM_CPARMS_FLUSHFILEBUFFERS;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hFile, (uint64_t)hFile);  
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimCloseHandle
{
    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter hObject;  /*  IN long long  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimCloseHandle;
static BOOL SimCloseHandle(long long hObject)
{
    TJSimCloseHandle Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_CLOSEHANDLE;
    Msg.hdr.cParms = GDLSIM_CPARMS_CLOSEHANDLE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    VbglHGCMParmUInt64Set(&Msg.hObject, (uint64_t)hObject);  
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimCreateDirectory
{
    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter lpPathName;  /*  IN LPCTSTR  */
    HGCMFunctionParameter lpSecurityAttributes; /* IN LPSECURITY_ATTRIBUTES */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimCreateDirectory;
static BOOL SimCreateDirectory(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TJSimCreateDirectory Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_CREATEDIRECTORY;
    Msg.hdr.cParms = GDLSIM_CPARMS_CREATEDIRECTORY;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    char tmpstr[1024]={0,};
    if(lpPathName){
        strcpy(tmpstr,lpPathName);
        VbglHGCMParmPtrSet(&Msg.lpPathName, (void *)tmpstr,strlen(tmpstr)+1);
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpPathName, (void *)0,0);
   if(lpSecurityAttributes){
        VbglHGCMParmPtrSet(&Msg.lpSecurityAttributes, (void *)lpSecurityAttributes,sizeof(SECURITY_ATTRIBUTES));
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpSecurityAttributes, (void *)0,0);   
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimRemoveDirectory
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter lpPathName;  /*  IN LPCTSTR  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimRemoveDirectory;
static BOOL SimRemoveDirectory(LPCTSTR lpPathName)
{
    TJSimRemoveDirectory Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_REMOVEDIRECTORY;
    Msg.hdr.cParms = GDLSIM_CPARMS_REMOVEDIRECTORY;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    char tmpstr[1024]={0,};
    
    if(lpPathName){
        strcpy(tmpstr,lpPathName);
        VbglHGCMParmPtrSet(&Msg.lpPathName, (void *)tmpstr,strlen(tmpstr)+1);
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpPathName, (void *)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimGetDiskFreeSpaceEx
{

    VBoxGuestHGCMCallInfo hdr;
     HGCMFunctionParameter lpDirectoryName;   /* IN LPCTSTR */
     HGCMFunctionParameter lpFreeBytesAvailable;    /* IN PULARGE_INTEGER  */
     HGCMFunctionParameter lpTotalNumberOfBytes;    /* IN PULARGE_INTEGER  */
     HGCMFunctionParameter lpTotalNumberOfFreeBytes;    /* IN PULARGE_INTEGER  */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimGetDiskFreeSpaceEx;
static BOOL SimGetDiskFreeSpaceEx(  LPCTSTR lpDirectoryName,  PULARGE_INTEGER lpFreeBytesAvailable,  
                                        PULARGE_INTEGER lpTotalNumberOfBytes,  PULARGE_INTEGER lpTotalNumberOfFreeBytes
)
{
    TJSimGetDiskFreeSpaceEx Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETDISKFREESPACEEX;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETDISKFREESPACEEX;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    if(lpDirectoryName){
        VbglHGCMParmPtrSet(&Msg.lpDirectoryName, (void *)lpDirectoryName,strlen(lpDirectoryName)+1);
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpDirectoryName, (void *)0,0);
   if(lpFreeBytesAvailable){
        VbglHGCMParmPtrSet(&Msg.lpFreeBytesAvailable, (void *)lpFreeBytesAvailable,sizeof(ULARGE_INTEGER));
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpFreeBytesAvailable, (void *)0,0);
   if(lpTotalNumberOfBytes){
        VbglHGCMParmPtrSet(&Msg.lpTotalNumberOfBytes, (void *)lpTotalNumberOfBytes,sizeof(ULARGE_INTEGER));
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpTotalNumberOfBytes, (void *)0,0);
   if(lpTotalNumberOfFreeBytes){
        VbglHGCMParmPtrSet(&Msg.lpTotalNumberOfFreeBytes, (void *)lpTotalNumberOfFreeBytes,sizeof(ULARGE_INTEGER));
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpTotalNumberOfFreeBytes, (void *)0,0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

typedef struct _TJSimGetDriveType
{
    VBoxGuestHGCMCallInfo hdr;
     HGCMFunctionParameter lpRootPathName;   /* IN LPCTSTR */
    HGCMFunctionParameter ret;   /* OUT UINT */
} TJSimGetDriveType;
static UINT SimGetDriveType(  LPCTSTR lpRootPathName)
{
    TJSimGetDriveType Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETDRIVETYPE;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETDRIVETYPE;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    if(lpRootPathName){
        VbglHGCMParmPtrSet(&Msg.lpRootPathName, (void *)lpRootPathName,strlen(lpRootPathName)+1);
    }
   else
        VbglHGCMParmPtrSet(&Msg.lpRootPathName, (void *)0,0);
   
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        UINT ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return DRIVE_UNKNOWN;
}

typedef struct _TJSimGetLastError
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter ret;   /* OUT DWORD */
} TJSimGetLastError;
static DWORD SimGetLastError()
{
    TJSimGetLastError Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETLASTERROR;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETLASTERROR;
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        DWORD ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}

typedef struct _TJSimGetUserName
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pBuf;   /* OUT buf */
    HGCMFunctionParameter ret;   /* OUT BOOL */
} TJSimGetUserName,TJSimGetPasswd;

static BOOL SimGetUserName(char * pUserName,DWORD cbSize)
{
    TJSimGetUserName Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETUSERNAME;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETUSERNAME;
    VbglHGCMParmPtrSet(&Msg.pBuf,pUserName,pUserName?cbSize:0);
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}

static BOOL SimGetPasswd(char * pPasswd,DWORD cbSize)
{
    TJSimGetPasswd Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETPASSWD;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETPASSWD;
    VbglHGCMParmPtrSet(&Msg.pBuf,pPasswd,pPasswd?cbSize:0);
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        BOOL ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return FALSE;
}


typedef struct _TJSimGetMachineCode
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pCode;   /* OUT pCode */
    HGCMFunctionParameter puiOutLen;   /* IN/OUT puiOutLen */
    HGCMFunctionParameter ret;   /* OUT UINT */
} TJSimGetMachineCode;
static UINT  SimGetMachineCode(PBYTE pCode, DWORD InLen,PDWORD puiOutLen)
{
    TJSimGetMachineCode Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETMACHINECODE;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETMACHINECODE;
    VbglHGCMParmPtrSet(&Msg.pCode,pCode,pCode?InLen:0);
    VbglHGCMParmPtrSet(&Msg.puiOutLen,puiOutLen,puiOutLen?sizeof(PDWORD):0);
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        UINT ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}

typedef struct _TJSimGetCurrentModuleDir
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter pBuf;   /* OUT pBuf */
    HGCMFunctionParameter ret;   /* OUT UINT */
} TJSimGetCurrentModuleDir;
static DWORD  SimGetCurrentModuleDir(LPTSTR pBuf, DWORD InLen)
{
    TJSimGetCurrentModuleDir Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETCURRENTMODULEDIR;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETCURRENTMODULEDIR;
    VbglHGCMParmPtrSet(&Msg.pBuf,pBuf,pBuf?InLen:0);
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        DWORD ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}

typedef struct _TJSimGetDiskType
{

    VBoxGuestHGCMCallInfo hdr;
    HGCMFunctionParameter lpRootPathName;   /* IN LPCTSTR */
    HGCMFunctionParameter ret;   /* OUT UINT */
} TJSimGetDiskType;
static UINT  SimGetDiskType(LPCTSTR lpRootPathName)
{
    TJSimGetDiskType Msg;

    Msg.hdr.result = VERR_WRONG_ORDER;
    Msg.hdr.u32ClientID = u32ClientId;
    Msg.hdr.u32Function = GDLSIM_FN_GETDISKTYPE;
    Msg.hdr.cParms = GDLSIM_CPARMS_GETDISKTYPE;
    char tRootPathName[MAX_PATH]={0};
    strcpy(tRootPathName,lpRootPathName);
    VbglHGCMParmPtrSet(&Msg.lpRootPathName,tRootPathName,lpRootPathName?strlen(lpRootPathName):0);
    VbglHGCMParmUInt32Set(&Msg.ret, (uint32_t)0);
    
    int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
    if (RT_SUCCESS(rc))
    {
        UINT ret;
        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
        if (RT_SUCCESS(rc))
        {
            return ret;
        }
    }
    return 0;
}

typedef struct _TJSimPathFileExists
{
    VBoxGuestHGCMCallInfo hdr;    
    HGCMFunctionParameter pszPath; /* IN LPCTSTR  */
    HGCMFunctionParameter    ret;   /* OUT BOOL */
} TJSimPathFileExists;
static BOOL  SimPathFileExists(LPCTSTR pszPath)
{
	TJSimPathFileExists Msg;
	Msg.hdr.result = VERR_WRONG_ORDER;
	Msg.hdr.u32ClientID = u32ClientId;
	Msg.hdr.u32Function = GDLSIM_FN_PATHFILEEXISTS;
	Msg.hdr.cParms = GDLSIM_CPARMS_PATHFILEEXISTS;
       char tPszPath[MAX_PATH]={0};
       int len = strlen(pszPath);
       if(pszPath && len>0)
        {
            strcpy(tPszPath,pszPath);
        }
	VbglHGCMParmPtrSet(&Msg.pszPath, tPszPath,len>0?len:0);
	VbglHGCMParmUInt32Set(&Msg.ret, 0); 


	int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
	if (RT_SUCCESS(rc))
	{
	    rc = Msg.hdr.result;
	    if (RT_SUCCESS(rc))
	    {
	        BOOL ret;
	        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
	        if (RT_SUCCESS(rc))
	        {
	            return ret;
	        }
	    }
	}
	return FALSE;
}

typedef struct _TJSimSHGetFolderPath
{

    VBoxGuestHGCMCallInfo hdr;

    HGCMFunctionParameter hwndOwner; 

    HGCMFunctionParameter nFolder; 

    HGCMFunctionParameter hToken; 

    HGCMFunctionParameter dwFlags; 

    HGCMFunctionParameter pszPath; 
	
    HGCMFunctionParameter ret; /* */	
} TJSimSHGetFolderPath;
/*
static HRESULT SimSHGetFolderPath(HWND hwndOwner,int nFolder,HANDLE hToken,DWORD dwFlags, LPTSTR pszPath)
{
	TJSimSHGetFolderPath Msg;
    char szPath[MAX_PATH];

	Msg.hdr.result = VERR_WRONG_ORDER;
	Msg.hdr.u32ClientID = u32ClientId;
	Msg.hdr.u32Function = GDLSIM_FN_SHGETFOLDERPATH;
	Msg.hdr.cParms = GDLSIM_CPARMS_SHGETFOLDERPATH;

	VbglHGCMParmUInt32Set(&Msg.hwndOwner,(uint32_t) hwndOwner);
	VbglHGCMParmUInt32Set(& Msg.nFolder, (uint32_t)nFolder);
	VbglHGCMParmUInt32Set(& Msg.hToken,(uint32_t)hToken);
	VbglHGCMParmUInt32Set(& Msg.dwFlags, (uint32_t)dwFlags);
	VbglHGCMParmPtrSet(&Msg.pszPath, (void *)szPath,MAX_PATH);
	VbglHGCMParmUInt32Set(&Msg.ret, 0); 

	HRESULT ret = E_INVALIDARG;
	int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CALL(sizeof(Msg)), &Msg, sizeof(Msg));
	if (RT_SUCCESS(rc))
	{
	    rc = Msg.hdr.result;
	    if (RT_SUCCESS(rc))
	    {
	        rc = VbglHGCMParmUInt32Get(&Msg.ret, (uint32_t*)&ret);
	     	 AssertRC(rc);
             if(pszPath){
                    strcpy(pszPath,szPath);
                }
	    }
	}
	return ret;	
}
*/
/**
 * DLL entry point.
 */
BOOL WINAPI DllMain(HINSTANCE hInstance,
                    DWORD     dwReason,
                    LPVOID    lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            RTR3InitDll(RTR3INIT_FLAGS_UNOBTRUSIVE);
            VbglR3Init();

            FhahaLibLoadConfiguration();

            FhahaLibVerbose(0, "FhahaLib: v%s r%s (%s %s) loaded\n",
                            RTBldCfgVersion(), RTBldCfgRevisionStr(),
                            __DATE__, __TIME__);

            VBoxGuestHGCMConnectInfo Info;
            Info.result = VERR_WRONG_ORDER;
            Info.Loc.type = VMMDevHGCMLoc_LocalHost_Existing;
            RT_ZERO(Info.Loc.u);
            strcpy(Info.Loc.u.host.achName, "OutEnvFSSvc");
            Info.u32ClientID = UINT32_MAX;  /* try make valgrind shut up. */

            int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_CONNECT, &Info, sizeof(Info));
            if (RT_SUCCESS(rc))
            {
                rc = Info.result;
                if (RT_SUCCESS(rc))
                    u32ClientId = Info.u32ClientID;
            }
            if (rc == VERR_HGCM_SERVICE_NOT_FOUND)
                rc = VINF_PERMISSION_DENIED;       

            DisableThreadLibraryCalls(hInstance);
            hDllInstance = hInstance;
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            FhahaLibVerbose(0, "FhahaLib: Unloaded\n");
            VBoxGuestHGCMDisconnectInfo Info;
            Info.result = VERR_WRONG_ORDER;
            Info.u32ClientID = u32ClientId;

            int rc = vbglR3DoIOCtl(VBOXGUEST_IOCTL_HGCM_DISCONNECT, &Info, sizeof(Info));
            if (RT_SUCCESS(rc))
                rc = Info.result;
            VbglR3Term();
            /// @todo RTR3Term();
            break;
        }

        default:
            break;
    }
    return TRUE;
}

DWORD WINAPI FhahaLibDebug(void)
{
    return 0;
}

static FhahaLib_Func glFuncs={FALSE,0,};
FhahaLib_Func * WINAPI  FhahaLibGetProcStruct()
{
	if(glFuncs.isLoaded==FALSE){
		glFuncs.isLoaded = TRUE;
        glFuncs.CloseHandle = SimCloseHandle;
        glFuncs.CopyFile = SimCopyFile;
        glFuncs.CreateDirectory = SimCreateDirectory;
        glFuncs.CreateFile = SimCreateFile;
        glFuncs.DeleteFile = SimDeleteFile;
        glFuncs.FindClose = SimFindClose;
        glFuncs.FindFirstFile = SimFindFirstFile;
        glFuncs.FindNextFile = SimFindNextFile;
        glFuncs.FlushFileBuffers = SimFlushFileBuffers;
        glFuncs.GetCurrentDirectory = SimGetCurrentDirectory;
        glFuncs.GetLogicalDrives = SimGetLogicalDrives;
        glFuncs.GetFileInformationByHandle = SimGetFileInformationByHandle;
        glFuncs.GetFileSize = SimGetFileSize;
        glFuncs.GetFileSizeEx = SimGetFileSizeEx;
        glFuncs.GetLastError = SimGetLastError;
        glFuncs.GetTempPath = SimGetTempPath;
        glFuncs.MoveFile = SimMoveFile;
        glFuncs.ReadFile = SimReadFile;
        glFuncs.RemoveDirectory = SimRemoveDirectory;
        glFuncs.SetCurrentDirectory = SimSetCurrentDirectory;
        glFuncs.SetEndOfFile = SimSetEndOfFile;
        glFuncs.SetFilePointer = SimSetFilePointer;
        glFuncs.SetFileTime = SimSetFileTime;
        glFuncs.WriteFile = SimWriteFile;
        glFuncs.GetDiskFreeSpaceEx = SimGetDiskFreeSpaceEx;
        glFuncs.GetDriveType = SimGetDriveType;
        glFuncs.GetCurrentModuleDir= SimGetCurrentModuleDir;
        glFuncs.PathFileExists = SimPathFileExists;
        glFuncs.GetDiskType = SimGetDiskType;
        //glFuncs.SHGetFolderPath = SimSHGetFolderPath;
        glFuncs.GetFileAttributesEx = SimGetFileAttributesEx;
	}
	return &glFuncs;

}

