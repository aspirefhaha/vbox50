/* $Id: UIGlobalSettingsSafeEnv.cpp $ */
/** @file
 * VBox Qt GUI - UIGlobalSettingsSafeEnv class implementation.
 */

/*
 * Copyright (C) 2009-2013 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifdef VBOX_WITH_PRECOMPILED_HEADERS
# include <precomp.h>
#else  /* !VBOX_WITH_PRECOMPILED_HEADERS */

/* Qt includes: */

/* GUI includes: */
# include <QMessageBox>
# include <QDateTime>
# include "VBoxGlobal.h"
# include "UIGlobalSettingsSafeEnv.h"

/* COM includes: */

/* Other VBox includes: */
# include <iprt/cidr.h>

#endif /* !VBOX_WITH_PRECOMPILED_HEADERS */


UIGlobalSettingsSafeEnv::UIGlobalSettingsSafeEnv(QWidget * parent)
    : QDialog(parent),ui(new Ui::UIGlobalSettingsSafeEnv), m_fChanged(false)
{
    /* Apply UI decorations: */
    ui->setupUi(this);
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Dialog);

    retranslateUi();
    CVirtualBox vbox = vboxGlobal().virtualBox();
    CUserInfo userInfo = vbox.GetUserInfo();
    ui->lcd_UserCount->display(userInfo.GetUserleftcount());
    ui->lcd_AdminCount->display(userInfo.GetAdminleftcount());
    int checked = vbox.GetExtraDataInt("selfDestroy");

    ui->cb_SelfDestroy->setCheckState(checked?Qt::Checked : Qt::Unchecked);
}

UIGlobalSettingsSafeEnv::~UIGlobalSettingsSafeEnv()
{

}

void UIGlobalSettingsSafeEnv::retranslateUi()
{
    /* Translate uic generated strings: */
    ui->retranslateUi(this);

}

void UIGlobalSettingsSafeEnv::resetUserCount()
{
    CVirtualBox vbox = vboxGlobal().virtualBox();
    CUserInfo userInfo = vbox.GetUserInfo();
    userInfo.SetUserleftcount("5");
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz");
    vbox.SetExtraData("safeenvlastsave",current_date);
    ui->lcd_UserCount->display(userInfo.GetUserleftcount());
}

void UIGlobalSettingsSafeEnv::setSelfDestroy(bool des)
{
    CVirtualBox vbox = vboxGlobal().virtualBox();
    vbox.SetExtraDataInt("selfDestroy",des?1:0);
}

void UIGlobalSettingsSafeEnv::resetUserPwd()
{
    CVirtualBox vbox = vboxGlobal().virtualBox();
    CUserInfo userInfo = vbox.GetUserInfo();
    QString newPwd = ui->le_newPwd->text();
    QString verifyPwd = ui->le_verifyPwd->text();
    if(newPwd != verifyPwd){
        QMessageBox::warning(NULL,"Password Err", "New Password is <font color='red'>not matched</font> with verify Password!");
        return;
    }
    userInfo.SetUserpwd(newPwd);
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz");
    vbox.SetExtraData("safeenvlastsave",current_date);
    
}

void UIGlobalSettingsSafeEnv::accept()
{
    //CVirtualBox vbox = vboxGlobal().virtualBox();
    //CUserInfo userInfo = vbox.GetUserInfo();
    /*QString oldPwd = ui->le_oldPwd->text();
    QString newPwd = ui->le_newPwd->text();
    QString verifyPwd = ui->le_verifyPwd->text();
    QString oriPwd ;
    QString curusr = userInfo.GetCurrentuser();
    if(curusr == "admin"){
        oriPwd = userInfo.GetAdminpwd();
    }
    else
        oriPwd = userInfo.GetUserpwd();
    if(oldPwd == oriPwd){
        if(newPwd != verifyPwd){
            QMessageBox::warning(NULL,"Password Err", "New Password is <font color='red'>not matched</font> with verify Password!");
            return;
        }
        if(curusr == "admin"){
            userInfo.SetAdminpwd(newPwd);
        }
        else{
            userInfo.SetUserpwd(newPwd);
        }
        
        return QDialog::accept();
        
    }
    else{
        //QMessageBox::information(NULL, "Old Password Err", "Old Password is not match!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        QMessageBox::warning(NULL,"Password Err", "Old Password is <font color='red'>not matched</font>");
    }*/
    QDialog::accept();
}

