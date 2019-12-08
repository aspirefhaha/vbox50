/* $Id: UIWizardLogin.cpp $ */
/** @file
 * VBox Qt GUI - UIWizardLogin class implementation.
 */

/*
 * Copyright (C) 2008-2012 
 */

#ifdef VBOX_WITH_PRECOMPILED_HEADERS
# include <precomp.h>
#else  /* !VBOX_WITH_PRECOMPILED_HEADERS */

/* GUI includes: */
# include "UIWizardLogin.h"
# include "UIWizardLoginPageBasic.h"
# include "VBoxGlobal.h"
# include "UIMessageCenter.h"
# include "QAbstractButton.h"

/* COM includes: */

#endif /* !VBOX_WITH_PRECOMPILED_HEADERS */


UIWizardLogin::UIWizardLogin(QWidget *pParent)
    : UIWizard(pParent, WizardType_Login)
{
#ifndef Q_WS_MAC
    /* Assign watermark: */
    assignWatermark(":/vmw_first_run.png");
#else /* Q_WS_MAC */
    /* Assign background image: */
    assignBackground(":/vmw_first_run_bg.png");
#endif /* Q_WS_MAC */
}

void UIWizardLogin::retranslateUi()
{
    /* Call to base-class: */
	QWizard::setOption(QWizard::HaveCustomButton1,false);
    UIWizard::retranslateUi();

    /* Translate wizard: */
    setWindowTitle(tr("Login"));
    setButtonText(QWizard::FinishButton, tr("Enter"));
	
	QWizard::setOption(QWizard::NoBackButtonOnStartPage,true);
	QWizard::setOption(QWizard::NoBackButtonOnLastPage,true);
	(QWizard::button(QWizard::CustomButton1))->setEnabled(false);
	(QWizard::button(QWizard::CustomButton1))->hide();
}

void UIWizardLogin::prepare()
{
    /* Create corresponding pages: */
    switch (mode())
    {
        case WizardMode_Expert:
        case WizardMode_Auto:
        case WizardMode_Basic:
        {
            setPage(Page, new UIWizardLoginPageBasic());
            break;
        }
        default:
        {
            AssertMsgFailed(("Invalid mode: %d", mode()));
            break;
        }
    }
    /* Call to base-class: */
    UIWizard::prepare();
	
	(QWizard::button(QWizard::CustomButton1))->hide();
}


