#include "delegatespage.h"

#include <functional>

#include <QPainter>
#include "extra/dynamicmove.h"

#include "ui_delegatespage.h"
#include "delegateslistmodel.h"
#include "delegateslistdelegate.h"
#include "datamgr.h"
#include "waitingpage.h"
#include "goopal.h"

#define PAGE_ROW_MAX 10

DelegatesPage::DelegatesPage(QWidget *parent) : 
QWidget(parent),
ui(new Ui::DelegatesPage)
{
	ui->setupUi(this);
	setAutoFillBackground(true);

	ui->allDelegatesBtn->setStyleSheet("QToolButton{color:rgb(49,189,198);border:0px;}");
	ui->supportedDelegatesBtn->setStyleSheet("QToolButton{color:rgb(16,16,16);border:0px;}");

	ui->prePageBtn->setStyleSheet("QToolButton:!hover{border:0px;color:#999999;} QToolButton:hover{border:0px;color:#469cfc;}");
	ui->nextPageBtn->setStyleSheet("QToolButton:!hover{border:0px;color:#999999;} QToolButton:hover{border:0px;color:#469cfc;}");

	DataMgr::getInstance()->getVoteDelegateAccounts();

	//init list view
	delegatesListModel = new DelegatesListModel(this);

	DelegatesListDelegate* listDelegate = new DelegatesListDelegate(this);
	listDelegate->setCheckboxClickedCallback(std::bind(&DelegatesPage::onCheckboxClicked, this, std::placeholders::_1));

	ui->listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	ui->listView->setModel(delegatesListModel);
	ui->listView->setItemDelegate(listDelegate);

	QRegExp regx("[0-9]+$");
	QRegExpValidator *pReg1 = new QRegExpValidator(regx, this);
	ui->pageLineEdit->setValidator(pReg1);
	ui->pageLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
	ui->pageLineEdit->clearFocus();

	connect(DataMgr::getInstance(), &DataMgr::onWalletDelegateAccounts, this, &DelegatesPage::onDelegateAccountsCallback);
	connect(DataMgr::getInstance(), &DataMgr::onWalletDelegateAccountsByIds, this, &DelegatesPage::onDelegateAccountsByIdsCallback);

	on_allDelegatesBtn_clicked();
}

DelegatesPage::~DelegatesPage()
{
	delete ui;
}

void DelegatesPage::onCheckboxClicked(int accountId)
{
	if (DataMgr::getInstance()->isVoteDelegate(accountId))
	{
		DataMgr::getInstance()->deleteVoteDelegateAccount(accountId);
		if (tabFlag == SUPPORTED_DELEGATES_TAB)
		{
			goToSupportedPage(pageNum);
		}
	}
	else
	{
		auto pAccount = delegatesListModel->getDelegateAccount(accountId);
		DataMgr::getInstance()->saveVoteDelegateAccount(*pAccount);
	}

	delegatesListModel->refresh();
}

void DelegatesPage::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBrush(QColor(255, 255, 255));
	painter.setPen(QPen(QColor(187, 187, 187), 2));
	painter.drawRect(QRect(0, 0, 826, 532));
}

void DelegatesPage::resetPageNum()
{
	pageNum = 1;
	totalPageNum = 1;

	ui->pageLabel->setText("1/1");

	delegatesListModel->setDelegateAccounts(QVector<DelegateAccount>());
}

void DelegatesPage::goToAllPage(int num)
{
	showWaitingPage();
	DataMgr::getInstance()->walletDelegateAccounts(num);

	delegatesListModel->setDelegateAccounts(QVector<DelegateAccount>());
}

void DelegatesPage::onDelegateAccountsCallback(bool success, int currentPage, int totalPage, const QVector<DelegateAccount>& accounts)
{
	if (success)
	{
		pageNum = currentPage;
		totalPageNum = totalPage;
		ui->pageLabel->setText(QString::number(pageNum) + "/" + QString::number(totalPageNum));
		delegatesListModel->setDelegateAccounts(accounts);
		delegatesListModel->setPageNum(pageNum);
	}

	hideWaitingPage();
}

void DelegatesPage::goToSupportedPage(int num)
{
	showWaitingPage();

	auto delegateAccounts = DataMgr::getInstance()->getVoteDelegateAccounts();
	totalPageNum = delegateAccounts->size() / PAGE_ROW_MAX + (delegateAccounts->size() % PAGE_ROW_MAX != 0 ? 1 : 0);
	totalPageNum = totalPageNum == 0 ? 1 : totalPageNum;

	if (num > totalPageNum)
		num = totalPageNum;

	pageNum = num;
	delegatesListModel->setPageNum(pageNum);

	int startNum = (pageNum - 1) * PAGE_ROW_MAX;
	int endNum = startNum + PAGE_ROW_MAX;
	endNum = delegateAccounts->size() < endNum ? delegateAccounts->size() : endNum;

	QVector<int> accounts;
	for (int i = startNum; i < endNum; i++)
	{
		accounts.push_back(delegateAccounts->at(i).id);
	}
	DataMgr::getInstance()->walletDelegateAccountsByIds(accounts);

	delegatesListModel->setDelegateAccounts(QVector<DelegateAccount>());
}

void DelegatesPage::onDelegateAccountsByIdsCallback(bool success, const QVector<DelegateAccount>& accounts)
{
	if (success)
	{
		ui->pageLabel->setText(QString::number(pageNum) + "/" + QString::number(totalPageNum));
		delegatesListModel->setDelegateAccounts(accounts);
	}

	hideWaitingPage();
}

void DelegatesPage::on_allDelegatesBtn_clicked()
{
	tabFlag = ALL_DELEGATES_TAB;

	DynamicMove* dynamicMove = new DynamicMove(ui->moveLabel, QPoint(34, 48), 180, this);
	dynamicMove->start();

	ui->numberLabel->show();
	ui->pageLineEdit->show();
	ui->goToBtn->show();

	ui->allDelegatesBtn->setStyleSheet("QToolButton{color:rgb(49,189,198);border:0px;}");
	ui->supportedDelegatesBtn->setStyleSheet("QToolButton{color:rgb(16,16,16);border:0px;}");

	resetPageNum();
	goToAllPage(1);
}

void DelegatesPage::on_supportedDelegatesBtn_clicked()
{
	tabFlag = SUPPORTED_DELEGATES_TAB;

	DynamicMove* dynamicMove = new DynamicMove(ui->moveLabel, QPoint(161, 48), 180, this);
	dynamicMove->start();

	ui->numberLabel->hide();
	ui->pageLineEdit->hide();
	ui->goToBtn->hide();

	ui->allDelegatesBtn->setStyleSheet("QToolButton{color:rgb(16,16,16);border:0px;}");
	ui->supportedDelegatesBtn->setStyleSheet("QToolButton{color:rgb(49,189,198);border:0px;}");

	resetPageNum();
	goToSupportedPage(1);
}

void DelegatesPage::on_prePageBtn_clicked()
{
	if (pageNum <= 1)
		return;
	pageNum--;

	if (tabFlag == ALL_DELEGATES_TAB)
		goToAllPage(pageNum);
	else if (tabFlag == SUPPORTED_DELEGATES_TAB)
		goToSupportedPage(pageNum);
}

void DelegatesPage::on_nextPageBtn_clicked()
{
	if (pageNum >= totalPageNum)
		return;
	pageNum++;

	if (tabFlag == ALL_DELEGATES_TAB)
		goToAllPage(pageNum);
	else if (tabFlag == SUPPORTED_DELEGATES_TAB)
		goToSupportedPage(pageNum);
}

void DelegatesPage::on_goToBtn_clicked()
{
	QString sPageNum = ui->pageLineEdit->text();
	if (sPageNum.isEmpty())
		return;

	pageNum = sPageNum.toInt();

	if (tabFlag == ALL_DELEGATES_TAB)
		goToAllPage(pageNum);
	else if (tabFlag == SUPPORTED_DELEGATES_TAB)
		goToSupportedPage(pageNum);
}

void DelegatesPage::on_pageLineEdit_textChanged(const QString &arg1)
{
	if (ui->pageLineEdit->text().isEmpty())
		return;

	int num = ui->pageLineEdit->text().toInt();
	if (totalPageNum == 0)
		ui->pageLineEdit->setText("");
	else if (num > totalPageNum)
		ui->pageLineEdit->setText(QString::number(totalPageNum));
	else if (num < 1)
		ui->pageLineEdit->setText("1");
}

void DelegatesPage::showWaitingPage()
{
	ui->pageLineEdit->clearFocus();

	if (waitingPage == nullptr)
	{
		waitingPage = new WaitingPage(Goopal::getInstance()->mainFrame);
		waitingPage->move(0, 0);
		waitingPage->show();
	}
}

void DelegatesPage::hideWaitingPage()
{
	if (waitingPage != nullptr)
	{
		waitingPage->close();
		delete waitingPage;
		waitingPage = nullptr;
	}
}
