#ifndef ACCOUNTPAGE_H
#define ACCOUNTPAGE_H

#include <QWidget>
#include <QMap>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QLabel>

#include "accountcellwidget.h"

namespace Ui {
class AccountPage;
}

class AccountDetailWidget;
class WaitingPage;
class AccountPage;

class AddressWidget : public QWidget
{
	Q_OBJECT

public:
	AddressWidget();
	~AddressWidget(){};

	void setAddress(const QString& address);

	public slots:
	void copyBtnClicked();

private:
	QLabel* addressLabel;
	QToolButton* copyBtn;
	QString _address;
};

class OperationsWidget : public QWidget
{
	Q_OBJECT

public:
	OperationsWidget(AccountPage* page);
	~OperationsWidget(){};

	void setAccountName(const QString& account);

	public slots:
	void deleteBtnClicked();
	void importBtnClicked();

private:
	AccountPage* accountPage;
	QToolButton* deleteBtn;
	QToolButton* importBtn;
	QString accountName;
};

class AccountPage : public QWidget
{
    Q_OBJECT

public:
    explicit AccountPage(QWidget *parent = 0);
    ~AccountPage();

	void updateAccountList();
	void retranslator(QString language);
	void renameAccount(QString name);
	void deleteAccount(QString name);
	void refreshPage();

private:
	void updateAccountCountLabel();
	void paintEvent(QPaintEvent*);

signals:
	void openAccountPage(QString);
	void showShadowWidget();
	void hideShadowWidget();
	void showUpgradePage(QString);
	void showApplyDelegatePage(QString);
	void showTransferPage(QString);
	void refreshAccountInfo();

private slots:
	void addAccount();
	void importAccount();
	void on_addAccountBtn_clicked();
	void on_accountTableWidget_cellClicked(int row, int column);
	void showExportDialog(QString name);
	void showDetailWidget(QString name);
	void hideDetailWidget();

private:
	Ui::AccountPage *ui;
	bool hasDelegateOrNot;
	bool detailOrNot;
	QString newAccountName;

	AccountDetailWidget* detailWidget;

	QList<QString> accountNameList;
};

#endif // ACCOUNTPAGE_H
