#include <QDebug>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTextItem>
#include <QtCore/qmath.h>
#include <QPushButton>
#include <QHBoxLayout>
#include <QClipboard>

#include "accountpage.h"
#include "ui_accountpage.h"
#include "goopal.h"
#include "debug_log.h"
#include "namedialog.h"
#include "deleteaccountdialog.h"
#include "accountcellwidget.h"
#include "exportdialog.h"
#include "importdialog.h"
#include "commondialog.h"
#include "showcontentdialog.h"
#include "incomecellwidget.h"
#include "dialog/chooseexportdialog.h"
#include "control/rightclickmenudialog.h"
#include "control/chooseaddaccountdialog.h"
#include "dialog/renamedialog.h"
#include "control/accountdetailwidget.h"
#include "frame.h"

#include "datamgr.h"
#include "thirddatamgr.h"

const int tabelWidgetHeight = 57;

AddressWidget::AddressWidget()
{
	addressLabel = new QLabel(this);
	addressLabel->setObjectName(QStringLiteral("identityLabel"));
	addressLabel->setGeometry(5, 0, 0, 0);
	addressLabel->setFont(QFont("Microsoft Yahei", 9));
	addressLabel->setStyleSheet(QStringLiteral("background:transparent;"));
	addressLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	copyBtn = new QToolButton(this);
	copyBtn->setGeometry(5, 0, 11, 13);
	copyBtn->setCursor(QCursor(Qt::PointingHandCursor));
	copyBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/copyBtn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
		"QToolButton:hover{background-image:url(%2pic2/copyBtn_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));

	connect(copyBtn, SIGNAL(clicked()), this, SLOT(copyBtnClicked()));
}

void AddressWidget::copyBtnClicked()
{
	QClipboard* clipBoard = QApplication::clipboard();
	clipBoard->setText(_address);
	CommonDialog commonDialog(CommonDialog::OkOnly);
	commonDialog.setText(tr("Copy to clipboard"));
	commonDialog.pop();
}

void AddressWidget::setAddress(const QString& address)
{
	_address = address;
	addressLabel->setText(address);
	addressLabel->adjustSize();
	addressLabel->move(5, (tabelWidgetHeight - addressLabel->size().height()) / 2);
	copyBtn->move(5 + addressLabel->size().width() + 5, (tabelWidgetHeight - copyBtn->size().height()) / 2);
}

OperationsWidget::OperationsWidget(AccountPage* page)
{
	accountPage = page;

	deleteBtn = new QToolButton(this);
	deleteBtn->setGeometry(QRect(4, (tabelWidgetHeight - 30) / 2, 76, 30));
	QFont font2;
	deleteBtn->setFont(font2);
	deleteBtn->setCursor(QCursor(Qt::PointingHandCursor));
	deleteBtn->setFont(QFont("Microsoft Yahei", 11));
	deleteBtn->setText(tr("Hide"));
	deleteBtn->setStyleSheet(QLatin1String("QToolButton{background-color:#999999;color:#ffffff;border:0px solid rgb(64,153,255);border-radius:3px;}"
		"QToolButton:hover{background-color:#62a9f8;}"));

	connect(deleteBtn, SIGNAL(clicked()), this, SLOT(deleteBtnClicked()));

	importBtn = new QToolButton(this);
	importBtn->setGeometry(QRect(84, (tabelWidgetHeight - 30) / 2, 76, 30));
	importBtn->setFont(font2);
	importBtn->setCursor(QCursor(Qt::PointingHandCursor));
	importBtn->setFont(QFont("Microsoft Yahei", 11));
	importBtn->setText(tr("Export"));
	importBtn->setStyleSheet(QLatin1String("QToolButton{background-color:#999999;color:#ffffff;border:1px solid rgb(187,187,187);border-radius:3px;}"
		"QToolButton:hover{background-color:#62a9f8;}"));

	connect(importBtn, SIGNAL(clicked()), this, SLOT(importBtnClicked()));
}

void OperationsWidget::deleteBtnClicked()
{
	accountPage->deleteAccount(accountName);
}

void OperationsWidget::importBtnClicked()
{
	ExportDialog exportDialog(accountName);
	exportDialog.hiddenCancelBtn();
	exportDialog.pop();
}

void OperationsWidget::setAccountName(const QString& account)
{
	accountName = account;
}

AccountPage::AccountPage(QWidget *parent) :
	QWidget(parent),
	hasDelegateOrNot(false),
	detailOrNot(false),
	ui(new Ui::AccountPage)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

	ui->setupUi(this);
	setAutoFillBackground(true);

	ui->accountTableWidget->installEventFilter(this);
	ui->accountTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
	ui->accountTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->accountTableWidget->setFocusPolicy(Qt::NoFocus);
	ui->accountTableWidget->setShowGrid(false);
	ui->accountTableWidget->setFrameShape(QFrame::NoFrame);
	ui->accountTableWidget->setMouseTracking(true);
	ui->accountTableWidget->setStyleSheet("QTableWidget{background:transparent;}");

	ui->accountTableWidget->setColumnWidth(0, 100);
	ui->accountTableWidget->setColumnWidth(1, 315);
	ui->accountTableWidget->setColumnWidth(2, 175);
	ui->accountTableWidget->setColumnWidth(3, 186);

	retranslator(DataMgr::getInstance()->getLanguage());

	ui->accountTableWidget->hide();
	ui->loadingLabel->show();
	ui->initLabel->hide();

	detailWidget = new AccountDetailWidget(this);
	connect(detailWidget, SIGNAL(back()), this, SLOT(hideDetailWidget()));
	connect(detailWidget, SIGNAL(upgrade(QString)), this, SIGNAL(showUpgradePage(QString)));
	connect(detailWidget, SIGNAL(applyDelegate(QString)), this, SIGNAL(showApplyDelegatePage(QString)));

	detailWidget->move(827, 93);
	detailWidget->raise();
	detailWidget->show();

	QLabel* addAccountBtnLabel = new QLabel(ui->addAccountBtn);
	addAccountBtnLabel->setGeometry(23, 12, 12, 12);

	updatePage();

	Frame* frame = dynamic_cast<Frame*>(Goopal::getInstance()->mainFrame);
	connect(frame, &Frame::updateAccountBalance, this, &AccountPage::updatePage);

	DLOG_QT_WALLET_FUNCTION_END;
}

AccountPage::~AccountPage()
{
	delete ui;
}

void AccountPage::importAccount()
{
	ImportDialog importDialog;
	importDialog.pop();

	updatePage();
	updateAccountCountLabel();

	emit refreshAccountInfo();
}

void AccountPage::addAccount()
{
	NameDialog nameDialog;
	QString name = nameDialog.pop();

	if (!name.isEmpty())
	{
		newAccountName = name;
		bool ret = DataMgr::getInstance()->walletAccountCreate(name);
		if (ret)
		{
			CommonDialog commonDialog(CommonDialog::OkOnly);
			commonDialog.setText(tr("Your account has been added successfully. Please save your private key now."));
			bool choice = commonDialog.pop();
			if (choice) {
				ExportDialog exportDialog(newAccountName);
				exportDialog.hiddenCancelBtn();
				exportDialog.pop();
			}

			updatePage();
		}
	}
}

void AccountPage::updateAccountList()
{
	DataMgr::getInstance()->walletListAccounts();

	accountNameList = DataMgr::getInstance()->getAccountInfo()->keys();
	if (accountNameList.size() == 0)  // 如果还没有账户
	{
		ui->initLabel->show();
		ui->loadingLabel->hide();
		ui->accountTableWidget->hide();
		return;
	}
	else {
		ui->initLabel->hide();
		ui->loadingLabel->show();
		ui->accountTableWidget->show();
	}

	int rowNum = 0;
	int size = DataMgr::getInstance()->getAccountInfo()->size();

	ui->accountTableWidget->setRowCount(size);

	for (int i = 0; i < size; i++)
	{
		QString accountName = accountNameList.at(i);

		ui->accountTableWidget->setRowHeight(rowNum, tabelWidgetHeight);

		//account name
		QTableWidgetItem* item0 = ui->accountTableWidget->item(rowNum, 0);
		if (item0 == nullptr)
		{
			item0 = new QTableWidgetItem();
			item0->setFont(QFont("Microsoft Yahei", 11));
			ui->accountTableWidget->setItem(rowNum, 0, item0);
		}
		item0->setText(accountName);

		//address
		AddressWidget* item1 = dynamic_cast<AddressWidget*>(ui->accountTableWidget->cellWidget(rowNum, 1));
		if (item1 == nullptr)
		{
			item1 = new AddressWidget();
			item1->setFont(QFont("Microsoft Yahei", 11));
			ui->accountTableWidget->setCellWidget(rowNum, 1, item1);
		}
		QString address = DataMgr::getInstance()->getAccountInfo()->value(accountName).address;
		item1->setAddress(address);

		//total price
		double totalPrice = 0;
		const auto& accountInfo = DataMgr::getInstance()->getAccountInfo()->value(accountName);
        auto itr = accountInfo.balances.begin();
		for (; itr != accountInfo.balances.end(); itr++)
		{
			double price = ThirdDataMgr::getInstance()->getAssetPrice(itr.key());
			double balance = itr.value().toDouble();
			if (price > 0 && balance > 0)
				totalPrice += price * balance;
		}

		QTableWidgetItem* item2 = ui->accountTableWidget->item(rowNum, 2);
		if (item2 == NULL)
		{
			item2 = new QTableWidgetItem();
			item2->setFont(QFont("Microsoft Yahei", 11));
			ui->accountTableWidget->setItem(rowNum, 2, item2);
		}
		QString currencySymbol = ThirdDataMgr::getInstance()->getCurrentCurrencySymbol();
		item2->setText(doubleToStr(totalPrice) + currencySymbol);

		//operations buttons
		OperationsWidget* item3 = dynamic_cast<OperationsWidget*>(ui->accountTableWidget->cellWidget(rowNum, 3));
		if (item3 == nullptr)
		{
			item3 = new OperationsWidget(this);
			ui->accountTableWidget->setCellWidget(rowNum, 3, item3);
		}
		item3->setAccountName(accountName);

		rowNum++;
	}

	ui->accountTableWidget->show();
	ui->loadingLabel->hide();
}

void AccountPage::on_addAccountBtn_clicked()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

	ChooseAddAccountDialog* chooseAddAccountDialog = new ChooseAddAccountDialog(this);
	chooseAddAccountDialog->move(ui->addAccountBtn->mapToGlobal(QPoint(0, -75)));
	connect(chooseAddAccountDialog, SIGNAL(newAccount()), this, SLOT(addAccount()));
	connect(chooseAddAccountDialog, SIGNAL(importAccount()), this, SLOT(importAccount()));
	chooseAddAccountDialog->setModal(false);
	chooseAddAccountDialog->show();

	DLOG_QT_WALLET_FUNCTION_END;
}

void AccountPage::on_accountTableWidget_cellClicked(int row, int column)
{
	//auto& accountName = accountNameList[row];
	//showDetailWidget(accountName);
}

void AccountPage::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBrush(QColor(255, 255, 255));
	painter.setPen(QPen(QColor(187, 187, 187), 2));
	painter.drawRect(QRect(0, 0, 826, 532));
	painter.drawRect(QRect(30, 70, 770, 42));
}

void AccountPage::retranslator(QString language)
{
	ui->retranslateUi(this);

	ui->addAccountBtn->setStyleSheet("QToolButton{background-color:#4dc8cf;color:#ffffff;border:0px solid rgb(64,153,255);border-radius:3px;}"
		"QToolButton:hover{background-color:#7dd9df;}");

	updateAccountCountLabel();
}

void AccountPage::updateAccountCountLabel()
{
	int totalCount = 0;
	int hasAssetCount = 0;

	auto accounts = DataMgr::getInstance()->getAccountInfo();
	for (auto itr = accounts->begin(); itr != accounts->end(); itr++)
	{
		totalCount++;

		auto balancesItr = itr->balances.begin();
		for (; balancesItr != itr->balances.end(); balancesItr++)
		{
			if (balancesItr.value().toDouble() > 0)
			{
				hasAssetCount++;
				break;
			}
		}
	}

	QString account_str(tr("account"));
	QString accounts_str(tr("accounts"));
	QString account_1 = totalCount == 1 ? account_str : accounts_str;
	QString account_2 = hasAssetCount == 1 ? account_str : accounts_str;

	auto countTip = QString(tr("You currently have %1 %2 and %3 %4 have assets.")).arg(totalCount).arg(account_1).arg(hasAssetCount).arg(account_2);
	ui->accountCountLabel->setText(countTip);
}

void AccountPage::updatePage()
{
	updateAccountCountLabel();
	updateAccountList();
	detailWidget->setAccount(detailWidget->accountName);
}

void AccountPage::showExportDialog(QString name)
{
	ExportDialog exportDialog(name);
	exportDialog.pop();
}

void AccountPage::renameAccount(QString name)
{
	RenameDialog renameDialog;
	QString newName = renameDialog.pop();

	if (!newName.isEmpty() && newName != name)
	{
		detailWidget->accountName = newName;
		emit newAccount();
	}
}

void AccountPage::deleteAccount(QString name)
{
	DeleteAccountDialog deleteACcountDialog(name);
	if (deleteACcountDialog.pop())
	{
		if (DataMgr::getInstance()->getCurrentAccount() == name)
		{
			DataMgr::getInstance()->setCurrentAccount("");
		}

		updatePage();

		CommonDialog tipDialog(CommonDialog::OkOnly);
        auto text = QString(tr("You have successfully hidden the account \"%1\".\nYou can retrieve the account with the private key in Add Account-Import Account.")).arg(name);
		tipDialog.setText(text, Qt::AlignLeft | Qt::AlignVCenter);
		tipDialog.pop();
	}
}

void AccountPage::showDetailWidget(QString name)
{
	detailOrNot = true;

//	ui->accountTableWidget->setGeometry(50, 124, 245, 263);
	ui->accountTableWidget->setColumnWidth(0, 100);
	ui->accountTableWidget->setColumnWidth(1, 1);
	ui->accountTableWidget->setColumnWidth(2, 158);
	ui->accountTableWidget->setColumnWidth(3, 1);

//	ui->addAccountBtn->move(153, 470);

	detailWidget->setAccount(name);
	detailWidget->dynamicShow();
}

void AccountPage::hideDetailWidget()
{
	detailOrNot = false;

	ui->accountTableWidget->setGeometry(50, 124, 776, 341);
	ui->accountTableWidget->setColumnWidth(0, 100);
	ui->accountTableWidget->setColumnWidth(1, 315);
	ui->accountTableWidget->setColumnWidth(2, 175);
	ui->accountTableWidget->setColumnWidth(3, 186);

	ui->addAccountBtn->move(675, 470);

	detailWidget->dynamicHide();
}
