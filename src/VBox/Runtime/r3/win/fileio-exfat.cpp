/* $Id: fileio-exfat.cpp $ */
/** @file
 * IPRT - File I/O, native implementation for the exfat file system.
 */

/*
 * Copyright (C) 2020-2025 Fhaha Inc.
 *
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#define LOG_GROUP RTLOGGROUP_DIR
#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0500
#endif
#include <Windows.h>
extern "C" {
#include "exfat.h"
}
#include <iprt/file.h>
#include <iprt/path.h>
#include <iprt/assert.h>
#include <iprt/string.h>
#include <iprt/err.h>
#include <iprt/log.h>
#include "internal/file.h"
#include "internal/fs.h"
#include "internal/path.h"


/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/

static struct exfat * ef = NULL;

static void init_exfatfs(void)
{
    if(ef==NULL){
        ef = (struct exfat *)malloc(sizeof(struct exfat));
        if(exfat_mount(ef,"D:\\Code\\exfat_forwin\\win\\exfatDlg\\fhaha.img","rw")){
            free(ef);
            ef=NULL;
        }
        else{
            set_dllexfat(ef);
        }
    }
}

#define EXFAT_CHECK(kk)    do{init_exfatfs();if(ef==NULL)return kk;}while(0)
/**
 * This is wrapper around the ugly SetFilePointer api.
 *
 * It's equivalent to SetFilePointerEx which we so unfortunately cannot use because of
 * it not being present in NT4 GA.
 *
 * @returns Success indicator. Extended error information obtainable using GetLastError().
 * @param   hFile       Filehandle.
 * @param   offSeek     Offset to seek.
 * @param   poffNew     Where to store the new file offset. NULL allowed.
 * @param   uMethod     Seek method. (The windows one!)
 */
DECLINLINE(bool) EFSetFilePointer(RTFILE hFile, uint64_t offSeek, uint64_t *poffNew, unsigned uMethod)
{
#if 0
    bool            fRc;
    LARGE_INTEGER   off;

    off.QuadPart = offSeek;
#if 1
    if (off.LowPart != INVALID_SET_FILE_POINTER)
    {
        off.LowPart = SetFilePointer((HANDLE)RTFileToNative(hFile), off.LowPart, &off.HighPart, uMethod);
        fRc = off.LowPart != INVALID_SET_FILE_POINTER;
    }
    else
    {
        SetLastError(NO_ERROR);
        off.LowPart = SetFilePointer((HANDLE)RTFileToNative(hFile), off.LowPart, &off.HighPart, uMethod);
        fRc = GetLastError() == NO_ERROR;
    }
#else
    fRc = SetFilePointerEx((HANDLE)RTFileToNative(hFile), off, &off, uMethod);
#endif
    if (fRc && poffNew)
        *poffNew = off.QuadPart;
    return fRc;
#endif
    return false;
}


/**
 * This is a helper to check if an attempt was made to grow a file beyond the
 * limit of the filesystem.
 *
 * @returns true for file size limit exceeded.
 * @param   hFile       Filehandle.
 * @param   offSeek     Offset to seek.
 * @param   uMethod     The seek method.
 */
DECLINLINE(bool) EFIsBeyondLimit(RTFILE hFile, uint64_t offSeek, unsigned uMethod)
{
#if 0
    bool fIsBeyondLimit = false;

    /*
     * Get the current file position and try set the new one.
     * If it fails with a seek error it's because we hit the file system limit.
     */
/** @todo r=bird: I'd be very interested to know on which versions of windows and on which file systems
 * this supposedly works. The fastfat sources in the latest WDK makes no limit checks during
 * file seeking, only at the time of writing (and some other odd ones we cannot make use of). */
    uint64_t offCurrent;
    if (EFSetFilePointer(hFile, 0, &offCurrent, FILE_CURRENT))
    {
        if (!EFSetFilePointer(hFile, offSeek, NULL, uMethod))
            fIsBeyondLimit = GetLastError() == ERROR_SEEK;
        else /* Restore file pointer on success. */
            EFSetFilePointer(hFile, offCurrent, NULL, FILE_BEGIN);
    }

    return fIsBeyondLimit;
#endif
    return false;
}


RTR3DECL(int) EFFileFromNative(PRTFILE pFile, RTHCINTPTR uNative)
{
#if 0
    HANDLE h = (HANDLE)uNative;
    AssertCompile(sizeof(h) == sizeof(uNative));
    if (h == INVALID_HANDLE_VALUE)
    {
        AssertMsgFailed(("%p\n", uNative));
        *pFile = NIL_RTFILE;
        return VERR_INVALID_HANDLE;
    }
    *pFile = (RTFILE)h;
    return VINF_SUCCESS;
#endif
    return VERR_NOT_IMPLEMENTED;
}


RTR3DECL(RTHCINTPTR) EFFileToNative(RTFILE hFile)
{
    AssertReturn(hFile != NIL_RTFILE, (RTHCINTPTR)INVALID_HANDLE_VALUE);
    return (RTHCINTPTR)hFile;
}


RTR3DECL(int) EFFileOpen(PRTFILE pFile, const char *pszFilename, uint64_t fOpen)
{
    /*
     * Validate input.
     */
    EXFAT_CHECK(VERR_INVALID_PARAMETER);
    if (!pFile)
    {
        AssertMsgFailed(("Invalid pFile\n"));
        return VERR_INVALID_PARAMETER;
    }
    *pFile = NIL_RTFILE;
    if (!pszFilename)
    {
        AssertMsgFailed(("Invalid pszFilename\n"));
        return VERR_INVALID_PARAMETER;
    }

    /*
     * Merge forced open flags and validate them.
     */
    int rc = rtFileRecalcAndValidateFlags(&fOpen);
    if (RT_FAILURE(rc))
        return rc;

    /*
     * Determine disposition, access, share mode, creation flags, and security attributes
     * for the CreateFile API call.
     */
    DWORD dwCreationDisposition;
    switch (fOpen & RTFILE_O_ACTION_MASK)
    {
        case RTFILE_O_OPEN:
            dwCreationDisposition = fOpen & RTFILE_O_TRUNCATE ? TRUNCATE_EXISTING : OPEN_EXISTING;
            break;
        case RTFILE_O_OPEN_CREATE:
            dwCreationDisposition = OPEN_ALWAYS;
            break;
        case RTFILE_O_CREATE:
            dwCreationDisposition = CREATE_NEW;
            break;
        case RTFILE_O_CREATE_REPLACE:
            dwCreationDisposition = CREATE_ALWAYS;
            break;
        default:
            AssertMsgFailed(("Impossible fOpen=%#llx\n", fOpen));
            return VERR_INVALID_PARAMETER;
    }

    DWORD dwDesiredAccess;
    switch (fOpen & RTFILE_O_ACCESS_MASK)
    {
        case RTFILE_O_READ:
            dwDesiredAccess = FILE_GENERIC_READ; /* RTFILE_O_APPEND is ignored. */
            break;
        case RTFILE_O_WRITE:
            dwDesiredAccess = fOpen & RTFILE_O_APPEND
                            ? FILE_GENERIC_WRITE & ~FILE_WRITE_DATA
                            : FILE_GENERIC_WRITE;
            break;
        case RTFILE_O_READWRITE:
            dwDesiredAccess = fOpen & RTFILE_O_APPEND
                            ? FILE_GENERIC_READ | (FILE_GENERIC_WRITE & ~FILE_WRITE_DATA)
                            : FILE_GENERIC_READ | FILE_GENERIC_WRITE;
            break;
        default:
            AssertMsgFailed(("Impossible fOpen=%#llx\n", fOpen));
            return VERR_INVALID_PARAMETER;
    }
    if (dwCreationDisposition == TRUNCATE_EXISTING)
        /* Required for truncating the file (see MSDN), it is *NOT* part of FILE_GENERIC_WRITE. */
        dwDesiredAccess |= GENERIC_WRITE;

    /* RTFileSetMode needs following rights as well. */
    switch (fOpen & RTFILE_O_ACCESS_ATTR_MASK)
    {
        case RTFILE_O_ACCESS_ATTR_READ:      dwDesiredAccess |= FILE_READ_ATTRIBUTES  | SYNCHRONIZE; break;
        case RTFILE_O_ACCESS_ATTR_WRITE:     dwDesiredAccess |= FILE_WRITE_ATTRIBUTES | SYNCHRONIZE; break;
        case RTFILE_O_ACCESS_ATTR_READWRITE: dwDesiredAccess |= FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE; break;
        default:
            /* Attributes access is the same as the file access. */
            switch (fOpen & RTFILE_O_ACCESS_MASK)
            {
                case RTFILE_O_READ:          dwDesiredAccess |= FILE_READ_ATTRIBUTES  | SYNCHRONIZE; break;
                case RTFILE_O_WRITE:         dwDesiredAccess |= FILE_WRITE_ATTRIBUTES | SYNCHRONIZE; break;
                case RTFILE_O_READWRITE:     dwDesiredAccess |= FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE; break;
                default:
                    AssertMsgFailed(("Impossible fOpen=%#llx\n", fOpen));
                    return VERR_INVALID_PARAMETER;
            }
    }

    DWORD dwShareMode;
    switch (fOpen & RTFILE_O_DENY_MASK)
    {
        case RTFILE_O_DENY_NONE:                                dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE; break;
        case RTFILE_O_DENY_READ:                                dwShareMode = FILE_SHARE_WRITE; break;
        case RTFILE_O_DENY_WRITE:                               dwShareMode = FILE_SHARE_READ; break;
        case RTFILE_O_DENY_READWRITE:                           dwShareMode = 0; break;

        case RTFILE_O_DENY_NOT_DELETE | RTFILE_O_DENY_NONE:     dwShareMode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE; break;
        case RTFILE_O_DENY_NOT_DELETE | RTFILE_O_DENY_READ:     dwShareMode = FILE_SHARE_DELETE | FILE_SHARE_WRITE; break;
        case RTFILE_O_DENY_NOT_DELETE | RTFILE_O_DENY_WRITE:    dwShareMode = FILE_SHARE_DELETE | FILE_SHARE_READ; break;
        case RTFILE_O_DENY_NOT_DELETE | RTFILE_O_DENY_READWRITE:dwShareMode = FILE_SHARE_DELETE; break;
        default:
            AssertMsgFailed(("Impossible fOpen=%#llx\n", fOpen));
            return VERR_INVALID_PARAMETER;
    }
#if 0
    SECURITY_ATTRIBUTES  SecurityAttributes;
    PSECURITY_ATTRIBUTES pSecurityAttributes = NULL;
    if (fOpen & RTFILE_O_INHERIT)
    {
        SecurityAttributes.nLength              = sizeof(SecurityAttributes);
        SecurityAttributes.lpSecurityDescriptor = NULL;
        SecurityAttributes.bInheritHandle       = TRUE;
        pSecurityAttributes = &SecurityAttributes;
    }

    DWORD dwFlagsAndAttributes;
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    if (fOpen & RTFILE_O_WRITE_THROUGH)
        dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
    if (fOpen & RTFILE_O_ASYNC_IO)
        dwFlagsAndAttributes |= FILE_FLAG_OVERLAPPED;
    if (fOpen & RTFILE_O_NO_CACHE)
    {
        dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
        dwDesiredAccess &= ~FILE_APPEND_DATA;
    }

    /*
     * Open/Create the file.
     */
    PRTUTF16 pwszFilename;
    rc = RTStrToUtf16(pszFilename, &pwszFilename);
    if (RT_FAILURE(rc))
        return rc;
    EXFAT_CHECK(rc);
    if(ef){

        return rc;
    }
    HANDLE hFile = CreateFileW(pwszFilename,
                               dwDesiredAccess,
                               dwShareMode,
                               pSecurityAttributes,
                               dwCreationDisposition,
                               dwFlagsAndAttributes,
                               NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        bool fCreated = dwCreationDisposition == CREATE_ALWAYS
                     || dwCreationDisposition == CREATE_NEW
                     || (dwCreationDisposition == OPEN_ALWAYS && GetLastError() == 0);

        /*
         * Turn off indexing of directory through Windows Indexing Service.
         */
        if (    fCreated
            &&  (fOpen & RTFILE_O_NOT_CONTENT_INDEXED))
        {
            if (!SetFileAttributesW(pwszFilename, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED))
                rc = RTErrConvertFromWin32(GetLastError());
        }
        /*
         * Do we need to truncate the file?
         */
        else if (    !fCreated
                 &&     (fOpen & (RTFILE_O_TRUNCATE | RTFILE_O_ACTION_MASK))
                     == (RTFILE_O_TRUNCATE | RTFILE_O_OPEN_CREATE))
        {
            if (!SetEndOfFile(hFile))
                rc = RTErrConvertFromWin32(GetLastError());
        }
        if (RT_SUCCESS(rc))
        {
            *pFile = (RTFILE)hFile;
            Assert((HANDLE)*pFile == hFile);
            RTUtf16Free(pwszFilename);
            return VINF_SUCCESS;
        }

        CloseHandle(hFile);
    }
    else
        rc = RTErrConvertFromWin32(GetLastError());
    RTUtf16Free(pwszFilename);
#else
    struct exfat_node* node = NULL;
    char tmpname[MAX_PATH] = {0};
    char localfilename[MAX_PATH]={0};
    int findres = -1;
    strcpy(tmpname,pszFilename);
    char * shortname  = strchr((char *)tmpname,'\\');
    while( shortname  != NULL ){
        *shortname = '/';
        shortname = strchr((char*)tmpname,'\\');
    }
    if(tmpname[0] != '/')
    {
        localfilename[0]='/';
        strcpy(&localfilename[1],tmpname);
        
    }
    else{
        strcpy(localfilename,tmpname);
    }
    
    if(dwCreationDisposition != 0){
        switch(dwCreationDisposition){
        case OPEN_EXISTING:
            findres = exfat_lookup(ef,&node,localfilename);
            if(findres==0){
                *pFile = (RTFILE) node;
                rc = VINF_SUCCESS;
            }
            else{
                rc = VERR_INVALID_FLAGS;
            }
            break;
        case CREATE_NEW:
            findres = exfat_lookup(ef,&node,localfilename);
            if(findres==0){
                exfat_put_node(ef,node);
                rc = VERR_INVALID_FLAGS;
            }
            else{
                int createres = exfat_mknod(ef,localfilename);
                if(createres == 0){
                    findres = exfat_lookup(ef,&node,localfilename);
                    if(findres==0){
                        *pFile = (RTFILE) node;
                        rc = VINF_SUCCESS;
                    }
                    else{
                        rc = VERR_INVALID_FLAGS;
                    }
                }
                else{
                    rc = VERR_INVALID_FLAGS;
                }
            }
            break;
        case OPEN_ALWAYS:
            findres = exfat_lookup(ef,&node,localfilename);
            if(findres==0){
                *pFile = (RTFILE) node;
                rc = VINF_SUCCESS;
            }
            else{
                int createres = exfat_mknod(ef,localfilename);
                if(createres == 0){
                    findres = exfat_lookup(ef,&node,localfilename);
                    if(findres==0){
                        *pFile = (RTFILE) node;
                        rc = VINF_SUCCESS;
                    }
                    else{
                        rc = VERR_INVALID_FLAGS;
                    }
                }
                else{
                    rc = VERR_INVALID_FLAGS;
                }
            }
            break;
        case TRUNCATE_EXISTING:
            findres = exfat_lookup(ef,&node,localfilename);
            if(findres==0){
                exfat_truncate(ef,node,0,1);
                *pFile = (RTFILE) node;
                rc = VINF_SUCCESS;
            }
            else{
                rc = VERR_INVALID_FLAGS;
            }
            break;
        case CREATE_ALWAYS:
            findres = exfat_lookup(ef,&node,localfilename);
            if(findres==0){
                exfat_truncate(ef,node,0,1);
                *pFile = (RTFILE) node;
                rc = VINF_SUCCESS;
            }
            else{
                int createres = exfat_mknod(ef,localfilename);
                if(createres == 0){
                    findres = exfat_lookup(ef,&node,localfilename);
                    if(findres==0){
                        *pFile = (RTFILE) node;
                        rc = VINF_SUCCESS;
                    }
                    else{
                        rc = VERR_INVALID_FLAGS;
                    }
                }
                else{
                    rc = VERR_INVALID_FLAGS;
                }
            }
            break;
        
        }
#if 0
        int createres = exfat_mknod(ef,localfilename);
        if(createres == 0){
            findres = exfat_lookup(ef,&node,localfilename);
            if(findres==0){
                *pFile = (RTFILE) node;
                rc = VINF_SUCCESS;
            }

        }
        else
            rc =  VERR_INVALID_PARAMETER;
#endif       
    }
       
#endif
    return rc;
}


RTR3DECL(int) EFFileOpenBitBucket(PRTFILE phFile, uint64_t fAccess)
{
    AssertReturn(   fAccess == RTFILE_O_READ
                 || fAccess == RTFILE_O_WRITE
                 || fAccess == RTFILE_O_READWRITE,
                 VERR_INVALID_PARAMETER);
    //return RTFileOpen(phFile, "NUL", fAccess | RTFILE_O_DENY_NONE | RTFILE_O_OPEN);
    return VERR_NOT_IMPLEMENTED;
}


RTR3DECL(int) EFFileClose(RTFILE hFile)
{
    if (hFile == NIL_RTFILE)
        return VINF_SUCCESS;
    struct exfat_node * pnode = (struct exfat_node *)hFile;
	exfat_flush_node(ef,pnode);
    exfat_put_node(ef,pnode);
    pnode->curpos = 0;
    return VINF_SUCCESS;
}


RTFILE efFileGetStandard(RTHANDLESTD enmStdHandle)
{
#if 0
    DWORD dwStdHandle;
    switch (enmStdHandle)
    {
        case RTHANDLESTD_INPUT:     dwStdHandle = STD_INPUT_HANDLE;  break;
        case RTHANDLESTD_OUTPUT:    dwStdHandle = STD_OUTPUT_HANDLE; break;
        case RTHANDLESTD_ERROR:     dwStdHandle = STD_ERROR_HANDLE;  break;
            break;
        default:
            AssertFailedReturn(NIL_RTFILE);
    }

    HANDLE hNative = GetStdHandle(dwStdHandle);
    if (hNative == INVALID_HANDLE_VALUE)
        return NIL_RTFILE;

    RTFILE hFile = (RTFILE)(uintptr_t)hNative;
    AssertReturn((HANDLE)(uintptr_t)hFile == hNative, NIL_RTFILE);
    return hFile;
#endif
    return NIL_RTFILE;
}


RTR3DECL(int) EFFileSeek(RTFILE hFile, int64_t offSeek, unsigned uMethod, uint64_t *poffActual)
{
#if 0
    static ULONG aulSeekRecode[] =
    {
        FILE_BEGIN,
        FILE_CURRENT,
        FILE_END,
    };

    /*
     * Validate input.
     */
    if (uMethod > RTFILE_SEEK_END)
    {
        AssertMsgFailed(("Invalid uMethod=%d\n", uMethod));
        return VERR_INVALID_PARAMETER;
    }

    /*
     * Execute the seek.
     */
    if (EFSetFilePointer(hFile, offSeek, poffActual, aulSeekRecode[uMethod]))
        return VINF_SUCCESS;
    return RTErrConvertFromWin32(GetLastError());
#endif
    if (uMethod > RTFILE_SEEK_END)
    {
        AssertMsgFailed(("Invalid uMethod=%d\n", uMethod));
        return VERR_INVALID_PARAMETER;
    } 
    struct exfat_node * pnode = (struct exfat_node*)hFile; 
    switch(uMethod){
    case RTFILE_SEEK_BEGIN:
        pnode->curpos = offSeek;
        if(poffActual)
            *poffActual = pnode->curpos;
        break;
    case RTFILE_SEEK_CURRENT:
        pnode->curpos += offSeek;
        if(poffActual)
            *poffActual = pnode->curpos;
        break;
    case RTFILE_SEEK_END:
        pnode->curpos = pnode->size;
        pnode->curpos += offSeek;
        if(poffActual)
            *poffActual = pnode->curpos;
        break;
    default:
        return VERR_INVALID_PARAMETER;
    }
    return VINF_SUCCESS;
}


RTR3DECL(int) EFFileRead(RTFILE hFile, void *pvBuf, size_t cbToRead, size_t *pcbRead)
{
    if (cbToRead <= 0)
        return VINF_SUCCESS;
    ULONG cbToReadAdj = (ULONG)cbToRead;
    AssertReturn(cbToReadAdj == cbToRead, VERR_NUMBER_TOO_BIG);

    ULONG cbRead = 0;
#if 0
    if (ReadFile((HANDLE)RTFileToNative(hFile), pvBuf, cbToReadAdj, &cbRead, NULL))
    {
        if (pcbRead)
            /* Caller can handle partial reads. */
            *pcbRead = cbRead;
        else
        {
            /* Caller expects everything to be read. */
            while (cbToReadAdj > cbRead)
            {
                ULONG cbReadPart = 0;
                if (!ReadFile((HANDLE)RTFileToNative(hFile), (char*)pvBuf + cbRead, cbToReadAdj - cbRead, &cbReadPart, NULL))
                    return RTErrConvertFromWin32(GetLastError());
                if (cbReadPart == 0)
                    return VERR_EOF;
                cbRead += cbReadPart;
            }
        }
        return VINF_SUCCESS;
    }

    /*
     * If it's a console, we might bump into out of memory conditions in the
     * ReadConsole call.
     */
    DWORD dwErr = GetLastError();
    if (dwErr == ERROR_NOT_ENOUGH_MEMORY)
    {
        ULONG cbChunk = cbToReadAdj / 2;
        if (cbChunk > 16*_1K)
            cbChunk = 16*_1K;
        else
            cbChunk = RT_ALIGN_32(cbChunk, 256);

        cbRead = 0;
        while (cbToReadAdj > cbRead)
        {
            ULONG cbToRead   = RT_MIN(cbChunk, cbToReadAdj - cbRead);
            ULONG cbReadPart = 0;
            if (!ReadFile((HANDLE)RTFileToNative(hFile), (char *)pvBuf + cbRead, cbToRead, &cbReadPart, NULL))
            {
                /* If we failed because the buffer is too big, shrink it and
                   try again. */
                dwErr = GetLastError();
                if (   dwErr == ERROR_NOT_ENOUGH_MEMORY
                    && cbChunk > 8)
                {
                    cbChunk /= 2;
                    continue;
                }
                return RTErrConvertFromWin32(dwErr);
            }
            cbRead += cbReadPart;

            /* Return if the caller can handle partial reads, otherwise try
               fill the buffer all the way up. */
            if (pcbRead)
            {
                *pcbRead = cbRead;
                break;
            }
            if (cbReadPart == 0)
                return VERR_EOF;
        }
        return VINF_SUCCESS;
    }

    return RTErrConvertFromWin32(dwErr);
#endif
    struct exfat_node * pnode = (struct exfat_node *)hFile;
    if(((uint64_t)pnode->curpos) >= pnode->size){
        return VERR_EOF;
    }
    size_t retsize =  exfat_generic_pread(ef,pnode,pvBuf,cbToRead,pnode->curpos);
    pnode->curpos += retsize;
    if(pcbRead)
        *pcbRead = retsize;
    if(retsize < cbToRead ){
        return VINF_SUCCESS;
    }
    

    return VINF_SUCCESS;
}


RTR3DECL(int) EFFileWrite(RTFILE hFile, const void *pvBuf, size_t cbToWrite, size_t *pcbWritten)
{
    if (cbToWrite <= 0)
        return VINF_SUCCESS;
    ULONG cbToWriteAdj = (ULONG)cbToWrite;
    AssertReturn(cbToWriteAdj == cbToWrite, VERR_NUMBER_TOO_BIG);
#if 0
    ULONG cbWritten = 0;
    if (WriteFile((HANDLE)RTFileToNative(hFile), pvBuf, cbToWriteAdj, &cbWritten, NULL))
    {
        if (pcbWritten)
            /* Caller can handle partial writes. */
            *pcbWritten = cbWritten;
        else
        {
            /* Caller expects everything to be written. */
            while (cbToWriteAdj > cbWritten)
            {
                ULONG cbWrittenPart = 0;
                if (!WriteFile((HANDLE)RTFileToNative(hFile), (char*)pvBuf + cbWritten,
                               cbToWriteAdj - cbWritten, &cbWrittenPart, NULL))
                {
                    int rc = RTErrConvertFromWin32(GetLastError());
                    if (   rc == VERR_DISK_FULL
                        && EFIsBeyondLimit(hFile, cbToWriteAdj - cbWritten, FILE_CURRENT)
                       )
                        rc = VERR_FILE_TOO_BIG;
                    return rc;
                }
                if (cbWrittenPart == 0)
                    return VERR_WRITE_ERROR;
                cbWritten += cbWrittenPart;
            }
        }
        return VINF_SUCCESS;
    }

    /*
     * If it's a console, we might bump into out of memory conditions in the
     * WriteConsole call.
     */
    DWORD dwErr = GetLastError();
    if (dwErr == ERROR_NOT_ENOUGH_MEMORY)
    {
        ULONG cbChunk = cbToWriteAdj / 2;
        if (cbChunk > _32K)
            cbChunk = _32K;
        else
            cbChunk = RT_ALIGN_32(cbChunk, 256);

        cbWritten = 0;
        while (cbToWriteAdj > cbWritten)
        {
            ULONG cbToWrite     = RT_MIN(cbChunk, cbToWriteAdj - cbWritten);
            ULONG cbWrittenPart = 0;
            if (!WriteFile((HANDLE)RTFileToNative(hFile), (const char *)pvBuf + cbWritten, cbToWrite, &cbWrittenPart, NULL))
            {
                /* If we failed because the buffer is too big, shrink it and
                   try again. */
                dwErr = GetLastError();
                if (   dwErr == ERROR_NOT_ENOUGH_MEMORY
                    && cbChunk > 8)
                {
                    cbChunk /= 2;
                    continue;
                }
                int rc = RTErrConvertFromWin32(dwErr);
                if (   rc == VERR_DISK_FULL
                    && EFIsBeyondLimit(hFile, cbToWriteAdj - cbWritten, FILE_CURRENT))
                    rc = VERR_FILE_TOO_BIG;
                return rc;
            }
            cbWritten += cbWrittenPart;

            /* Return if the caller can handle partial writes, otherwise try
               write out everything. */
            if (pcbWritten)
            {
                *pcbWritten = cbWritten;
                break;
            }
            if (cbWrittenPart == 0)
                return VERR_WRITE_ERROR;
        }
        return VINF_SUCCESS;
    }

    int rc = RTErrConvertFromWin32(dwErr);
    if (   rc == VERR_DISK_FULL
        && EFIsBeyondLimit(hFile, cbToWriteAdj - cbWritten, FILE_CURRENT))
        rc = VERR_FILE_TOO_BIG;
    return rc;
#endif
    struct exfat_node * pnode = (struct exfat_node *)hFile;

    size_t retsize = exfat_generic_pwrite(ef,pnode,pvBuf,cbToWrite,pnode->curpos);
    pnode->curpos += retsize;
    if(pcbWritten)
        *pcbWritten = retsize;
    return VINF_SUCCESS;
}


RTR3DECL(int) EFFileFlush(RTFILE hFile)
{
    if (!FlushFileBuffers((HANDLE)RTFileToNative(hFile)))
    {
        int rc = GetLastError();
        Log(("FlushFileBuffers failed with %d\n", rc));
        return RTErrConvertFromWin32(rc);
    }
    return VINF_SUCCESS;
}


RTR3DECL(int) EFFileSetSize(RTFILE hFile, uint64_t cbSize)
{
#if 0
    /*
     * Get current file pointer.
     */
    int         rc;
    uint64_t    offCurrent;
    if (EFSetFilePointer(hFile, 0, &offCurrent, FILE_CURRENT))
    {
        /*
         * Set new file pointer.
         */
        if (EFSetFilePointer(hFile, cbSize, NULL, FILE_BEGIN))
        {
            /* set file pointer */
            if (SetEndOfFile((HANDLE)RTFileToNative(hFile)))
            {
                /*
                 * Restore file pointer and return.
                 * If the old pointer was beyond the new file end, ignore failure.
                 */
                if (    EFSetFilePointer(hFile, offCurrent, NULL, FILE_BEGIN)
                    ||  offCurrent > cbSize)
                    return VINF_SUCCESS;
            }

            /*
             * Failed, try restoring the file pointer.
             */
            rc = GetLastError();
            EFSetFilePointer(hFile, offCurrent, NULL, FILE_BEGIN);
        }
        else
            rc = GetLastError();
    }
    else
        rc = GetLastError();

    return RTErrConvertFromWin32(rc);
#endif
    return VERR_NOT_IMPLEMENTED;
}

RTR3DECL(int) EFFileQuerySize(const char * filename ,uint64_t *pcbSize)
{
    struct exfat_node * pnode ;
    if(exfat_lookup(ef,&pnode,filename)==0){

        if(pcbSize){
            *pcbSize = pnode->size;
        }
        exfat_put_node(ef,pnode);
        return VINF_SUCCESS;
    }
    return VERR_INVALID_PARAMETER;
}


RTR3DECL(int) EFFileGetSize(RTFILE hFile, uint64_t *pcbSize)
{
    struct exfat_node * pnode = (struct exfat_node *)hFile;
    *pcbSize = pnode->size;
    return  VINF_SUCCESS;
#if 0
    /*
     * GetFileSize works for most handles.
     */
    ULARGE_INTEGER  Size;
    Size.LowPart = GetFileSize((HANDLE)RTFileToNative(hFile), &Size.HighPart);
    if (Size.LowPart != INVALID_FILE_SIZE)
    {
        *pcbSize = Size.QuadPart;
        return VINF_SUCCESS;
    }
    int rc = RTErrConvertFromWin32(GetLastError());

    /*
     * Could it be a volume or a disk?
     */
    DISK_GEOMETRY   DriveGeo;
    DWORD           cbDriveGeo;
    if (DeviceIoControl((HANDLE)RTFileToNative(hFile),
                        IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
                        &DriveGeo, sizeof(DriveGeo), &cbDriveGeo, NULL))
    {
        if (   DriveGeo.MediaType == FixedMedia
            || DriveGeo.MediaType == RemovableMedia)
        {
            *pcbSize = DriveGeo.Cylinders.QuadPart
                     * DriveGeo.TracksPerCylinder
                     * DriveGeo.SectorsPerTrack
                     * DriveGeo.BytesPerSector;

            GET_LENGTH_INFORMATION  DiskLenInfo;
            DWORD                   Ignored;
            if (DeviceIoControl((HANDLE)RTFileToNative(hFile),
                                IOCTL_DISK_GET_LENGTH_INFO, NULL, 0,
                                &DiskLenInfo, sizeof(DiskLenInfo), &Ignored, (LPOVERLAPPED)NULL))
            {
                /* IOCTL_DISK_GET_LENGTH_INFO is supported -- override cbSize. */
                *pcbSize = DiskLenInfo.Length.QuadPart;
            }
            return VINF_SUCCESS;
        }
    }

    /*
     * Return the GetFileSize result if not a volume/disk.
     */
    return rc;
#endif
}


RTR3DECL(int) EFFileGetMaxSizeEx(RTFILE hFile, PRTFOFF pcbMax)
{
    /** @todo r=bird:
     * We might have to make this code OS version specific... In the worse
     * case, we'll have to try GetVolumeInformationByHandle on vista and fall
     * back on NtQueryVolumeInformationFile(,,,, FileFsAttributeInformation)
     * else where, and check for known file system names. (For LAN shares we'll
     * have to figure out the remote file system.) */
    return VERR_NOT_IMPLEMENTED;
}


RTR3DECL(bool) EFFileIsValid(RTFILE hFile)
{
    if (hFile != NIL_RTFILE)
    {
#if 0        
        DWORD dwType = GetFileType((HANDLE)RTFileToNative(hFile));
        switch (dwType)
        {
            case FILE_TYPE_CHAR:
            case FILE_TYPE_DISK:
            case FILE_TYPE_PIPE:
            case FILE_TYPE_REMOTE:
                return true;

            case FILE_TYPE_UNKNOWN:
                if (GetLastError() == NO_ERROR)
                    return true;
                break;

            default:
                break;
        }
#endif
        struct exfat_node * pnode = (struct exfat_node *)hFile;
        if(pnode->is_unlinked)
            return false;
        return true;
    }
    return false;
}


#define LOW_DWORD(u64) ((DWORD)u64)
#define HIGH_DWORD(u64) (((DWORD *)&u64)[1])

RTR3DECL(int) EFFileLock(RTFILE hFile, unsigned fLock, int64_t offLock, uint64_t cbLock)
{
    return VERR_NOT_IMPLEMENTED;
#if 0
    Assert(offLock >= 0);

    /* Check arguments. */
    if (fLock & ~RTFILE_LOCK_MASK)
    {
        AssertMsgFailed(("Invalid fLock=%08X\n", fLock));
        return VERR_INVALID_PARAMETER;
    }

    /* Prepare flags. */
    Assert(RTFILE_LOCK_WRITE);
    DWORD dwFlags = (fLock & RTFILE_LOCK_WRITE) ? LOCKFILE_EXCLUSIVE_LOCK : 0;
    Assert(RTFILE_LOCK_WAIT);
    if (!(fLock & RTFILE_LOCK_WAIT))
        dwFlags |= LOCKFILE_FAIL_IMMEDIATELY;

    /* Windows structure. */
    OVERLAPPED Overlapped;
    memset(&Overlapped, 0, sizeof(Overlapped));
    Overlapped.Offset = LOW_DWORD(offLock);
    Overlapped.OffsetHigh = HIGH_DWORD(offLock);

    /* Note: according to Microsoft, LockFileEx API call is available starting from NT 3.5 */
    if (LockFileEx((HANDLE)RTFileToNative(hFile), dwFlags, 0, LOW_DWORD(cbLock), HIGH_DWORD(cbLock), &Overlapped))
        return VINF_SUCCESS;

    return RTErrConvertFromWin32(GetLastError());
#endif    
}


RTR3DECL(int) EFFileChangeLock(RTFILE hFile, unsigned fLock, int64_t offLock, uint64_t cbLock)
{
    Assert(offLock >= 0);
    return VERR_NOT_IMPLEMENTED;
#if 0    
    /* Check arguments. */
    if (fLock & ~RTFILE_LOCK_MASK)
    {
        AssertMsgFailed(("Invalid fLock=%08X\n", fLock));
        return VERR_INVALID_PARAMETER;
    }

    /* Remove old lock. */
    int rc = RTFileUnlock(hFile, offLock, cbLock);
    if (RT_FAILURE(rc))
        return rc;

    /* Set new lock. */
    rc = RTFileLock(hFile, fLock, offLock, cbLock);
    if (RT_SUCCESS(rc))
        return rc;

    /* Try to restore old lock. */
    unsigned fLockOld = (fLock & RTFILE_LOCK_WRITE) ? fLock & ~RTFILE_LOCK_WRITE : fLock | RTFILE_LOCK_WRITE;
    rc = RTFileLock(hFile, fLockOld, offLock, cbLock);
    if (RT_SUCCESS(rc))
        return VERR_FILE_LOCK_VIOLATION;
    else
        return VERR_FILE_LOCK_LOST;
#endif        
}


RTR3DECL(int) EFFileUnlock(RTFILE hFile, int64_t offLock, uint64_t cbLock)
{
    Assert(offLock >= 0);
    return VERR_NOT_IMPLEMENTED;
#if 0    
    if (UnlockFile((HANDLE)RTFileToNative(hFile),
                   LOW_DWORD(offLock), HIGH_DWORD(offLock),
                   LOW_DWORD(cbLock), HIGH_DWORD(cbLock)))
        return VINF_SUCCESS;

    return RTErrConvertFromWin32(GetLastError());
#endif    
}



RTR3DECL(int) EFFileQueryInfo(RTFILE hFile, PRTFSOBJINFO pObjInfo, RTFSOBJATTRADD enmAdditionalAttribs)
{
    /*
     * Validate input.
     */
    if (hFile == NIL_RTFILE)
    {
        AssertMsgFailed(("Invalid hFile=%RTfile\n", hFile));
        return VERR_INVALID_PARAMETER;
    }
    return VERR_NOT_IMPLEMENTED;
#if 0    
    if (!pObjInfo)
    {
        AssertMsgFailed(("Invalid pObjInfo=%p\n", pObjInfo));
        return VERR_INVALID_PARAMETER;
    }
    if (    enmAdditionalAttribs < RTFSOBJATTRADD_NOTHING
        ||  enmAdditionalAttribs > RTFSOBJATTRADD_LAST)
    {
        AssertMsgFailed(("Invalid enmAdditionalAttribs=%p\n", enmAdditionalAttribs));
        return VERR_INVALID_PARAMETER;
    }

    /*
     * Query file info.
     */
    BY_HANDLE_FILE_INFORMATION Data;
    if (!GetFileInformationByHandle((HANDLE)RTFileToNative(hFile), &Data))
    {
        DWORD dwErr = GetLastError();
        /* Only return if we *really* don't have a valid handle value,
         * everything else is fine here ... */
        if (dwErr == ERROR_INVALID_HANDLE)
            return RTErrConvertFromWin32(dwErr);
        RT_ZERO(Data);
        Data.dwFileAttributes = RTFS_DOS_NT_DEVICE;
    }

    /*
     * Setup the returned data.
     */
    pObjInfo->cbObject    = ((uint64_t)Data.nFileSizeHigh << 32)
                          |  (uint64_t)Data.nFileSizeLow;
    pObjInfo->cbAllocated = pObjInfo->cbObject;

    Assert(sizeof(uint64_t) == sizeof(Data.ftCreationTime));
    RTTimeSpecSetNtTime(&pObjInfo->BirthTime,         *(uint64_t *)&Data.ftCreationTime);
    RTTimeSpecSetNtTime(&pObjInfo->AccessTime,        *(uint64_t *)&Data.ftLastAccessTime);
    RTTimeSpecSetNtTime(&pObjInfo->ModificationTime,  *(uint64_t *)&Data.ftLastWriteTime);
    pObjInfo->ChangeTime  = pObjInfo->ModificationTime;

    pObjInfo->Attr.fMode  = rtFsModeFromDos((Data.dwFileAttributes << RTFS_DOS_SHIFT) & RTFS_DOS_MASK_NT, "", 0);

    /*
     * Requested attributes (we cannot provide anything actually).
     */
    switch (enmAdditionalAttribs)
    {
        case RTFSOBJATTRADD_NOTHING:
            pObjInfo->Attr.enmAdditional          = RTFSOBJATTRADD_NOTHING;
            break;

        case RTFSOBJATTRADD_UNIX:
            pObjInfo->Attr.enmAdditional          = RTFSOBJATTRADD_UNIX;
            pObjInfo->Attr.u.Unix.uid             = ~0U;
            pObjInfo->Attr.u.Unix.gid             = ~0U;
            pObjInfo->Attr.u.Unix.cHardlinks      = Data.nNumberOfLinks ? Data.nNumberOfLinks : 1;
            pObjInfo->Attr.u.Unix.INodeIdDevice   = 0; /** @todo Use the volume serial number (see GetFileInformationByHandle). */
            pObjInfo->Attr.u.Unix.INodeId         = 0; /** @todo Use the fileid (see GetFileInformationByHandle). */
            pObjInfo->Attr.u.Unix.fFlags          = 0;
            pObjInfo->Attr.u.Unix.GenerationId    = 0;
            pObjInfo->Attr.u.Unix.Device          = 0;
            break;

        case RTFSOBJATTRADD_UNIX_OWNER:
            pObjInfo->Attr.enmAdditional          = RTFSOBJATTRADD_UNIX_OWNER;
            pObjInfo->Attr.u.UnixOwner.uid        = ~0U;
            pObjInfo->Attr.u.UnixOwner.szName[0]  = '\0'; /** @todo return something sensible here. */
            break;

        case RTFSOBJATTRADD_UNIX_GROUP:
            pObjInfo->Attr.enmAdditional          = RTFSOBJATTRADD_UNIX_GROUP;
            pObjInfo->Attr.u.UnixGroup.gid        = ~0U;
            pObjInfo->Attr.u.UnixGroup.szName[0]  = '\0';
            break;

        case RTFSOBJATTRADD_EASIZE:
            pObjInfo->Attr.enmAdditional          = RTFSOBJATTRADD_EASIZE;
            pObjInfo->Attr.u.EASize.cb            = 0;
            break;

        default:
            AssertMsgFailed(("Impossible!\n"));
            return VERR_INTERNAL_ERROR;
    }

    return VINF_SUCCESS;
#endif
}


RTR3DECL(int) EFFileSetTimes(RTFILE hFile, PCRTTIMESPEC pAccessTime, PCRTTIMESPEC pModificationTime,
                             PCRTTIMESPEC pChangeTime, PCRTTIMESPEC pBirthTime)
{

    if (!pAccessTime && !pModificationTime && !pBirthTime)
        return VINF_SUCCESS;    /* NOP */
    return VERR_NOT_IMPLEMENTED;
#if 0    
    FILETIME    CreationTimeFT;
    PFILETIME   pCreationTimeFT = NULL;
    if (pBirthTime)
        pCreationTimeFT = RTTimeSpecGetNtFileTime(pBirthTime, &CreationTimeFT);

    FILETIME    LastAccessTimeFT;
    PFILETIME   pLastAccessTimeFT = NULL;
    if (pAccessTime)
        pLastAccessTimeFT = RTTimeSpecGetNtFileTime(pAccessTime, &LastAccessTimeFT);

    FILETIME    LastWriteTimeFT;
    PFILETIME   pLastWriteTimeFT = NULL;
    if (pModificationTime)
        pLastWriteTimeFT = RTTimeSpecGetNtFileTime(pModificationTime, &LastWriteTimeFT);

    int rc = VINF_SUCCESS;
    if (!SetFileTime((HANDLE)RTFileToNative(hFile), pCreationTimeFT, pLastAccessTimeFT, pLastWriteTimeFT))
    {
        DWORD Err = GetLastError();
        rc = RTErrConvertFromWin32(Err);
        Log(("RTFileSetTimes(%RTfile, %p, %p, %p, %p): SetFileTime failed with lasterr %d (%Rrc)\n",
             hFile, pAccessTime, pModificationTime, pChangeTime, pBirthTime, Err, rc));
    }
    return rc;
#endif    
}


/* This comes from a source file with a different set of system headers (DDK)
 * so it can't be declared in a common header, like internal/file.h.
 */
//extern int efFileNativeSetAttributes(HANDLE FileHandle, ULONG FileAttributes);


RTR3DECL(int) EFFileSetMode(RTFILE hFile, RTFMODE fMode)
{
#if 0
    /*
     * Normalize the mode and call the API.
     */
    fMode = rtFsModeNormalize(fMode, NULL, 0);
    if (!rtFsModeIsValid(fMode))
        return VERR_INVALID_PARAMETER;

    ULONG FileAttributes = (fMode & RTFS_DOS_MASK) >> RTFS_DOS_SHIFT;
    int Err = efFileNativeSetAttributes((HANDLE)hFile, FileAttributes);
    if (Err != ERROR_SUCCESS)
    {
        int rc = RTErrConvertFromWin32(Err);
        Log(("RTFileSetMode(%RTfile, %RTfmode): EFtFileNativeSetAttributes (0x%08X) failed with err %d (%Rrc)\n",
             hFile, fMode, FileAttributes, Err, rc));
        return rc;
    }
#endif
    return VINF_SUCCESS;
}


RTR3DECL(int) EFFileQueryFsSizes(RTFILE hFile, PRTFOFF pcbTotal, RTFOFF *pcbFree,
                                 uint32_t *pcbBlock, uint32_t *pcbSector)
{
    /** @todo implement this using NtQueryVolumeInformationFile(hFile,,,,
     *        FileFsSizeInformation). */
    return VERR_NOT_SUPPORTED;
}


RTR3DECL(int) EFFileDelete(const char *pszFilename)
{
#if 0    
    PRTUTF16 pwszFilename;
    int rc = RTStrToUtf16(pszFilename, &pwszFilename);
    if (RT_SUCCESS(rc))
    {
        if (!DeleteFileW(pwszFilename))
            rc = RTErrConvertFromWin32(GetLastError());
        RTUtf16Free(pwszFilename);
    }
    return rc;
#endif
    struct exfat_node * pnode ;
    if(exfat_lookup(ef,&pnode,pszFilename)==0){
        exfat_unlink(ef,pnode);
        exfat_put_node(ef,pnode);
        return VINF_SUCCESS;
    }
    return VERR_GENERAL_FAILURE;
}


RTDECL(int) EFFileRename(const char *pszSrc, const char *pszDst, unsigned fRename)
{
    /*
     * Validate input.
     */
    AssertMsgReturn(VALID_PTR(pszSrc), ("%p\n", pszSrc), VERR_INVALID_POINTER);
    AssertMsgReturn(VALID_PTR(pszDst), ("%p\n", pszDst), VERR_INVALID_POINTER);
    AssertMsgReturn(!(fRename & ~RTPATHRENAME_FLAGS_REPLACE), ("%#x\n", fRename), VERR_INVALID_PARAMETER);

    /*
     * Hand it on to the worker.
     */
#if 0    
    int rc = rtPathWin32MoveRename(pszSrc, pszDst,
                                   fRename & RTPATHRENAME_FLAGS_REPLACE ? MOVEFILE_REPLACE_EXISTING : 0,
                                   RTFS_TYPE_FILE);
#endif
    int rc = exfat_rename(ef, pszSrc, pszDst);
    LogFlow(("RTFileMove(%p:{%s}, %p:{%s}, %#x): returns %Rrc\n",
             pszSrc, pszSrc, pszDst, pszDst, fRename, rc));
    return rc;

}


RTDECL(int) EFFileMove(const char *pszSrc, const char *pszDst, unsigned fMove)
{
    /*
     * Validate input.
     */
    AssertMsgReturn(VALID_PTR(pszSrc), ("%p\n", pszSrc), VERR_INVALID_POINTER);
    AssertMsgReturn(VALID_PTR(pszDst), ("%p\n", pszDst), VERR_INVALID_POINTER);
    AssertMsgReturn(!(fMove & ~RTFILEMOVE_FLAGS_REPLACE), ("%#x\n", fMove), VERR_INVALID_PARAMETER);
#if 0
    /*
     * Hand it on to the worker.
     */
    int rc = rtPathWin32MoveRename(pszSrc, pszDst,
                                   fMove & RTFILEMOVE_FLAGS_REPLACE
                                   ? MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING
                                   : MOVEFILE_COPY_ALLOWED,
                                   RTFS_TYPE_FILE);

    LogFlow(("RTFileMove(%p:{%s}, %p:{%s}, %#x): returns %Rrc\n",
             pszSrc, pszSrc, pszDst, pszDst, fMove, rc));
    return rc;
#endif
    return VERR_NOT_IMPLEMENTED;
}

