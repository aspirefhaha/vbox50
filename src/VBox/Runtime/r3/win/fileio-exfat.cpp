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

#define USE_TCP 1
#if USE_TCP
#include "exfatserver.h"


#else
extern "C" {
#include "exfat.h"
}
#endif
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
#if USE_TCP
struct st_ThSock {
    DWORD thId;
    SOCKET sock;
};

#define THSOCKMAX   20
 struct st_ThSock thSocks[THSOCKMAX] = {0};
 int curSockNum = -1; // -1 for init value , 0~THSOCKMAX is reasonable
static HANDLE ghMutex ;

#if 0
BOOL APIENTRY DllMain( HMODULE hModule,  
                       DWORD  ul_reason_for_call,  
                       LPVOID lpReserved  
                     )  
{  
    switch( ul_reason_for_call ) {
	case DLL_PROCESS_ATTACH:
		OutputDebugString("exfat DLL_PROCESS_ATTACH\n");
		
		break;
	case DLL_THREAD_ATTACH:
		OutputDebugString("exfat  DLL_THREAD_ATTACH\n");
		
		break;
	case DLL_THREAD_DETACH:
		OutputDebugString("exfat DLL_THREAD_DETACH\n");
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
		
		break;
	case DLL_PROCESS_DETACH:
		{
			OutputDebugString("exfat  DLL_PROCESS_DETACH\n");
			
		}
		break;
	}
	return TRUE; 
}  
#endif
//static SOCKET sockClient = INVALID_SOCKET;
static SOCKET init_exfatfs(void)
{
    DWORD curID = GetCurrentThreadId();
    SOCKET curSock = INVALID_SOCKET;
    if((getenv("FHAHADEBUG") != NULL && strcmp(getenv("FHAHADEBUG"),"1")==0)){
        return curSock;
    }
    if(curSockNum==-1){
      ghMutex  = CreateMutex(NULL,FALSE,NULL);
      for(int i = 0;i < THSOCKMAX;i++){
          thSocks[i].sock = INVALID_SOCKET;
          thSocks[i].thId = 0;
      }
      curSockNum = 0;
    }

    for ( int i = 0 ;i<THSOCKMAX;i++){
        if(thSocks[i].thId == curID && thSocks[i].sock!= INVALID_SOCKET){
            curSock = thSocks[i].sock;
            break;
        }
    }
    if(curSock == INVALID_SOCKET){
		WORD wVerisonRequested;
		WSADATA wsaData;
		int err;
		wVerisonRequested = MAKEWORD(1, 1);
		err = WSAStartup(wVerisonRequested, &wsaData);
		if (err != 0)
		{
			return INVALID_SOCKET;
		}
		curSock = socket(AF_INET, SOCK_STREAM, 0);

		// connect server socket
		SOCKADDR_IN addrServer;
		addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(EXFATSERVERPORT);
		if(connect(curSock, (SOCKADDR *)&addrServer, sizeof(addrServer))==SOCKET_ERROR){
			curSock = INVALID_SOCKET;
            Assert(0);
		}
        else{
            int i = 0;
            for(i = 0;i < THSOCKMAX;i++){
                if(thSocks[i].sock == INVALID_SOCKET){
                    thSocks[i].thId = curID;
                    thSocks[i].sock = curSock;
					break;
                }
                  
            }
            Assert(i<THSOCKMAX);
        }
        
    }
    return curSock;
    
}

#define EXFAT_CHECK(kk)    SOCKET sockClient = init_exfatfs();if(sockClient==INVALID_SOCKET)return kk
#else
static struct exfat * ef = NULL;

static void init_exfatfs(void)
{
    if(ef==NULL){
        ef = get_ef();
        
    }
}

#define EXFAT_CHECK(kk)    do{init_exfatfs();if(ef==NULL)return kk;}while(0)
#endif
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
    return false;
}


RTR3DECL(int) EFFileFromNative(PRTFILE pFile, RTHCINTPTR uNative)
{
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

    struct exfat_node* node = NULL;
    char tmpname[EXFAT_UTF8_NAME_BUFFER_MAX] = {0};
    char localfilename[EXFAT_UTF8_NAME_BUFFER_MAX]={0};
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
 #if USE_TCP
    rc = VERR_GENERAL_FAILURE;
    do{
        char cmddata[TEC_EFFILEOPEN]={0};
        cmddata[0]= TECMD_EFFileOpen;
        memcpy(&cmddata[1],&dwCreationDisposition,sizeof(dwCreationDisposition));
        memcpy(&cmddata[5] ,&localfilename,sizeof(localfilename));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILEOPEN , 0);

        if(sendret != TEC_EFFILEOPEN)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILEOPEN]={0};        
        while(tmpRecvLen < TEA_EFFILEOPEN && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEOPEN-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileOpen)
            break;
        rc = recvdata[1] ;
        memcpy(pFile,&recvdata[2],sizeof(RTFILE));
    }while(0);
 #else   
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
    return VERR_NOT_IMPLEMENTED;
}


RTR3DECL(int) EFFileClose(RTFILE hFile)
{
    if (hFile == NIL_RTFILE)
        return VINF_SUCCESS;
#if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_GENERAL_FAILURE;
    do{
        char cmddata[TEC_EFFILECLOSE]={0};
        cmddata[0]= TECMD_EFFileClose;
        memcpy(&cmddata[1],&hFile,sizeof(RTFILE));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILECLOSE , 0);

        if(sendret != TEC_EFFILECLOSE)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILECLOSE]={0};        
        while(tmpRecvLen < TEA_EFFILECLOSE && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILECLOSE-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileClose)
            break;
        rc = recvdata[1] ;
    }while(0);
 #else   
    int rc = VINF_SUCCESS;    
    struct exfat_node * pnode = (struct exfat_node *)hFile;
	exfat_flush_node(ef,pnode);
    exfat_put_node(ef,pnode);
    pnode->curpos = 0;
 #endif   
    return rc;
}


RTFILE efFileGetStandard(RTHANDLESTD enmStdHandle)
{

    return NIL_RTFILE;
}

RTR3DECL(int) EFPathQueryInfoEx(const char *pszPath, PRTFSOBJINFO pObjInfo, RTFSOBJATTRADD enmAdditionalAttribs, uint32_t fFlags)
{
    /*
     * Validate input.
     */
    AssertPtrReturn(pszPath, VERR_INVALID_POINTER);
    AssertReturn(*pszPath, VERR_INVALID_PARAMETER);
    AssertPtrReturn(pObjInfo, VERR_INVALID_POINTER);
    AssertMsgReturn(    enmAdditionalAttribs >= RTFSOBJATTRADD_NOTHING
                    &&  enmAdditionalAttribs <= RTFSOBJATTRADD_LAST,
                    ("Invalid enmAdditionalAttribs=%p\n", enmAdditionalAttribs),
                    VERR_INVALID_PARAMETER);
    AssertMsgReturn(RTPATH_F_IS_VALID(fFlags, 0), ("%#x\n", fFlags), VERR_INVALID_PARAMETER);
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int vrc = VERR_GENERAL_FAILURE;
    do{
        char cmddata[TEC_EFPATHQUERYINFOEX]={0};
        memset(cmddata,0,TEC_EFPATHQUERYINFOEX);
        cmddata[0]= TECMD_EFPathQueryInfoEx;
        strcpy(&cmddata[1],pszPath);
        
        int sendret = send(sockClient, cmddata, TEC_EFPATHQUERYINFOEX , 0);

        if(sendret != TEC_EFPATHQUERYINFOEX)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFPATHQUERYINFOEX]={0};        
        while(tmpRecvLen < TEA_EFPATHQUERYINFOEX && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFPATHQUERYINFOEX-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFPathQueryInfoEx)
            break;
        vrc = VINF_SUCCESS ;
        memcpy(pObjInfo,&recvdata[1],sizeof(EFFSOBJINFO));
        pObjInfo->ChangeTime  = pObjInfo->ModificationTime;

        pObjInfo->Attr.fMode  = RTFS_TYPE_FILE;
        pObjInfo->Attr.enmAdditional = RTFSOBJATTRADD_NOTHING;
    }while(0);
 #else
    /*
     * Query file info.
     */
    struct exfat_node * pnode ;
    int vrc = exfat_lookup(ef,&pnode,pszPath);
    if(vrc != 0){
        return VERR_INVALID_PARAMETER;
    }
    else{
        vrc = VINF_SUCESS;
    }
    pObjInfo->cbObject    = pnode->size;
    pObjInfo->cbAllocated = ROUND_UP(pnode->size, CLUSTER_SIZE(*ef->sb)) ;
    pObjInfo->AccessTime.i64NanosecondsRelativeToUnixEpoch  = pnode->atime;
    pObjInfo->BirthTime.i64NanosecondsRelativeToUnixEpoch   = pnode->mtime;
    pObjInfo->ModificationTime.i64NanosecondsRelativeToUnixEpoch = pnode->mtime;
    exfat_put_node(ef,pnode);
    
    pObjInfo->ChangeTime  = pObjInfo->ModificationTime;

    pObjInfo->Attr.fMode  = RTFS_TYPE_FILE;
    pObjInfo->Attr.enmAdditional = RTFSOBJATTRADD_NOTHING;
    
#endif
    return vrc;
}

RTR3DECL(int) EFPathQueryInfo(const char *pszPath, PRTFSOBJINFO pObjInfo, RTFSOBJATTRADD enmAdditionalAttribs)
{
    return RTPathQueryInfoEx(pszPath, pObjInfo, enmAdditionalAttribs, RTPATH_F_ON_LINK);
}

RTR3DECL(int) EFFsQuerySizes(const char *pszFsPath, RTFOFF *pcbTotal, RTFOFF *pcbFree,
                             uint32_t *pcbBlock, uint32_t *pcbSector)
{
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_GENERAL_FAILURE;
    do{
        char cmddata[TEC_EFFSQUERYSIZES]={0};
        cmddata[0]= TECMD_EFFsQuerySizes;
        
        int sendret = send(sockClient, cmddata, TEC_EFFSQUERYSIZES , 0);

        if(sendret != TEC_EFFSQUERYSIZES)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFSQUERYSIZES]={0};        
        while(tmpRecvLen < TEA_EFFSQUERYSIZES && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFSQUERYSIZES-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFsQuerySizes)
            break;
        rc = VINF_SUCCESS ;
        if(pcbTotal){
            memcpy(pcbTotal,&recvdata[1],sizeof(RTFOFF));
        }
        if(pcbFree){
            memcpy(pcbFree,&recvdata[9],sizeof(RTFOFF));
        }
        if(pcbBlock){
            memcpy(pcbBlock,&recvdata[17],sizeof(uint32_t));
        }
        if(pcbSector){
            memcpy(pcbSector,&recvdata[21],sizeof(uint32_t));
        }  
    }while(0);
 #else    
    /*
     * Validate & get valid root path.
     */
    AssertMsgReturn(VALID_PTR(pszFsPath) && *pszFsPath, ("%p", pszFsPath), VERR_INVALID_PARAMETER);
    int rc = VINF_SUCCESS;
    if(pcbTotal){
        *pcbTotal = ef->dev->size;
    }
    if(pcbFree){
        *pcbFree = (100 - ef->sb->allocated_percent) * ef->dev->size;
    }
    if(pcbBlock){   // cluster as block
        *pcbBlock = 1 << (ef->sb->sector_bits+ ef->sb->spc_bits);
    }
    if(pcbSector){
        *pcbSector = 1 << ef->sb->sector_bits;
    }
    return rc;
#endif
    return VERR_NOT_IMPLEMENTED;    
}


RTR3DECL(int) EFFileSeek(RTFILE hFile, int64_t offSeek, unsigned uMethod, uint64_t *poffActual)
{
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_GENERAL_FAILURE;
    do{
        char cmddata[TEC_EFFILESEEK]={0};
        cmddata[0]= TECMD_EFFileSeek;
        memcpy(&cmddata[1],&hFile,sizeof(RTFILE));
        memcpy(&cmddata[9],&offSeek,sizeof(int64_t));
        memcpy(&cmddata[17],&uMethod,sizeof(unsigned));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILESEEK , 0);

        if(sendret != TEC_EFFILESEEK)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILESEEK]={0};        
        while(tmpRecvLen < TEA_EFFILESEEK && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILESEEK-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileSeek)
            break;
        rc = VINF_SUCCESS ;
        
        if(poffActual){
            memcpy(poffActual,&recvdata[1],sizeof(uint64_t));
        }
                    
    }while(0);
 #else
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
#endif    
    return VINF_SUCCESS;
}


RTR3DECL(int) EFFileRead(RTFILE hFile, void *pvBuf, size_t cbToRead, size_t *pcbRead)
{
	
    if (cbToRead <= 0)
        return VINF_SUCCESS;
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_GENERAL_FAILURE;
    do{
        char cmddata[TEC_EFFILEREAD]={0};
        cmddata[0]= TECMD_EFFileRead;
        memcpy(&cmddata[1],&hFile,sizeof(RTFILE));
        memcpy(&cmddata[9],&cbToRead,sizeof(cbToRead));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILEREAD , 0);

        if(sendret != TEC_EFFILEREAD)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILEREAD]={0};        
        while(tmpRecvLen < TEA_EFFILEREAD && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEREAD-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileRead)
            break;
        size_t tmpCount = 0;
        memcpy(&tmpCount,&recvdata[1],sizeof(size_t));
        Assert(tmpCount<0x100000000LL);
        int needCount = (int)tmpCount;
        curLen = 1;
		tmpRecvLen = 0;
        while(tmpRecvLen < needCount && curLen >0){
            char * tmppos = (char *)pvBuf;
            tmppos += tmpRecvLen;
            curLen =  recv(sockClient,tmppos,needCount-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        rc = VINF_SUCCESS ;
        
        if(pcbRead){
            *pcbRead = tmpCount;
        }
                    
    }while(0);
 #else    
    ULONG cbToReadAdj = (ULONG)cbToRead;
    AssertReturn(cbToReadAdj == cbToRead, VERR_NUMBER_TOO_BIG);

    ULONG cbRead = 0;

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
    
#endif
    return VINF_SUCCESS;
}

/**
 * Write bytes to a file at a given offset.
 * This function may modify the file position.
 *
 * @returns iprt status code.
 * @param   File        Handle to the file.
 * @param   off         Where to write.
 * @param   pvBuf       What to write.
 * @param   cbToWrite   How much to write.
 * @param   *pcbWritten How much we actually wrote.
 *                      If NULL an error will be returned for a partial write.
 */
RTR3DECL(int)  EFFileWriteAt(RTFILE File, RTFOFF off, const void *pvBuf, size_t cbToWrite, size_t *pcbWritten)
{
    int rc = EFFileSeek(File, off, RTFILE_SEEK_BEGIN, NULL);
    if (RT_SUCCESS(rc))
        rc = EFFileWrite(File, pvBuf, cbToWrite, pcbWritten);
    return rc; 
}

/**
 * Read bytes from a file at a given offset.
 * This function may modify the file position.
 *
 * @returns iprt status code.
 * @param   File        Handle to the file.
 * @param   off         Where to read.
 * @param   pvBuf       Where to put the bytes we read.
 * @param   cbToRead    How much to read.
 * @param   *pcbRead    How much we actually read.
 *                      If NULL an error will be returned for a partial read.
 */
RTR3DECL(int)  EFFileReadAt(RTFILE File, RTFOFF off, void *pvBuf, size_t cbToRead, size_t *pcbRead)
{
 
    int rc = EFFileSeek(File, off, RTFILE_SEEK_BEGIN, NULL);
    if (RT_SUCCESS(rc))
        rc = EFFileRead(File, pvBuf, cbToRead, pcbRead);
    return rc;
}


RTR3DECL(int) EFFileWrite(RTFILE hFile, const void *pvBuf, size_t cbToWrite, size_t *pcbWritten)
{
    if (cbToWrite <= 0)
        return VINF_SUCCESS;
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    char cmdParams[TEC_EFFILEWRITE] ={0};
    cmdParams[0] = TECMD_EFFileWrite;
    memcpy(&cmdParams[1],&hFile,sizeof(RTFILE));
    memcpy(&cmdParams[9],&cbToWrite,sizeof(size_t));
    
    int sendret = send(sockClient,cmdParams,TEC_EFFILEWRITE,0);
    if(sendret != TEC_EFFILEWRITE)
        return VERR_GENERAL_FAILURE;
    Assert(cbToWrite < 0x100000000LL);
    int needToWrite = (int)cbToWrite;
    sendret = send(sockClient,(char *)pvBuf,needToWrite,0);
    if(sendret != cbToWrite)
        return VERR_GENERAL_FAILURE;

    
    int tmpRecvLen = 0;
    char recvdata[TEA_EFFILEWRITE] = {0};
    int curLen = 1;
    while(tmpRecvLen < TEA_EFFILEWRITE && curLen >0){
        curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEWRITE-tmpRecvLen,0);
        tmpRecvLen += curLen;
    }
    if(recvdata[0]!= TECMD_EFFileWrite)
        return VERR_INVALID_PARAMETER;
    
    size_t writtencount = 0;
    memcpy(&writtencount,&recvdata[1],sizeof(size_t));
    if(pcbWritten){
        *pcbWritten = writtencount;
    }
            
 #else        
    ULONG cbToWriteAdj = (ULONG)cbToWrite;
    AssertReturn(cbToWriteAdj == cbToWrite, VERR_NUMBER_TOO_BIG);
    struct exfat_node * pnode = (struct exfat_node *)hFile;

    size_t retsize = exfat_generic_pwrite(ef,pnode,pvBuf,cbToWrite,pnode->curpos);
    pnode->curpos += retsize;
    if(pcbWritten)
        *pcbWritten = retsize;
#endif        
    return VINF_SUCCESS;
}


RTR3DECL(int) EFFileFlush(RTFILE hFile)
{
#if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_INVALID_HANDLE;
    do{
        char cmddata[TEC_EFFILEFLUSH]={0};
        cmddata[0]= TECMD_EFFileFlush;
        memcpy(&cmddata[1],&hFile,sizeof(RTFILE));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILEFLUSH , 0);

        if(sendret != TEC_EFFILEFLUSH)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILEFLUSH]={0};        
        while(tmpRecvLen < TEA_EFFILEFLUSH && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEFLUSH-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileFlush)
            break;
        rc = VINF_SUCCESS ;
                    
    }while(0);
 #else
    struct exfat_node * pnode = (struct exfat_node*)hFile;
    int rc = exfat_flush_node(ef,pnode);
    if(rc == 0){
        return VINF_SUCCESS;
    }
#endif    
    return rc;
}


RTR3DECL(int) EFFileSetSize(RTFILE hFile, uint64_t cbSize)
{

    return VERR_NOT_IMPLEMENTED;
}

RTR3DECL(int) EFFileQuerySize(const char * filename ,uint64_t *pcbSize)
{
 #if USE_TCP
    RTFSOBJINFO ObjectInfo;
    int vrc = EFPathQueryInfoEx(filename,&ObjectInfo,RTFSOBJATTRADD_NOTHING,0);
    if(vrc != 0)
        return vrc;
    if(pcbSize){
        *pcbSize = ObjectInfo.cbObject;
    }
    return VINF_SUCCESS;
 #else
    struct exfat_node * pnode ;
    if(exfat_lookup(ef,&pnode,filename)==0){

        if(pcbSize){
            *pcbSize = pnode->size;
        }
        exfat_put_node(ef,pnode);
        return VINF_SUCCESS;
    }
#endif
    return VERR_INVALID_PARAMETER;
}


RTR3DECL(int) EFFileGetSize(RTFILE hFile, uint64_t *pcbSize)
{
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_INVALID_HANDLE;
    do{
        char cmddata[TEC_EFFILEGETSIZE]={0};
        cmddata[0]= TECMD_EFFileGetSize;
        memcpy(&cmddata[1],&hFile,sizeof(RTFILE));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILEGETSIZE , 0);

        if(sendret != TEC_EFFILEGETSIZE)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILEGETSIZE]={0};        
        while(tmpRecvLen < TEA_EFFILEGETSIZE && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEGETSIZE-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileGetSize)
            break;
        uint64_t filesize = 0;
        memcpy(&filesize,&recvdata[1],sizeof(uint64_t));
        if(pcbSize){
            *pcbSize = filesize;
        }
        rc = VINF_SUCCESS ;
                    
    }while(0);
 #else
    struct exfat_node * pnode = (struct exfat_node *)hFile;
    *pcbSize = pnode->size;
#endif
    return  VINF_SUCCESS;
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
 #if USE_TCP
    EXFAT_CHECK(false) ;
    int rc = VERR_INVALID_HANDLE;
    do{
        char cmddata[TEC_EFFILEISVALID]={0};
        cmddata[0]= TECMD_EFFileIsValid;
        memcpy(&cmddata[1],&hFile,sizeof(RTFILE));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILEISVALID , 0);

        if(sendret != TEC_EFFILEISVALID)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILEISVALID]={0};        
        while(tmpRecvLen < TEA_EFFILEISVALID && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEISVALID-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileIsValid)
            break;
        int retvalue = 0;
        memcpy(&retvalue,&recvdata[1],sizeof(int));
        return (retvalue!=0);
                    
    }while(0);
 #else        
        struct exfat_node * pnode = (struct exfat_node *)hFile;
        if(pnode->is_unlinked)
            return false;
#endif            
        return true;
    }
    return false;
}


#define LOW_DWORD(u64) ((DWORD)u64)
#define HIGH_DWORD(u64) (((DWORD *)&u64)[1])

RTR3DECL(int) EFFileLock(RTFILE hFile, unsigned fLock, int64_t offLock, uint64_t cbLock)
{
    return VERR_NOT_IMPLEMENTED;
}


RTR3DECL(int) EFFileChangeLock(RTFILE hFile, unsigned fLock, int64_t offLock, uint64_t cbLock)
{
    Assert(offLock >= 0);
    return VERR_NOT_IMPLEMENTED;
}


RTR3DECL(int) EFFileUnlock(RTFILE hFile, int64_t offLock, uint64_t cbLock)
{
    Assert(offLock >= 0);
    return VERR_NOT_IMPLEMENTED;
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
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_INVALID_HANDLE;
    do{
        char cmddata[TEC_EFFILEQUERYINFO]={0};
        cmddata[0]= TECMD_EFFileQueryInfo;
        memcpy(&cmddata[1],&hFile,sizeof(RTFILE));
        
        int sendret = send(sockClient, cmddata, TEC_EFFILEQUERYINFO , 0);

        if(sendret != TEC_EFFILEQUERYINFO)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILEQUERYINFO]={0};        
        while(tmpRecvLen < TEA_EFFILEQUERYINFO && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEQUERYINFO-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileQueryInfo)
            break;
        if(pObjInfo){

            memcpy(pObjInfo,&recvdata[1],sizeof(EFFSOBJINFO));
            pObjInfo->ChangeTime  = pObjInfo->ModificationTime;

            pObjInfo->Attr.fMode  = RTFS_TYPE_FILE;
            pObjInfo->Attr.enmAdditional = RTFSOBJATTRADD_NOTHING;
        }
        return VINF_SUCCESS;
                    
    }while(0);
 #else    
    struct exfat_node * pnode = (struct exfat_node *)hFile;
    pObjInfo->cbObject    = pnode->size;
    pObjInfo->cbAllocated = ROUND_UP(pnode->size, CLUSTER_SIZE(*ef->sb)) ;
    pObjInfo->AccessTime.i64NanosecondsRelativeToUnixEpoch  = pnode->atime;
    pObjInfo->BirthTime.i64NanosecondsRelativeToUnixEpoch   = pnode->mtime;
    pObjInfo->ModificationTime.i64NanosecondsRelativeToUnixEpoch = pnode->mtime;
    
    pObjInfo->ChangeTime  = pObjInfo->ModificationTime;

    pObjInfo->Attr.fMode  = RTFS_TYPE_FILE;
    pObjInfo->Attr.enmAdditional = RTFSOBJATTRADD_NOTHING;
    return VINF_SUCCESS;
#endif
    return VERR_INVALID_PARAMETER;

}


RTR3DECL(int) EFFileSetTimes(RTFILE hFile, PCRTTIMESPEC pAccessTime, PCRTTIMESPEC pModificationTime,
                             PCRTTIMESPEC pChangeTime, PCRTTIMESPEC pBirthTime)
{

    if (!pAccessTime && !pModificationTime && !pBirthTime)
        return VINF_SUCCESS;    /* NOP */
    return VERR_NOT_IMPLEMENTED;   
}


/* This comes from a source file with a different set of system headers (DDK)
 * so it can't be declared in a common header, like internal/file.h.
 */
//extern int efFileNativeSetAttributes(HANDLE FileHandle, ULONG FileAttributes);


RTR3DECL(int) EFFileSetMode(RTFILE hFile, RTFMODE fMode)
{

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
 #if USE_TCP
    EXFAT_CHECK(VERR_GENERAL_FAILURE) ;
    int rc = VERR_GENERAL_FAILURE;
    do{
        char cmddata[TEC_EFFILEDELETE]={0};
        cmddata[0]= TECMD_EFFileDelete;
        strcpy(&cmddata[1] ,pszFilename);
        
        int sendret = send(sockClient, cmddata, TEC_EFFILEDELETE , 0);

        if(sendret != TEC_EFFILEDELETE)
            break;
        int tmpRecvLen = 0;
        int curLen = 1;
        char recvdata[TEA_EFFILEDELETE]={0};        
        while(tmpRecvLen < TEA_EFFILEDELETE && curLen >0){
            curLen =  recv(sockClient,recvdata+tmpRecvLen,TEA_EFFILEDELETE-tmpRecvLen,0);
            tmpRecvLen += curLen;
        }
        if(recvdata[0]!= TECMD_EFFileDelete)
            break;
        memcpy(&rc,&recvdata[1],sizeof(rc));
        return rc;
    }while(0);
 #else    
    struct exfat_node * pnode ;
    if(exfat_lookup(ef,&pnode,pszFilename)==0){
        exfat_unlink(ef,pnode);
        exfat_cleanup_node(ef,pnode);
        exfat_put_node(ef,pnode);
        return VINF_SUCCESS;
    }
#endif
    return VERR_NOT_IMPLEMENTED;
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
    //int rc = exfat_rename(ef, pszSrc, pszDst);
    //LogFlow(("RTFileMove(%p:{%s}, %p:{%s}, %#x): returns %Rrc\n",
    //         pszSrc, pszSrc, pszDst, pszDst, fRename, rc));
    return VERR_NOT_IMPLEMENTED;

}


RTDECL(int) EFFileMove(const char *pszSrc, const char *pszDst, unsigned fMove)
{
    /*
     * Validate input.
     */
    AssertMsgReturn(VALID_PTR(pszSrc), ("%p\n", pszSrc), VERR_INVALID_POINTER);
    AssertMsgReturn(VALID_PTR(pszDst), ("%p\n", pszDst), VERR_INVALID_POINTER);
    AssertMsgReturn(!(fMove & ~RTFILEMOVE_FLAGS_REPLACE), ("%#x\n", fMove), VERR_INVALID_PARAMETER);

    return VERR_NOT_IMPLEMENTED;
}

