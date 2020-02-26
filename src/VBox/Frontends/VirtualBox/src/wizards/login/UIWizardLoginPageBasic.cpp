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
# include <QFormLayout>
# include <QDateTime>

/* GUI includes: */
# include "UIWizardLoginPageBasic.h"
# include "UIWizardLogin.h"
# include "UIIconPool.h"
# include "VBoxGlobal.h"
# include "UIMessageCenter.h"
# include "UIGlobalSettingsChgPwd.h"

# include "QIToolButton.h"
# include "QIRichTextLabel.h"
# include "QLineEdit.h"
# include "QRadioButton.h"
# include "QButtonGroup.h"


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
		m_pTip = new QIRichTextLabel(this);
        QFormLayout *pUserPwdLayout = new QFormLayout;
		QHBoxLayout * pRoleLayout = new QHBoxLayout;
        {
            
            //m_pSelectMediaButton = new QIToolButton(this);
            //{
            //    m_pSelectMediaButton->setIcon(UIIconPool::iconSet(":/select_file_16px.png", ":/select_file_disabled_16px.png"));
            //    m_pSelectMediaButton->setAutoRaise(true);
            //}
            //pSourceDiskLayout->addWidget(m_pMediaSelector);
            //pSourceDiskLayout->addWidget(m_pSelectMediaButton);
            m_pRole = new QButtonGroup(this);
			m_pAdmin = new QRadioButton(UIWizardLogin::tr("Admin"),this);
    		m_pAdmin->setIcon(QIcon(":/select_file_16px.png"));
			m_pUser = new QRadioButton(UIWizardLogin::tr("User"),this);
			m_pUser->setIcon(QIcon(":/select_file_disabled_16px.png"));
			m_pUser->setChecked(true);
			m_pRole->addButton(m_pAdmin,0);
			m_pRole->addButton(m_pUser,1);
			m_pPwd = new QLineEdit(this);
			m_pPwd->setEchoMode(QLineEdit::Password);
			pRoleLayout->addWidget(m_pAdmin);
			pRoleLayout->addWidget(m_pUser);
			
			pUserPwdLayout->addRow(UIWizardLogin::tr("Role"),pRoleLayout);
			pUserPwdLayout->addRow(UIWizardLogin::tr("Password"),m_pPwd);
        }
        pMainLayout->addWidget(m_pLabel);
        pMainLayout->addLayout(pUserPwdLayout);
        m_pbChgPwd = new QPushButton(UIWizardLogin::tr("Change Pwd"),this);
        connect(m_pbChgPwd,SIGNAL(clicked()),this,SLOT(sltChgPwd()));
        pMainLayout->addWidget(m_pbChgPwd);
		pMainLayout->addWidget(m_pTip);
        pMainLayout->addStretch();
    }

    /* Setup connections: */
    //connect(m_pMediaSelector, SIGNAL(currentIndexChanged(int)), this, SIGNAL(completeChanged()));
    //connect(m_pSelectMediaButton, SIGNAL(clicked()), this, SLOT(sltOpenMediumWithFileOpenDialog()));

    /* Register fields: */
    registerField("source", this, "source");
    registerField("id", this, "id");
	registerField("pwd*",m_pPwd);
}

void UIWizardLoginPageBasic::retranslateUi()
{
    /* Translate widgets: */
    
    m_pLabel->setText(UIWizardLogin::tr("<p>Please select a role for further operations, "
                                           "Admin can change all USB virtual enviroment config."
                                           "User can only modify few virtual config."
                                           "Don't forget enter password for the role you choosed</p>"));
    //m_pSelectMediaButton->setToolTip(UIWizardLogin::tr("Choose a virtual optical disk file..."));
}

void UIWizardLoginPageBasic::initializePage()
{
    /* Translate page: */
    retranslateUi();
    
    CVirtualBox vbox = vboxGlobal().virtualBox();
    CUserInfo userinfo = vbox.GetUserInfo();
    QString lastuser = userinfo.GetLastuser();
    if(lastuser=="admin"){
        m_pAdmin->setChecked(true);
    }
    else{
        m_pUser->setChecked(true);
    }
}

bool UIWizardLoginPageBasic::isComplete() const
{
    /* Make sure valid medium chosen: */
    //return !vboxGlobal().medium(id()).isNull();
    return true;
}

void UIWizardLoginPageBasic::sltChgPwd()
{
    UIGlobalSettingsChgPwd * pChgPwdDlg = new UIGlobalSettingsChgPwd();
    pChgPwdDlg->exec();

}

bool UIWizardLoginPageBasic::validatePage()
{
    /* Initial result: */
    bool fResult = false;
	
    startProcessing();
	CVirtualBox vbox = vboxGlobal().virtualBox();
    CUserInfo userInfo = vbox.GetUserInfo();
	QString curser = userInfo.GetCurrentuser();
	QString lastuser = userInfo.GetLastuser();
	
	//QString userpwd = userinfo.GetUserpwd();
	//QString adminpwd = userinfo.GetAdminpwd();
	//QString userpwd = vbox.GetUserpwd();
	//QString	adminpwd = vbox.GetAdminpwd();
	QString inputpwd = field("pwd").toString();
	if(curser.isEmpty()){
		if(m_pRole->checkedId() == 0){
			//if(inputpwd == adminpwd){
            if(userInfo.Login("admin",inputpwd)=="admin"){
				//vbox.Login("admin");
				fResult = true;
			}
			else{
                QDateTime current_date_time =QDateTime::currentDateTime();
                QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz");
                vbox.SetExtraData("safeenvlastsave",current_date);
                //vbox.SaveSettings();
				m_pTip->setText(QString(UIWizardLogin::tr("Login Failed as<font color=red> Admin</font>,Left <font color=red>%1</font> Chances!")).arg(userInfo.GetAdminleftcount()));
                
            }
		}
		else if(m_pRole->checkedId() == 1){
			//if(inputpwd == userpwd){
            if(userInfo.Login("user",inputpwd)=="user"){
				//vbox.Login("user");
				fResult = true;
			}
			else {
                QDateTime current_date_time =QDateTime::currentDateTime();
                QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz");
                vbox.SetExtraData("safeenvlastsave",current_date);
				m_pTip->setText(QString(UIWizardLogin::tr("Login Failed as <font color=red>User</font>,Left <font color=red>%1</font> Chances!")).arg(userInfo.GetUserleftcount()));
            }
		}
		else 
		{
			m_pTip->setText(UIWizardLogin::tr("<font color=red>Login Failed</font>"));
		}
        //vbox.saveSettings();
	}
	else{
		fResult = true;
	}
	
    /* Lock finish button: */
	//fResult = false;

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

