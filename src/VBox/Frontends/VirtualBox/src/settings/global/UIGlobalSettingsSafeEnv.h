/* $Id:  $ */
/** @file
 * VBox Qt GUI - UIGlobalSettingsSafeEnv class declaration.
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

#ifndef __UIGlobalSettingsSafeEnv_h__
#define __UIGlobalSettingsSafeEnv_h__

/* GUI includes: */
#include <QDialog>
#include <QWidget>
#include "UIGlobalSettingsSafeEnv.gen.h"

/* Forward declarations: */

/* Global settings  data: */

/* Global settings / Change Password page: */
class UIGlobalSettingsSafeEnv : public QDialog
{
    Q_OBJECT

public:

    /* Constructor: */
    explicit UIGlobalSettingsSafeEnv(QWidget * parent = NULL);
    ~UIGlobalSettingsSafeEnv();

public slots:
    void resetUserCount();
    void resetUserPwd();
    void setSelfDestroy(bool);
    void sltSetInOutThrough(bool);

protected:

    /* API: Validation stuff: */
    //bool validate(QList<UIValidationMessage> &messages);
    void accept();

    /* API: Translation stuff: */
    void retranslateUi();
    Ui::UIGlobalSettingsSafeEnv * ui;
private slots:

private:

    /* Variable: Editness flag: */
    bool m_fChanged;

    /* Variable: Cache: */
    //UISettingsCacheGlobalNetwork m_cache;
};

#endif // __UIGlobalSettingsSafeEnv_h__
