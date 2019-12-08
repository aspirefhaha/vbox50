/* $Id: UserInfoImpl.cpp $ */
/** @file
 * VirtualBox COM class implementation: UserInfo
 */

/*
 * Copyright (C) 2004-2015 
 */

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

// for some reason Windows burns in sdk\...\winsock.h if this isn't included first
#include "VBox/com/ptr.h"
#include "VirtualBoxImpl.h"

#include "UserInfoImpl.h"



#include "AutoCaller.h"
#include "Logging.h"


#if defined(RT_OS_LINUX) || defined(RT_OS_DARWIN) || defined(RT_OS_FREEBSD)
# include <set>
#endif

#if defined(RT_OS_DARWIN) && ARCH_BITS == 32
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

#ifdef RT_OS_LINUX
# include <sys/ioctl.h>
# include <errno.h>
# include <net/if.h>
# include <net/if_arp.h>
#endif /* RT_OS_LINUX */

#ifdef RT_OS_SOLARIS
# include <fcntl.h>
# include <unistd.h>
# include <stropts.h>
# include <errno.h>
# include <limits.h>
# include <stdio.h>
# include <libdevinfo.h>
# include <sys/mkdev.h>
# include <sys/scsi/generic/inquiry.h>
# include <net/if.h>
# include <sys/socket.h>
# include <sys/sockio.h>
# include <net/if_arp.h>
# include <net/if.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/cdio.h>
# include <sys/dkio.h>
# include <sys/mnttab.h>
# include <sys/mntent.h>
/* Dynamic loading of libhal on Solaris hosts */
# ifdef VBOX_USE_LIBHAL
#  include "vbox-libhal.h"
extern "C" char *getfullrawname(char *);
# endif
# include "solaris/DynLoadLibSolaris.h"

#endif /* RT_OS_SOLARIS */

#ifdef RT_OS_WINDOWS
# define _WIN32_DCOM
# include <windows.h>
# include <shellapi.h>
# define INITGUID
# include <guiddef.h>
# include <devguid.h>
# include <objbase.h>
//# include <setupapi.h>
# include <shlobj.h>
# include <cfgmgr32.h>

#endif /* RT_OS_WINDOWS */

#ifdef RT_OS_DARWIN
# include "darwin/iokit.h"
#endif

#ifdef VBOX_WITH_CROGL
#include <VBox/VBoxOGL.h>
#endif /* VBOX_WITH_CROGL */

#include <iprt/asm-amd64-x86.h>
#include <iprt/string.h>
#include <iprt/mp.h>
#include <iprt/time.h>
#include <iprt/param.h>
#include <iprt/env.h>
#include <iprt/mem.h>
#include <iprt/system.h>
#ifndef RT_OS_WINDOWS
# include <iprt/path.h>
#endif
#ifdef RT_OS_SOLARIS
# include <iprt/ctype.h>
#endif


/* XXX Solaris: definitions in /usr/include/sys/regset.h clash with hm_svm.h */
#undef DS
#undef ES
#undef CS
#undef SS
#undef FS
#undef GS

#include <VBox/usb.h>
#include <VBox/vmm/hm_svm.h>
#include <VBox/err.h>
#include <VBox/settings.h>
#include <VBox/sup.h>
#include <iprt/x86.h>

#include "VBox/com/MultiResult.h"
#include "VBox/com/array.h"

#include <stdio.h>

#include <algorithm>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
//
// UserInfo private data definition
//
////////////////////////////////////////////////////////////////////////////////

struct UserInfo::Data
{
    Data()
    {};

    VirtualBox              *pParent;
	Utf8Str currentuser;
	Utf8Str adminpwd;
	Utf8Str userpwd;
    
};


////////////////////////////////////////////////////////////////////////////////
//
// Constructor / destructor
//
////////////////////////////////////////////////////////////////////////////////
DEFINE_EMPTY_CTOR_DTOR(UserInfo)

HRESULT UserInfo::FinalConstruct()
{
    return BaseFinalConstruct();
}

void UserInfo::FinalRelease()
{
    uninit();
    BaseFinalRelease();
}

/**
 * Initializes the UserInfo object.
 *
 * @param aParent   VirtualBox parent object.
 */
HRESULT UserInfo::init(VirtualBox *aParent)
{
    HRESULT hrc;
    LogFlowThisFunc(("aParent=%p\n", aParent));

    /* Enclose the state transition NotReady->InInit->Ready */
    AutoInitSpan autoInitSpan(this);
    AssertReturn(autoInitSpan.isOk(), E_FAIL);

    m = new Data();

    m->pParent = aParent;


    /* Confirm a successful initialization */
    autoInitSpan.setSucceeded();

    return S_OK;
}

/**
 *  Uninitializes the host object and sets the ready flag to FALSE.
 *  Called either from FinalRelease() or by the parent when it gets destroyed.
 */
void UserInfo::uninit()
{
    LogFlowThisFunc(("\n"));

    /* Enclose the state transition Ready->InUninit->NotReady */
    AutoUninitSpan autoUninitSpan(this);
    if (autoUninitSpan.uninitDone())
        return;

    delete m;
    m = NULL;
}

////////////////////////////////////////////////////////////////////////////////
//
// IUserInfo public methods
//
////////////////////////////////////////////////////////////////////////////////


HRESULT UserInfo::i_loadSettings(const settings::UserInfo &data)
{
	return S_OK;
}	

HRESULT UserInfo::i_saveSettings(settings::UserInfo &data)
{
	return S_OK;
}


/**
 * Returns normal user pwd.
 *
 * @returns COM status code
 * @param normal user password
 */
HRESULT UserInfo::getUserpwd(com::Utf8Str &aUserpwd)
{
    
    return S_OK;
}

/**
 * Returns admin pwd.
 *
 * @returns COM status code
 * @param admin password
 */
HRESULT UserInfo::getAdminpwd(com::Utf8Str &aUserpwd)
{
    
    return S_OK;
}

/**
 * Returns admin pwd.
 *
 * @returns COM status code
 * @param admin password
 */
HRESULT UserInfo::getCurrentuser(com::Utf8Str &aUserName)
{
    
    return S_OK;
}

HRESULT UserInfo::getParent(ComPtr<IVirtualBox> &aParent)
{
	/* mParent is constant during life time, no need to lock */
    //ComObjPtr<VirtualBox> pVirtualBox(mParent);
    aParent = m->pParent;

    return S_OK;

}

HRESULT UserInfo::login(const com::Utf8Str &username, const com::Utf8Str &pwd, com::Utf8Str &user)
{
	return S_OK;
}


HRESULT UserInfo::chgPwd(const com::Utf8Str &username, const com::Utf8Str &oldpwd,const  com::Utf8Str &newpwd, com::Utf8Str &user)
{
	return S_OK;
}
   




// private methods
////////////////////////////////////////////////////////////////////////////////




/* vi: set tabstop=4 shiftwidth=4 expandtab: */
