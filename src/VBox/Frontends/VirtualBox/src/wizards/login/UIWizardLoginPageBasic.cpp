/* $Id: UIWizardLoginPageBasic.cpp $ */
/** @file
 * VBox Qt GUI - UIWizardLoginPageBasic class implementation.
 */

/*
 * Copyright (C) 2008-2013 
 */

#ifdef VBOX_WITH_PRECOMPILED_HEADERS
# include <precomp.h>
#else  /* !VBOX_WITH_PRECOMPILED_HEADERS */

/* Qt includes: */
# include <QVBoxLayout>
# include <QHBoxLayout>

/* GUI includes: */
# include "UIWizardLoginPageBasic.h"
# include "UIWizardLogin.h"
# include "UIIconPool.h"
# include "VBoxGlobal.h"
# include "UIMessageCenter.h"

# include "QIToolButton.h"
# include "QIRichTextLabel.h"


#endif /* !VBOX_WITH_PRECOMPILED_HEADERS */


UIWizardLoginPage::UIWizardLoginPage()
   
{
}

QString UIWizardLoginPage::id() const
{
    //return m_pMediaSelector->id();
    return "1";
}

void UIWizardLoginPage::setId(const QString &strId)
{
    //m_pMediaSelector->setCurrentItem(strId);
}

UIWizardLoginPageBasic::UIWizardLoginPageBasic()
    : UIWizardLoginPage()
{
    /* Create widgets: */
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->setContentsMargins(8, 0, 8, 0);
        pMainLayout->setSpacing(10);
        m_pLabel = new QIRichTextLabel(this);
        QHBoxLayout *pSourceDiskLayout = new QHBoxLayout;
        {
            
            m_pSelectMediaButton = new QIToolButton(this);
            {
                m_pSelectMediaButton->setIcon(UIIconPool::iconSet(":/select_file_16px.png", ":/select_file_disabled_16px.png"));
                m_pSelectMediaButton->setAutoRaise(true);
            }
            //pSourceDiskLayout->addWidget(m_pMediaSelector);
            pSourceDiskLayout->addWidget(m_pSelectMediaButton);
        }
        pMainLayout->addWidget(m_pLabel);
        pMainLayout->addLayout(pSourceDiskLayout);
        pMainLayout->addStretch();
    }

    /* Setup connections: */
    //connect(m_pMediaSelector, SIGNAL(currentIndexChanged(int)), this, SIGNAL(completeChanged()));
    //connect(m_pSelectMediaButton, SIGNAL(clicked()), this, SLOT(sltOpenMediumWithFileOpenDialog()));

    /* Register fields: */
    registerField("source", this, "source");
    registerField("id", this, "id");
}

void UIWizardLoginPageBasic::retranslateUi()
{
    /* Translate widgets: */
    
    m_pLabel->setText(UIWizardLogin::tr("<p>Please select a virtual optical disk file "
                                           "or a physical optical drive containing a disk "
                                           "to start your new virtual machine from.</p>"
                                           "<p>The disk should be suitable for starting a computer from "
                                           "and should contain the operating system you wish to install "
                                           "on the virtual machine if you want to do that now. "
                                           "The disk will be ejected from the virtual drive "
                                           "automatically next time you switch the virtual machine off, "
                                           "but you can also do this yourself if needed using the Devices menu.</p>"));
    m_pSelectMediaButton->setToolTip(UIWizardLogin::tr("Choose a virtual optical disk file..."));
}

void UIWizardLoginPageBasic::initializePage()
{
    /* Translate page: */
    retranslateUi();
}

bool UIWizardLoginPageBasic::isComplete() const
{
    /* Make sure valid medium chosen: */
    //return !vboxGlobal().medium(id()).isNull();
    return true;
}

bool UIWizardLoginPageBasic::validatePage()
{
    /* Initial result: */
    bool fResult = true;

    /* Lock finish button: */
    startProcessing();

    /* Try to insert chosen medium: */
    //if (fResult)
    //    fResult = qobject_cast<UIWizardFirstRun*>(wizard())->insertMedium();

    /* Unlock finish button: */
    endProcessing();

    /* Return result: */
    return fResult;
}

QString UIWizardLoginPageBasic::source() const
{
    //return m_pMediaSelector->currentText();
    return "unknown source";
}

