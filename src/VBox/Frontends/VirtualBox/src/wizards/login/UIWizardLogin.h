/* $Id: UIWizardLogin.h $ */
/** @file
 * VBox Qt GUI - UIWizardLogin class declaration.
 */

/*
 * Copyright (C) 2008-2012 
 */

#ifndef __UIWizardLogin_h__
#define __UIWizardLogin_h__

/* GUI includes: */
#include "UIWizard.h"

/* COM includes: */
#include "COMEnums.h"

/* First Run wizard: */
class UIWizardLogin : public UIWizard
{
    Q_OBJECT;

public:

    /* Page IDs: */
    enum
    {
        Page
    };

    /* Constructor: */
    UIWizardLogin(QWidget *pParent);

protected:

    
    friend class UIWizardFirstRunPageBasic;

private:

    /* Translate stuff: */
    void retranslateUi();

    /* Pages related stuff: */
    void prepare();


    bool m_fHardDiskWasSet;
};

#endif // __UIWizardLogin_h__

