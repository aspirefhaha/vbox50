/* $Id: HostImpl.h $ */
/** @file
 * Implementation of IHost.
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

#ifndef ____H_USERINFOIMPL
#define ____H_USERINFOIMPL

#include "UserInfoWrap.h"

namespace settings
{
    struct UserInfo;
}

class ATL_NO_VTABLE UserInfo :
    public UserInfoWrap
{
public:

    DECLARE_EMPTY_CTOR_DTOR(UserInfo)

    HRESULT FinalConstruct();
    void FinalRelease();

    // public initializer/uninitializer for internal purposes only
    HRESULT init(VirtualBox *aParent);
    void uninit();

    // public methods only for internal purposes

    /**
     * Override of the default locking class to be used for validating lock
     * order with the standard member lock handle.
     */
    /*virtual VBoxLockingClass getLockingClass() const
    {
        return LOCKCLASS_USERINFOOBJECT;
    }*/

    HRESULT i_loadSettings(const settings::UserInfo &data);
    HRESULT i_saveSettings(settings::UserInfo &data);

private:

    // wrapped IUserInfo properties
    HRESULT getUserpwd(com::Utf8Str & userpwd);
    HRESULT getAdminpwd(com::Utf8Str & adminpwd);
    HRESULT setUserpwd(const com::Utf8Str & userpwd);
    HRESULT setAdminpwd(const com::Utf8Str & adminpwd);
    HRESULT getCurrentuser(com::Utf8Str & currentuser);
    HRESULT getLastuser(com::Utf8Str & lastuser);
    HRESULT getAdminleftcount(com::Utf8Str & adminleftcount);
    HRESULT getUserleftcount(com::Utf8Str & userleftcount);
    HRESULT setUserleftcount(const com::Utf8Str & userleftcount);
    //HRESULT setLastuser(const com::Utf8Str & lastuser);
    //HRESULT setCurrentuser(const com::Utf8Str & currentuser);
    HRESULT getParent(ComPtr<IVirtualBox> &aParent);

    // wrapped IUserInfo methods
    HRESULT login(const com::Utf8Str &username,const  com::Utf8Str &pwd, com::Utf8Str  &user);
    HRESULT chgPwd(const com::Utf8Str &username,const com::Utf8Str &oldpwd,const  com::Utf8Str &newpwd, com::Utf8Str &user);
    

    // Internal Methods.

    HRESULT i_doingNothing(com::Utf8Str & param);
    

#ifdef VBOX_WITH_RESOURCE_USAGE_API
#endif /* VBOX_WITH_RESOURCE_USAGE_API */

    struct Data;        // opaque data structure, defined in UserInfoImpl.cpp
    Data *m;
};

#endif // ____H_

