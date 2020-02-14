/** @file
 * FhahaLib - Windows Logon DLL for VirtualBox.
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

#ifndef __FHAHALIB_H__
#define __FHAHALIB_H__

/** @name FHAHALIB entry point calls
 * @{
 */
typedef struct _st_FhahaLib_Func{
	BOOL isLoaded;
    
	//windows 
	DWORD (*GetLogicalDrives)(void);
	BOOL (*SetCurrentDirectory)(LPCTSTR lpPathName);
	DWORD (*GetCurrentDirectory)(DWORD nBufferLength, LPTSTR lpBuffer);
	long long (*FindFirstFile)(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
	BOOL (*FindNextFile)(long long hFindFile, LPWIN32_FIND_DATA lpFindFileData);
	BOOL (*FindClose)(long long hFindFile);
	long long (*CreateFile)(LPCTSTR lpFileName,DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, long long hTemplateFile);
	BOOL (*DeleteFile)(LPCTSTR lpFileName);
	BOOL (*GetFileInformationByHandle)(long long hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation);
	DWORD (*GetFileSize)(long long hFile, LPDWORD lpFileSizeHigh);
	BOOL (*GetFileSizeEx)(long long hFile, PLARGE_INTEGER lpFileSize);
	DWORD (*GetTempPath)(DWORD nBufferLength, LPTSTR lpBuffer);
	BOOL (*MoveFile)(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName);
	BOOL (*CopyFile)(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists);
	BOOL (*SetFileTime)(long long hFile, const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime);
	BOOL (*ReadFile)(long long hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
	BOOL (*SetEndOfFile)(long long hFile);
	DWORD (*SetFilePointer)(long long hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
	BOOL (*WriteFile)(long long hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
	BOOL (*FlushFileBuffers)(long long hFile);
	BOOL (*CloseHandle)(long long hObject);
	BOOL (*CreateDirectory)(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	BOOL (*RemoveDirectory)(LPCTSTR lpPathName);
	BOOL (*GetDiskFreeSpaceEx)(LPCTSTR lpDirectoryName,  PULARGE_INTEGER lpFreeBytesAvailable,  PULARGE_INTEGER lpTotalNumberOfBytes,
  					PULARGE_INTEGER lpTotalNumberOfFreeBytes);
	UINT (*GetDriveType)( LPCTSTR lpRootPathName);
	DWORD (*GetLastError)(void);
	DWORD  (*GetCurrentModuleDir)(LPTSTR pBuf, DWORD InLen);
	BOOL (*PathFileExists)(LPCTSTR pszPath);	
	UINT (*GetDiskType)(LPCTSTR lpRootPathName);
	//HRESULT (*SHGetFolderPath)(HWND hwndOwner,int nFolder,long long hToken,DWORD dwFlags,LPTSTR pszPath);
}FhahaLib_Func;
/** @}  */

#endif /* !__FHAHALIB_H__ */

