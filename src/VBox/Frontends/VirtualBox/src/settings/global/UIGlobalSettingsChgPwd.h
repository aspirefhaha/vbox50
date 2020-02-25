/* $Id:  $ */
/** @file
 * VBox Qt GUI - UIGlobalSettingsChgPwd class declaration.
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

#ifndef __UIGlobalSettingsChgPwd_h__
#define __UIGlobalSettingsChgPwd_h__

/* GUI includes: */
#include <QDialog>
#include <QWidget>
#include "UIGlobalSettingsChgPwd.gen.h"

/* Forward declarations: */

/* Global settings  data: */

/* Global settings / Change Password page: */
class UIGlobalSettingsChgPwd : public QDialog
{
    Q_OBJECT

public:

    /* Constructor: */
    explicit UIGlobalSettingsChgPwd(QWidget * parent = NULL);
    ~UIGlobalSettingsChgPwd();

protected:

    /* API: Validation stuff: */
    //bool validate(QList<UIValidationMessage> &messages);
    void accept();
    void reject();

    /* API: Translation stuff: */
    void retranslateUi();
    Ui::UIGlobalSettingsChgPwd * ui;
private slots:

private:

    /* Variable: Editness flag: */
    bool m_fChanged;

    /* Variable: Cache: */
    //UISettingsCacheGlobalNetwork m_cache;
};

#endif // __UIGlobalSettingsChgPwd_h__
