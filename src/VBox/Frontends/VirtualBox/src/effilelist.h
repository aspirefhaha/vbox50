#ifndef MYEFFILELIST_H
#define MYEFFILELIST_H

#include <QtGui/QDialog>
#include <QModelIndex>
#include "effilelist.gen.h"

class effilelist : public QDialog
{
	Q_OBJECT

public:
	effilelist(QWidget *parent = 0, Qt::WFlags flags = 0);
	~effilelist();
	QString selectFileName;
private slots:
	void sltDirSelect(QModelIndex idx);
	void sltFileSelect(QModelIndex idx);
	void sltSelectOk();
	void sltCancel();

private:
	Ui::effilelistClass ui;
};

#endif // MYEFFILELIST_H
