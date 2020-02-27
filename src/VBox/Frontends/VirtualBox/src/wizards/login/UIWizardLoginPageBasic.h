/* $Id: UIWizardLoginPageBasic.h $ */
/** @file
 * VBox Qt GUI - UIWizardLoginPageBasic class declaration.
 */

/*
 * Copyright (C) 2008-2012
 */

#ifndef __UIWizardLoginPageBasic_h__
#define __UIWizardLoginPageBasic_h__

/* Local includes: */
#include "UIWizardPage.h"

/* Forward declarations: */
class QIToolButton;
class QIRichTextLabel;
class QLineEdit;
class QRadioButton;
class QCheckBox;
class QButtonGroup;

/* Single page of the First Run wizard (base part): */
class UIWizardLoginPage : public UIWizardPageBase
{
public:
    bool m_bChgPwd;
protected:

    /* Constructor: */
    UIWizardLoginPage();

    /* Handlers: */

    /* Stuff for 'id' field: */
    QString id() const;
    void setId(const QString &strId);

    /* Variables: */
	

    /* Widgets: */
    //QIToolButton *m_pSelectMediaButton;
    //QPushButton * m_pbChgPwd;
    QCheckBox * m_pckChgPwd;
    QButtonGroup * m_pRole;
    QRadioButton * m_pAdmin;
	QRadioButton * m_pUser;
	QLineEdit * m_pPwd;
};

/* Single page of the First Run wizard (basic extension): */
class UIWizardLoginPageBasic : public UIWizardPage, public UIWizardLoginPage
{
    Q_OBJECT;
    Q_PROPERTY(QString source READ source);
    Q_PROPERTY(QString id READ id WRITE setId);

public:

    /* Constructor: */
    UIWizardLoginPageBasic();

protected:

    /* Wrapper to access 'this' from base part: */
    UIWizardPage* thisImp() { return this; }

private slots:

    /* Open with file-open dialog: */
    void sltChgPwd();
    void sltUser(bool);

private:

    /* Translate stuff: */
    void retranslateUi();

    /* Prepare stuff: */
    void initializePage();

    /* Validation stuff: */
    bool isComplete() const;

    /* Validation stuff: */
    bool validatePage();

    /* Stuff for 'source' field: */
    QString source() const;

    /* Widgets: */
    QIRichTextLabel *m_pLabel;
	QIRichTextLabel * m_pTip;
};

#endif // __UIWizardFirstRunPageBasic_h__

