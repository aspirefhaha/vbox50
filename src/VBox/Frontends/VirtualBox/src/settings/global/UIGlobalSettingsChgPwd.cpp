/* $Id: UIGlobalSettingsChgPwd.cpp $ */
/** @file
 * VBox Qt GUI - UIGlobalSettingsChgPwd class implementation.
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
# include "VBoxGlobal.h"
# include "UIGlobalSettingsChgPwd.h"

/* COM includes: */

/* Other VBox includes: */
# include <iprt/cidr.h>

#endif /* !VBOX_WITH_PRECOMPILED_HEADERS */


UIGlobalSettingsChgPwd::UIGlobalSettingsChgPwd(QWidget * parent)
    : QDialog(parent),ui(new Ui::UIGlobalSettingsChgPwd), m_fChanged(false)
{
    /* Apply UI decorations: */
   ui->setupUi(this);
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Dialog);
    
    retranslateUi();
}

UIGlobalSettingsChgPwd::~UIGlobalSettingsChgPwd()
{

}

void UIGlobalSettingsChgPwd::retranslateUi()
{
    /* Translate uic generated strings: */
    ui->retranslateUi(this);

}

void UIGlobalSettingsChgPwd::reject()
{

    return QDialog::reject();
}

void UIGlobalSettingsChgPwd::accept()
{
    CUserInfo userInfo = vboxGlobal().userInfo();
    QString oldPwd = ui->le_oldPwd->text();
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
    }
}

