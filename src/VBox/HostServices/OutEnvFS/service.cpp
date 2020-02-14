/* $Id: service.cpp $ */
/** @file
 * Out of SafeEnv FileSystem Service.
 */

/*
 * Copyright (C) 2011-2015 
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

/** @page pg_svc_guest_control   Guest Control HGCM Service
 *
 * This service acts as a proxy for handling and buffering host command requests
 * and clients on the guest. It tries to be as transparent as possible to let
 * the guest (client) and host side do their protocol handling as desired.
 *
 * The following terms are used:
 * - Host:   A host process (e.g. VBoxManage or another tool utilizing the Main API)
 *           which wants to control something on the guest.
 * - Client: A client (e.g. VBoxService) running inside the guest OS waiting for
 *           new host commands to perform. There can be multiple clients connected
 *           to a service. A client is represented by its HGCM client ID.
 * - Context ID: An (almost) unique ID automatically generated on the host (Main API)
 *               to not only distinguish clients but individual requests. Because
 *               the host does not know anything about connected clients it needs
 *               an indicator which it can refer to later. This context ID gets
 *               internally bound by the service to a client which actually processes
 *               the command in order to have a relationship between client<->context ID(s).
 *
 * The host can trigger commands which get buffered by the service (with full HGCM
 * parameter info). As soon as a client connects (or is ready to do some new work)
 * it gets a buffered host command to process it. This command then will be immediately
 * removed from the command list. If there are ready clients but no new commands to be
 * processed, these clients will be set into a deferred state (that is being blocked
 * to return until a new command is available).
 *
 * If a client needs to inform the host that something happened, it can send a
 * message to a low level HGCM callback registered in Main. This callback contains
 * the actual data as well as the context ID to let the host do the next necessary
 * steps for this context. This context ID makes it possible to wait for an event
 * inside the host's Main API function (like starting a process on the guest and
 * wait for getting its PID returned by the client) as well as cancelling blocking
 * host calls in order the client terminated/crashed (HGCM detects disconnected
 * clients and reports it to this service's callback).
 */

/******************************************************************************
 *   Header Files                                                             *
 ******************************************************************************/
#ifdef LOG_GROUP
 #undef LOG_GROUP
#endif
#define LOG_GROUP LOG_GROUP_GUEST_DND

#include <map>
//#include <VBox/GuestHost/DragAndDrop.h>
#include <VBox/HostServices/Service.h>
#include <VBox/HostServices/OutEnvFSSVC.h>

#include <shlwapi.h>
#include <shlobj.h>

//#include "dndmanager.h"

/******************************************************************************
 *   Service class declaration                                                *
 ******************************************************************************/

/** Map holding pointers to HGCM clients. Key is the (unique) HGCM client ID. */
typedef std::map<uint32_t, HGCM::Client*> OFSClientMap;

/**
 * Specialized drag & drop service class.
 */
class OutEnvFSService : public HGCM::AbstractService<OutEnvFSService>
{
public:

    explicit OutEnvFSService(PVBOXHGCMSVCHELPERS pHelpers)
        : HGCM::AbstractService<OutEnvFSService>(pHelpers)
         {}

protected:

    int  init(VBOXHGCMSVCFNTABLE *pTable);
    int  uninit(void);
    int  clientConnect(uint32_t u32ClientID, void *pvClient);
    int  clientDisconnect(uint32_t u32ClientID, void *pvClient);
    void guestCall(VBOXHGCMCALLHANDLE callHandle, uint32_t u32ClientID, void *pvClient, uint32_t u32Function, uint32_t cParms, VBOXHGCMSVCPARM paParms[]);
    int  hostCall(uint32_t u32Function, uint32_t cParms, VBOXHGCMSVCPARM paParms[]);
#if 0
    int modeSet(uint32_t u32Mode);
    inline uint32_t modeGet() { return m_u32Mode; };
#endif

protected:

    static DECLCALLBACK(int) progressCallback(uint32_t uStatus, uint32_t uPercentage, int rc, void *pvUser);

protected:

    /** Map of all connected clients. */
    OFSClientMap            m_clientMap;
    /** List of all clients which are queued up (deferred return) and ready
     *  to process new commands. */
#if 0  
    RTCList<HGCM::Client*>  m_clientQueue;  
    uint32_t                m_u32Mode;
#endif
};

/******************************************************************************
 *   Service class implementation                                             *
 ******************************************************************************/

int OutEnvFSService::init(VBOXHGCMSVCFNTABLE *pTable)
{
    /* Register functions. */
    pTable->pfnHostCall          = svcHostCall;
    pTable->pfnSaveState         = NULL;  /* The service is stateless, so the normal */
    pTable->pfnLoadState         = NULL;  /* construction done before restoring suffices */
    pTable->pfnRegisterExtension = svcRegisterExtension;
#if 0
    /* Drag'n drop mode is disabled by default. */
    modeSet(VBOX_DRAG_AND_DROP_MODE_OFF);
#endif
    int rc = VINF_SUCCESS;
#if 0
    try
    {
        m_pManager = new DnDManager(&OutEnvFSService::progressCallback, this);
    }
    catch(std::bad_alloc &)
    {
        rc = VERR_NO_MEMORY;
    }
#endif
    LogFlowFuncLeaveRC(rc);
    return rc;
}

int OutEnvFSService::uninit(void)
{
#if 0    
    if (m_pManager)
    {
        delete m_pManager;
        m_pManager = NULL;
    }
#endif
    return VINF_SUCCESS;
}

int OutEnvFSService::clientConnect(uint32_t u32ClientID, void *pvClient)
{
    if (m_clientMap.size() >= UINT8_MAX) /* Don't allow too much clients at the same time. */
    {
        AssertMsgFailed(("Maximum number of clients reached\n"));
        return VERR_BUFFER_OVERFLOW;
    }

    int rc = VINF_SUCCESS;

    /*
     * Add client to our client map.
     */
    if (m_clientMap.find(u32ClientID) != m_clientMap.end())
        rc = VERR_ALREADY_EXISTS;

    if (RT_SUCCESS(rc))
    {
        try
        {
            m_clientMap[u32ClientID] = new HGCM::Client(u32ClientID);
        }
        catch(std::bad_alloc &)
        {
            rc = VERR_NO_MEMORY;
        }

        if (RT_SUCCESS(rc))
        {
            /*
             * Clear the message queue as soon as a new clients connect
             * to ensure that every client has the same state.
             */
#if 0            
            if (m_pManager)
                m_pManager->clear();
#endif                
        }
    }

    LogRel(("Client %RU32 connected, rc=%Rrc\n", u32ClientID, rc));
    return rc;
}

int OutEnvFSService::clientDisconnect(uint32_t u32ClientID, void *pvClient)
{
    /* Client not found? Bail out early. */
    OFSClientMap::iterator itClient =  m_clientMap.find(u32ClientID);
    if (itClient == m_clientMap.end())
        return VERR_NOT_FOUND;
#if 0
    /*
     * Remove from waiters queue.
     */
    for (size_t i = 0; i < m_clientQueue.size(); i++)
    {
        HGCM::Client *pClient = m_clientQueue.at(i);
        if (pClient->clientId() == u32ClientID)
        {
            if (m_pHelpers)
                m_pHelpers->pfnCallComplete(pClient->handle(), VERR_INTERRUPTED);

            m_clientQueue.removeAt(i);
            delete pClient;

            break;
        }
    }
#endif
    /*
     * Remove from client map and deallocate.
     */
    AssertPtr(itClient->second);
    delete itClient->second;

    m_clientMap.erase(itClient);

    LogRel(("Client %RU32 disconnected\n", u32ClientID));
    return VINF_SUCCESS;
}
#if 0
int OutEnvFSService::modeSet(uint32_t u32Mode)
{
    /** @todo Validate mode. */
    switch (u32Mode)
    {
        case VBOX_DRAG_AND_DROP_MODE_OFF:
        case VBOX_DRAG_AND_DROP_MODE_HOST_TO_GUEST:
        case VBOX_DRAG_AND_DROP_MODE_GUEST_TO_HOST:
        case VBOX_DRAG_AND_DROP_MODE_BIDIRECTIONAL:
            m_u32Mode = u32Mode;
            break;

        default:
            m_u32Mode = VBOX_DRAG_AND_DROP_MODE_OFF;
            break;
    }

    return VINF_SUCCESS;
}
#endif
static unsigned int simLastError = 0;
static BOOL GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc)
{ 
	DWORD dwOutBytes = 0;
	STORAGE_PROPERTY_QUERY Query;
	Query.PropertyId = StorageDeviceProperty; 
	Query.QueryType = PropertyStandardQuery;
	
	BOOL ret =  ::DeviceIoControl(
		hDevice,
		IOCTL_STORAGE_QUERY_PROPERTY,
		&Query, 
		sizeof(STORAGE_PROPERTY_QUERY), 
		pDevDesc, 
		pDevDesc->Size,
		&dwOutBytes, 
		(LPOVERLAPPED)NULL); 
    //DebugToFile("GetDisksProperty Ret %d LastError %d ",ret,GetLastError());
    return ret;
} 
static UINT GetDiskType(LPCTSTR lpRootPathName)
{
	char szDrive[32];
	memset(szDrive,0,32);
	sprintf(szDrive, "%s", lpRootPathName);
	UINT format = GetDriveType((LPCTSTR)szDrive);
	//DebugToFile("now first in GetDiskType %s type %d",szDrive,format);
	if(format == DRIVE_FIXED)
	{
		memset(szDrive,0,32);
		sprintf(szDrive, "\\\\?\\%s", lpRootPathName); 
		if(szDrive[strlen(szDrive)-1]=='\\')
			szDrive[strlen(szDrive)-1]=0;
		HANDLE hDevice = CreateFile(szDrive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL); 
		if (hDevice != INVALID_HANDLE_VALUE) 
		{ 
			PSTORAGE_DEVICE_DESCRIPTOR pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1];
			pDevDesc->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
			if(GetDisksProperty(hDevice, pDevDesc)) 
			{ 
				//DebugToFile("now middle  in GetBusType %s type %d",szDrive,pDevDesc->BusType);
				if(pDevDesc->BusType == BusTypeUsb) 
				{ 
					format = DRIVE_REMOVABLE;
					//DebugToFile("now last return %d",format);
				} 
			} 
			delete pDevDesc; 
			CloseHandle(hDevice);
		}
		else
		{
			//DebugToFile("CreateFile %s failed",szDrive);
			format = DRIVE_UNKNOWN;
		}
	}

	return format;
}
void OutEnvFSService::guestCall(VBOXHGCMCALLHANDLE callHandle, uint32_t u32ClientID,
                                   void *pvClient, uint32_t u32Function,
                                   uint32_t cParms, VBOXHGCMSVCPARM paParms[])
{
    LogRel(("u32ClientID=%RU32, u32Function=%RU32, cParms=%RU32\n",
                 u32ClientID, u32Function, cParms));

    /* Check if we've the right mode set. */
    int rc = VERR_ACCESS_DENIED; /* Play safe. */
    switch (u32Function)
    {
        case GDLSIM_FN_GETLOGICALDRIVES:
		{
			LogRel(("svcCall: GDLSIM_FN_GETLOGICALDRIVES \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETLOGICALDRIVES)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if ( paParms[0].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				DWORD  Out			= GetLogicalDrives();
				paParms[0].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_SETCURRENTDIRECTORY:
		{
			LogRel(("svcCall: GDLSIM_FN_SETCURRENTDIRECTORY \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_SETCURRENTDIRECTORY)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                     || paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				LPCTSTR lpPathName			= (LPCTSTR)paParms[0].u.pointer.addr;
				BOOL  Out			= SetCurrentDirectoryA(lpPathName);
#ifdef DEBUG 
                sprintf(tmpstr,"svcCall: GDLSIM_FN_SETCURRENTDIRECTORY lpPathName is %s,result is %d\n",lpPathName,Out);
                OutputDebugStringA(tmpstr);
#endif
                if(!Out){
                    simLastError = GetLastError();
                }
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETCURRENTDIRECTORY:
		{
			LogRel(("svcCall: GDLSIM_FN_GETCURRENTDIRECTORY \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETCURRENTDIRECTORY)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* return result BOOL */
                     || paParms[1].type !=   VBOX_HGCM_SVC_PARM_32BIT /*. */  
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				DWORD    nBufferLength	= (DWORD)paParms[0].u.pointer.size;
                LPTSTR lpBuffer	= (LPTSTR)paParms[0].u.pointer.addr;
				DWORD  Out			= GetCurrentDirectory(nBufferLength,lpBuffer);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETFILEATTRIBUTESEX:
        {
            LogRel(("svcCall: GDLSIM_FN_GETFILEATTRIBUTESEX  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETFILEATTRIBUTESEX)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* lp name. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* fInfoLevelId */  
					 || paParms[2].type != VBOX_HGCM_SVC_PARM_PTR   /* lpFileInformation ptr */
                     || paParms[3].type != VBOX_HGCM_SVC_PARM_32BIT   /* ret dword */  
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpFileName  = (LPCTSTR)paParms[0].u.pointer.addr;
                GET_FILEEX_INFO_LEVELS fInfoLevelId = (GET_FILEEX_INFO_LEVELS) paParms[1].u.uint32;
                LPVOID  lpFileInformation  = (LPVOID)paParms[2].u.pointer.addr;
				BOOL  Out			= GetFileAttributesEx(lpFileName,fInfoLevelId,lpFileInformation);
                LogRel(("GetFileAttributesEx:  Ret %d\n",Out));
				paParms[3].u.uint32 = (uint32_t)Out;
                if(Out)
                    rc = VINF_SUCCESS;
                else
                    rc = VERR_GENERAL_FAILURE;
            }
            break;
        }
        case GDLSIM_FN_FINDFIRSTFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_FINDFIRSTFILE  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_FINDFIRSTFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
					 || paParms[2].type != VBOX_HGCM_SVC_PARM_64BIT   /* return result HANDLE as uint64 */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpFileName  = (LPCTSTR)paParms[0].u.pointer.addr;
                LPWIN32_FIND_DATA lpFindFileData = (LPWIN32_FIND_DATA) paParms[1].u.pointer.addr;
				HANDLE  Out			= FindFirstFile(lpFileName,lpFindFileData);
                LogRel(("OutEnvFS:: FindFirstFile %s Ret %p\n",lpFileName,Out));
				paParms[2].u.uint64 = (uint64_t)Out;
                if(Out != INVALID_HANDLE_VALUE)
                    rc = VINF_SUCCESS;
                else
                    rc = VERR_GENERAL_FAILURE;
            }
            break;
		}
        case GDLSIM_FN_FINDNEXTFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_FINDNEXTFILE \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_FINDNEXTFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
					 || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				HANDLE    hFindFile	= (HANDLE)paParms[0].u.uint64;
                LPWIN32_FIND_DATA lpFindFileData = (LPWIN32_FIND_DATA)paParms[1].u.pointer.addr;
				BOOL  Out			= FindNextFile(hFindFile,lpFindFileData);
				paParms[2].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_FINDCLOSE:
		{
			LogRel(("svcCall: GDLSIM_FN_FINDCLOSE \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_FINDCLOSE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /*. */  
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				HANDLE    handle	= (HANDLE)paParms[0].u.uint64;
				BOOL  Out			= FindClose(handle);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_CREATEFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_CREATEFILE \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_CREATEFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                     || paParms[0].u.pointer.size == 0
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /*. */  
                     || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT   /*. */  
                     || paParms[3].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                     || paParms[4].type != VBOX_HGCM_SVC_PARM_32BIT   /*. */  
                     || paParms[5].type != VBOX_HGCM_SVC_PARM_32BIT   /*. */  
                     || paParms[6].type != VBOX_HGCM_SVC_PARM_32BIT   /*. */ 
			         || paParms[7].type != VBOX_HGCM_SVC_PARM_64BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpFileName = (LPCTSTR)paParms[0].u.pointer.addr;
                DWORD dwDesiredAccess = (DWORD)paParms[1].u.uint32;
                DWORD dwShareMode = (DWORD)paParms[2].u.uint32;
                LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
                if(paParms[3].u.pointer.size!=0)
                    lpSecurityAttributes = (LPSECURITY_ATTRIBUTES)paParms[3].u.pointer.addr;          
                DWORD dwCreationDisposition  = (DWORD)paParms[4].u.uint32; 
                DWORD dwFlagsAndAttributes = (DWORD)paParms[5].u.uint32;
                HANDLE hTemplateFile = (HANDLE)paParms[6].u.uint32;
				HANDLE  Out			= CreateFile(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
				paParms[7].u.uint64 = (uint64_t)Out;
				if(Out == INVALID_HANDLE_VALUE){
					simLastError = GetLastError();
				}
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_DELETEFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_DELETEFILE \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_DELETEFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* in filename ptr. */
                     || paParms[0].u.pointer.size <= 0
					 || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				LPCTSTR lpFileName = (LPCTSTR)paParms[0].u.pointer.addr;
				BOOL  Out			= DeleteFile(lpFileName);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETFILEINFORMATIONBYHANDLE:
		{
			LogRel(("svcCall: GDLSIM_FN_GETFILEINFORMATIONBYHANDLE \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETFILEINFORMATIONBYHANDLE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
					 || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                HANDLE hFile = (HANDLE)paParms[0].u.uint64 ;
                LPBY_HANDLE_FILE_INFORMATION lpFileInformation = NULL;
                if(paParms[1].u.pointer.size >0)
                    lpFileInformation = (LPBY_HANDLE_FILE_INFORMATION)paParms[1].u.pointer.addr;
				BOOL  Out			= GetFileInformationByHandle(hFile,lpFileInformation);
				paParms[2].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETFILESIZE:
		{
			LogRel(("svcCall:  GDLSIM_FN_GETFILESIZE\n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETFILESIZE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
					 || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				HANDLE    hFile	= (HANDLE)paParms[0].u.uint64;
                LPDWORD lpFileSizeHigh = NULL;
                if(paParms[1].u.pointer.size == sizeof(DWORD))
                    lpFileSizeHigh = (LPDWORD)paParms[1].u.pointer.addr;
			    DWORD  Out			= GetFileSize(hFile,lpFileSizeHigh);
				paParms[2].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETFFILESIZEEX:
		{
			LogRel(("svcCall:  GDLSIM_FN_GETFFILESIZEEX\n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETFFILESIZEEX)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
			         || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                HANDLE hFile  =(HANDLE)paParms[0].u.uint64;
                PLARGE_INTEGER lpFileSize = (PLARGE_INTEGER) paParms[1].u.pointer.addr;
				BOOL  Out			= GetFileSizeEx(hFile,lpFileSize);
				paParms[2].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETTEMPPATH:
		{
			LogRel(("svcCall:  GDLSIM_FN_GETTEMPPATH\n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETTEMPPATH)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /*return result BOOL. */  
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                DWORD nBufferLength = (DWORD)paParms[0].u.pointer.size;
                LPTSTR lpBuffer = (LPTSTR)paParms[0].u.pointer.addr;
				BOOL  Out			= GetTempPath(nBufferLength,lpBuffer);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_MOVEFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_ MOVEFILE \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_MOVEFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                       || paParms[0].u.pointer.size <= 0
                        || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR
                        || paParms[1].u.pointer.size ==0
			    || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				LPCTSTR lpExistingFileName = (LPCTSTR)paParms[0].u.pointer.addr;
                LPCTSTR lpNewFileName = (LPCTSTR)paParms[1].u.pointer.addr;
				BOOL  Out			= MoveFile(lpExistingFileName,lpNewFileName);
				paParms[2].u.uint32 = (uint32_t)Out;
                if(!Out)
                {
                    simLastError = GetLastError();
                }
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_COPYFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_COPYFILE  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_COPYFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                       || paParms[0].u.pointer.size <= 0
                        || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR
                        || paParms[1].u.pointer.size ==0
                        || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT
			    || paParms[3].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				LPCTSTR lpExistingFileName = (LPCTSTR)paParms[0].u.pointer.addr;
                LPCTSTR lpNewFileName = (LPCTSTR)paParms[1].u.pointer.addr;
                BOOL bFailIfExists = (BOOL)paParms[2].u.uint32;
				BOOL  Out			= CopyFile(lpExistingFileName,lpNewFileName,bFailIfExists);
				paParms[3].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_SETFILETIME:
		{
			LogRel(("svcCall: GDLSIM_FN_SETFILETIME  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_SETFILETIME)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                     || paParms[2].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                     || paParms[3].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
					 || paParms[4].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				HANDLE hFile= (HANDLE)paParms[0].u.uint64;
                const FILETIME* lpCreationTime = (const FILETIME*)paParms[1].u.pointer.addr;
                const FILETIME* lpLastAccessTime = (const FILETIME*)paParms[2].u.pointer.addr;
                const FILETIME* lpLastWriteTime = (const FILETIME*)paParms[3].u.pointer.addr;
				BOOL  Out			= SetFileTime(hFile,lpCreationTime,lpLastAccessTime,lpLastWriteTime);
				paParms[4].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_READFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_READFILE  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_READFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                    || paParms[1].u.pointer.size == 0
                    || paParms[2].type != VBOX_HGCM_SVC_PARM_PTR
                    || paParms[2].u.pointer.size != sizeof(DWORD)
                    || paParms[3].type!= VBOX_HGCM_SVC_PARM_PTR
		            || paParms[4].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                HANDLE hFile = (HANDLE)paParms[0].u.uint64;
                LPVOID lpBuffer = (LPVOID)paParms[1].u.pointer.addr;
                DWORD nNumberOfBytesToRead = (DWORD)paParms[1].u.pointer.size;
                LPDWORD lpNumberOfBytesRead = (LPDWORD)paParms[2].u.pointer.addr;
                LPOVERLAPPED lpOverlapped = (LPOVERLAPPED) paParms[3].u.pointer.addr;
				BOOL  Out			= ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
				paParms[4].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_SETENDOFFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_SETENDOFFILE  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_SETENDOFFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
					 || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
				HANDLE    tFile	= (HANDLE)paParms[0].u.uint64;
                BOOL  Out			= SetEndOfFile(tFile);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_SETFILEPOINTER:
		{
			LogRel(("svcCall: GDLSIM_FN_SETFILEPOINTER  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_SETFILEPOINTER)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /*. */  
                     || paParms[2].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                     || paParms[3].type != VBOX_HGCM_SVC_PARM_32BIT /* */
			|| paParms[4].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                HANDLE hFile= (HANDLE)paParms[0].u.uint64;
                LONG lDistanceToMove=(LONG)paParms[1].u.uint32;
                PLONG lpDistanceToMoveHigh = (PLONG) paParms[2].u.pointer.addr;
                DWORD dwMoveMethod = (DWORD)paParms[3].u.uint32;
				DWORD  Out			= SetFilePointer(hFile,lDistanceToMove,lpDistanceToMoveHigh,dwMoveMethod);
				paParms[4].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_WRITEFILE:
		{
			LogRel(("svcCall: GDLSIM_FN_WRITEFILE  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_WRITEFILE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                    || paParms[1].u.pointer.size == 0
                    || paParms[2].type != VBOX_HGCM_SVC_PARM_PTR
                    || paParms[2].u.pointer.size != sizeof(DWORD)
                    || paParms[3].type!= VBOX_HGCM_SVC_PARM_PTR
                    || paParms[4].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                HANDLE hFile = (HANDLE)paParms[0].u.uint64;
                LPCVOID lpBuffer = (LPCVOID)paParms[1].u.pointer.addr;
                DWORD nNumberOfBytesToWrite = (DWORD)paParms[1].u.pointer.size;
                LPDWORD lpNumberOfBytesWriten = (LPDWORD)paParms[2].u.pointer.addr;
                LPOVERLAPPED lpOverlapped = (LPOVERLAPPED) paParms[3].u.pointer.addr;
				BOOL  Out			= WriteFile(hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWriten,lpOverlapped);
				paParms[4].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_FLUSHFILEBUFFERS:
		{
			LogRel(("svcCall: GDLSIM_FN_FLUSHFILEBUFFERS  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_FLUSHFILEBUFFERS)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                    || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                HANDLE hFile = (HANDLE)paParms[0].u.uint64;
				BOOL  Out			= FlushFileBuffers(hFile);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_CLOSEHANDLE:
		{
			LogRel(("svcCall: GDLSIM_FN_CLOSEHANDLE  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_CLOSEHANDLE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_64BIT   /* the handle of device. */
                     || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                HANDLE hFile = (HANDLE)paParms[0].u.uint64;
				BOOL  Out			= CloseHandle(hFile);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_CREATEDIRECTORY:
		{
			LogRel(("svcCall: GDLSIM_FN_CREATEDIRECTORY  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_CREATEDIRECTORY)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /*. */  
                    || paParms[0].u.pointer.size ==0 
                    || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR
			        || paParms[2].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpPathName = (LPCTSTR) paParms[0].u.pointer.addr;
                LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
                if(paParms[1].u.pointer.size != 0)
                    lpSecurityAttributes = (LPSECURITY_ATTRIBUTES)paParms[1].u.pointer.addr;
				BOOL  Out			= CreateDirectory(lpPathName,lpSecurityAttributes);
				paParms[2].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_REMOVEDIRECTORY:
		{
			LogRel(("svcCall: GDLSIM_FN_REMOVEDIRECTORY  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_REMOVEDIRECTORY)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                    || paParms[0].u.pointer.size==0
                    || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpPathName = (LPCTSTR)paParms[0].u.pointer.addr;
				BOOL  Out			= RemoveDirectory(lpPathName);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETDISKFREESPACEEX:
		{
			LogRel(("svcCall: GDLSIM_FN_GETDISKFREESPACEEX  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETDISKFREESPACEEX)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                    || paParms[0].u.pointer.size==0
                    || paParms[1].type != VBOX_HGCM_SVC_PARM_PTR   /*  */
                    || paParms[2].type != VBOX_HGCM_SVC_PARM_PTR   /*  */
                    || paParms[3].type != VBOX_HGCM_SVC_PARM_PTR   /*  */
                    || paParms[4].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpDirectoryName = (LPCTSTR)paParms[0].u.pointer.addr;
                PULARGE_INTEGER lpFreeBytesAvailable = (PULARGE_INTEGER)paParms[1].u.pointer.addr;
                PULARGE_INTEGER lpTotalNumberOfBytes = (PULARGE_INTEGER)paParms[2].u.pointer.addr;
                PULARGE_INTEGER lpTotalNumberOfFreeBytes = (PULARGE_INTEGER)paParms[3].u.pointer.addr;
				BOOL  Out			= GetDiskFreeSpaceEx(lpDirectoryName,lpFreeBytesAvailable,lpTotalNumberOfBytes,lpTotalNumberOfFreeBytes);
				paParms[4].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETDRIVETYPE:
		{
			LogRel(("svcCall: GDLSIM_FN_GETDRIVETYPE  \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETDRIVETYPE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (   paParms[0].type != VBOX_HGCM_SVC_PARM_PTR   /* the handle of device. */
                    || paParms[0].u.pointer.size==0
                    || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpRootPathName = (LPCTSTR)paParms[0].u.pointer.addr;
				UINT   Out			= GetDriveType(lpRootPathName);
				paParms[1].u.uint32 = (uint32_t)Out;
                rc = VINF_SUCCESS;
				//DebugToFile("GetDriveType %s return %d",lpRootPathName,Out);
            }
            break;
		}
        case GDLSIM_FN_GETLASTERROR:
		{
			LogRel(("svcCall: GDLSIM_FN_GETLASTERROR \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETLASTERROR)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (    paParms[0].type != VBOX_HGCM_SVC_PARM_32BIT   /* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                if(simLastError){
                    paParms[0].u.uint32 = simLastError;
                    simLastError = 0;
                }
                else{
				    DWORD  Out			= GetLastError();
				    paParms[0].u.uint32 = (uint32_t)Out;
                }
                rc = VINF_SUCCESS;
            }
            break;
		}
        
        case GDLSIM_FN_GETCURRENTMODULEDIR:
        {
			LogRel(("svcCall: GDLSIM_FN_GETCURRENTMODULEDIR \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETCURRENTMODULEDIR)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (    paParms[0].type != VBOX_HGCM_SVC_PARM_PTR  
                        || paParms[0].u.pointer.addr == NULL 
                        || paParms[0].u.pointer.size <= 0 
                        || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT/* return result  */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPTSTR pBuf = (LPTSTR)paParms[0].u.pointer.addr;
                DWORD nSize = (DWORD)paParms[0].u.pointer.size;
                DWORD ret = GetModuleFileNameA(NULL,pBuf, nSize);
                if(strlen(pBuf)>0 && ret >0)
                {
                    char * pEnd = strrchr(pBuf,'\\');
                    if(pEnd)
                        {
                            *pEnd = 0;
                            ret = (DWORD)strlen(pBuf);
                        }
                }
                paParms[1].u.uint32 = ret;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_PATHFILEEXISTS:
        {
			LogRel(("svcCall: GDLSIM_FN_PATHFILEEXISTS \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_PATHFILEEXISTS)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (    paParms[0].type != VBOX_HGCM_SVC_PARM_PTR  
                        || paParms[0].u.pointer.addr == NULL 
                        || paParms[0].u.pointer.size <= 0 
                        || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT/* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPTSTR pszPath = (LPTSTR)paParms[0].u.pointer.addr;
               BOOL ret = PathFileExistsA(pszPath);
               paParms[1].u.uint32 = (BOOL)ret;
                rc = VINF_SUCCESS;
            }
            break;
		}
        case GDLSIM_FN_GETDISKTYPE:
        {
			LogRel(("svcCall: GDLSIM_FN_GETDISKTYPE \n"));

            /* Verify parameter count and types. */
            if (cParms != GDLSIM_CPARMS_GETDISKTYPE)
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else if (    paParms[0].type != VBOX_HGCM_SVC_PARM_PTR  
                        || paParms[0].u.pointer.addr == NULL 
                        || paParms[0].u.pointer.size <= 0 
                        || paParms[1].type != VBOX_HGCM_SVC_PARM_32BIT/* return result BOOL */
                    )
            {
                rc = VERR_INVALID_PARAMETER;
            }
            else
            {
                /* Fetch parameters. */
                LPCTSTR lpRootPathName = (LPCTSTR)paParms[0].u.pointer.addr;
                UINT ret = GetDiskType(lpRootPathName);
                paParms[1].u.uint32 = (uint32_t)ret;
                rc = VINF_SUCCESS;
                //DebugToFile("GetDiskType %s return %d",lpRootPathName,ret);
            }
            break;
		}   
        default:
        {
#if 0                
            /* All other messages are handled by the DnD manager. */
            rc = m_pManager->nextMessage(u32Function, cParms, paParms);
            if (rc == VERR_NO_DATA) /* Manager has no new messsages? Try asking the host. */
            {
                if (m_pfnHostCallback)
                {
                    OutEnvFSSvc::VBOXDNDCBHGGETNEXTHOSTMSGDATA data;
                    data.hdr.u32Magic = OutEnvFSSvc::CB_MAGIC_DND_HG_GET_NEXT_HOST_MSG_DATA;
                    data.uMsg    = u32Function;
                    data.cParms  = cParms;
                    data.paParms = paParms;

                    rc = m_pfnHostCallback(m_pvHostData, u32Function, &data, sizeof(data));
                    if (RT_SUCCESS(rc))
                    {
                        cParms  = data.cParms;
                        paParms = data.paParms;
                    }
                }
            }
#endif                
            break;
        }
        
    }

    /*
     * If async execution is requested, we didn't notify the guest yet about
     * completion. The client is queued into the waiters list and will be
     * notified as soon as a new event is available.
     */
    if (rc == VINF_HGCM_ASYNC_EXECUTE)
    {
        #if 0
        m_clientQueue.append(new HGCM::Client(u32ClientID, callHandle,
                                              u32Function, cParms, paParms));
        #endif
    }

    if (   rc != VINF_HGCM_ASYNC_EXECUTE
        && m_pHelpers)
    {
        m_pHelpers->pfnCallComplete(callHandle, rc);
    }

    LogRel(("Returning rc=%Rrc\n", rc));
}

int OutEnvFSService::hostCall(uint32_t u32Function,
                                 uint32_t cParms, VBOXHGCMSVCPARM paParms[])
{
#if 0    
    LogRel(("u32Function=%RU32, cParms=%RU32, cClients=%zu, cQueue=%zu\n",
                 u32Function, cParms, m_clientMap.size(), m_clientQueue.size()));
#else
    LogRel(("u32Function=%RU32, cParms=%RU32, cClients=%zu\n",
                 u32Function, cParms, m_clientMap.size()));
#endif
    int rc = VERR_INVALID_PARAMETER;
#if 0    
    if (u32Function == OutEnvFSSvc::HOST_DND_SET_MODE)
    {
        if (cParms != 1)
            rc = VERR_INVALID_PARAMETER;
        else if (paParms[0].type != VBOX_HGCM_SVC_PARM_32BIT)
            rc = VERR_INVALID_PARAMETER;
        else
            rc = modeSet(paParms[0].u.uint32);
    }
    else if (modeGet() != VBOX_DRAG_AND_DROP_MODE_OFF)
    {
        if (m_clientMap.size()) /* At least one client on the guest connected? */
        {
            rc = m_pManager->addMessage(u32Function, cParms, paParms, true /* fAppend */);
            if (RT_SUCCESS(rc))
            {
                if (m_clientQueue.size()) /* Any clients in our queue ready for processing the next command? */
                {
                    HGCM::Client *pClient = m_clientQueue.first();
                    AssertPtr(pClient);

                    /*
                     * Check if this was a request for getting the next host
                     * message. If so, return the message ID and the parameter
                     * count. The message itself has to be queued.
                     */
                    uint32_t uMsg = pClient->message();
                    if (uMsg == OutEnvFSSvc::GUEST_DND_GET_NEXT_HOST_MSG)
                    {
                        LogRel(("Client %RU32 is waiting for next host msg\n", pClient->clientId()));

                        uint32_t uMsg1;
                        uint32_t cParms1;
                        rc = m_pManager->nextMessageInfo(&uMsg1, &cParms1);
                        if (RT_SUCCESS(rc))
                        {
                            pClient->addMessageInfo(uMsg1, cParms1);
                            if (   m_pHelpers
                                && m_pHelpers->pfnCallComplete)
                            {
                                m_pHelpers->pfnCallComplete(pClient->handle(), rc);
                            }

                            m_clientQueue.removeFirst();

                            delete pClient;
                            pClient = NULL;
                        }
                        else
                            AssertMsgFailed(("m_pManager::nextMessageInfo failed with rc=%Rrc\n", rc));
                    }
                    else
                        AssertMsgFailed(("Client ID=%RU32 in wrong state with uMsg=%RU32\n",
                                         pClient->clientId(), uMsg));
                }
                else
                    LogRel(("All clients busy; delaying execution\n"));
            }
            else
                AssertMsgFailed(("Adding new message of type=%RU32 failed with rc=%Rrc\n",
                                 u32Function, rc));
        }
        else
        {
            /*
             * Tell the host that the guest does not support drag'n drop.
             * This might happen due to not installed Guest Additions or
             * not running VBoxTray/VBoxClient.
             */
            rc = VERR_NOT_SUPPORTED;
        }
    }
    else
    {
        /* Tell the host that a wrong drag'n drop mode is set. */
        rc = VERR_ACCESS_DENIED;
    }
#else
    rc = VERR_NOT_SUPPORTED;    
#endif
    LogFlowFuncLeaveRC(rc);
    return rc;
}

DECLCALLBACK(int) OutEnvFSService::progressCallback(uint32_t uStatus, uint32_t uPercentage, int rc, void *pvUser)
{
    AssertPtrReturn(pvUser, VERR_INVALID_POINTER);

    OutEnvFSService *pSelf = static_cast<OutEnvFSService *>(pvUser);
    AssertPtr(pSelf);
#if 0
    if (pSelf->m_pfnHostCallback)
    {
        LogRel(("GUEST_DND_HG_EVT_PROGRESS: uStatus=%RU32, uPercentage=%RU32, rc=%Rrc\n",
                     uStatus, uPercentage, rc));

        OutEnvFSSvc::VBOXDNDCBHGEVTPROGRESSDATA data;
        data.hdr.u32Magic = OutEnvFSSvc::CB_MAGIC_DND_HG_EVT_PROGRESS;
        data.uPercentage  = RT_MIN(uPercentage, 100);
        data.uStatus      = uStatus;
        data.rc           = rc; /** @todo uin32_t vs. int. */

        return pSelf->m_pfnHostCallback(pSelf->m_pvHostData,
                                        OutEnvFSSvc::GUEST_DND_HG_EVT_PROGRESS,
                                        &data, sizeof(data));
    }
#endif
    return VINF_SUCCESS;
}

/**
 * @copydoc VBOXHGCMSVCLOAD
 */
extern "C" DECLCALLBACK(DECLEXPORT(int)) VBoxHGCMSvcLoad(VBOXHGCMSVCFNTABLE *pTable)
{
	LogRel(("VBoxHGCMSvcLoad: fhaha load\n"));
    return OutEnvFSService::svcLoad(pTable);
}

