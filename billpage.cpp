#include <QDebug>
#include <QPainter>
#include <QHelpEvent>
#include <QDateTime>
#include <QTextCodec>
#include <QScrollBar>
#include <QFont>
#include <QTimeZone>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QLayout>
#include <QClipboard>

#include "macro.h"
#include "misc.h"
#include "BillPage.h"
#include "ui_billpage.h"
#include "goopal.h"
#include "debug_log.h"
#include "commondialog.h"
#include "showcontentdialog.h"
#include "chooseupgradedialog.h"
#include "control/remarkcellwidget.h"
#include "datamgr.h"
#include "frame.h"

BillPage::BillPage(QString name, QWidget *parent) :
	QWidget(parent),
	accountName(name),
	transactionType(0),
	address(""),
	ui(new Ui::BillPage)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;
	ui->setupUi(this);
	setAutoFillBackground(true);

	QPalette palette;
	palette.setColor(QPalette::Background, QColor(235, 235, 235));
	setPalette(palette);

	if (accountName.isEmpty())
	{
		// 如果是点击账单跳转
		emit back();
		return;
	}

	ui->toBrowserBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/browser.png"));
	ui->toBrowserBtn->setStyleSheet(QLatin1String("QToolButton{background-color:#31bdc6;color:#ffffff;border:none;border-radius:3px;}"
		"QToolButton:hover{background-color:#62a9f8;}"));

	ui->pageLineEdit->setStyleSheet("QLineEdit{background:transparent;border-width:0;border-style:outset}");
	ui->pageLineEdit->setText("1");
	QIntValidator *validator = new QIntValidator(1, 9999, this);
	ui->pageLineEdit->setValidator(validator);
	ui->pageLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
	ui->pageLineEdit->setDisabled(true);
	currentPageIndex = 1;
	ui->pageLineEdit->setText(QString::number(currentPageIndex));

	ui->prePageBtn->setStyleSheet("QToolButton:!hover{border:0px;color:#999999;} QToolButton:hover{border:0px;color:#469cfc;}");
	ui->nextPageBtn->setStyleSheet("QToolButton:!hover{border:0px;color:#999999;} QToolButton:hover{border:0px;color:#469cfc;}");

	ui->accountComboBox->setStyleSheet(QString(
		"QComboBox {"
		"border: 1px solid gray;"
		"border-radius: 3px;"
		"width: 72px;"
		"height: 20px;"
		"}"
		"QComboBox::drop-down {"
		"subcontrol-origin: padding;"
		"subcontrol-position: top right;"
		"width: 20px;"
		"border-left-width: 0px;"
		"border-top-right-radius: 3px;"
		"border-bottom-right-radius: 3px;"
		"}"
		"QComboBox::down-arrow {"
		"image: url(%1pic2/assetComboxArrow.png);"
		"}"
		).arg(DataMgr::getDataMgr()->getWorkPath()));

	// 账户下拉框按字母顺序排序
	QStringList keys = DataMgr::getInstance()->getAccountInfo()->keys();
	ui->accountComboBox->addItems(keys);
	ui->accountComboBox->setCurrentText(accountName);


	ui->assetComboBox->setStyleSheet(QString(
		"QComboBox {"
		"border: 1px solid gray;"
		"border-radius: 3px;"
		"width: 72px;"
		"height: 20px;"
		"}"
		"QComboBox::drop-down {"
		"subcontrol-origin: padding;"
		"subcontrol-position: top right;"
		"width: 20px;"
		"border-left-width: 0px;"
		"border-top-right-radius: 3px;"
		"border-bottom-right-radius: 3px;"
		"}"
		"QComboBox::down-arrow {"
		"image: url(%1pic2/assetComboxArrow.png);"
		"}"
		).arg(DataMgr::getDataMgr()->getWorkPath()));

	const QString& currencyName = DataMgr::getInstance()->getCurrCurrencyName();
	const auto& currenyList = DataMgr::getInstance()->getCurrencyList();
	int itemIdx = -1;
	for (int i = 0; i < currenyList.size(); i++)
	{
		ui->assetComboBox->addItem(currenyList.at(i).name, QVariant(currenyList.at(i).id));
		if (itemIdx == -1 && currencyName == currenyList.at(i).name)
		{
			itemIdx = i;
		}
	}
	if (itemIdx != -1)
	{
		ui->assetComboBox->setCurrentIndex(itemIdx);
	}

	ui->accountTransactionsTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
	ui->accountTransactionsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->accountTransactionsTableWidget->setFocusPolicy(Qt::NoFocus);
	ui->accountTransactionsTableWidget->setColumnWidth(0, 160);
	ui->accountTransactionsTableWidget->setColumnWidth(1, 250);
	ui->accountTransactionsTableWidget->setColumnWidth(2, 190);
	ui->accountTransactionsTableWidget->setColumnWidth(3, 188);
	ui->accountTransactionsTableWidget->setShowGrid(false);
	ui->accountTransactionsTableWidget->setStyleSheet("QTableWidget{background:transparent;} QTableView::item { border-bottom: 1px dashed rgb(180,180,180); }");
	ui->accountTransactionsTableWidget->setFrameShape(QFrame::NoFrame);
	ui->accountTransactionsTableWidget->setMouseTracking(true);

	ui->initLabel->hide();

	clearTransactionsTableWidget();
	updateAccountBalance();
	updateTransactions();
	setAssertType();

	inited = true;

	Frame* frame = dynamic_cast<Frame*>(Goopal::getInstance()->mainFrame);
	connect(frame, &Frame::updateAccountBalance, this, &BillPage::updateAccountBalance);
	connect(DataMgr::getInstance()->getTokenTrxConnect(), &TokenTransaction::tokenTrxRequestEnd, this, &BillPage::displayTokenTrxVector);

	DLOG_QT_WALLET_FUNCTION_END;
}

QString discard(const QString &str)
{
	int dotPos = str.indexOf(".");

	if (dotPos != -1) {
		return str.left(dotPos + 3);

	}
	else {
		DLOG_QT_WALLET(" no dot!");
		return NULL;
	}
}

BillPage::~BillPage() {
	DLOG_QT_WALLET_FUNCTION_BEGIN;
	delete ui;
	DLOG_QT_WALLET_FUNCTION_END;
}

void BillPage::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBrush(QColor(255, 255, 255));
	painter.setPen(QPen(QColor(187, 187, 187), 2));
	painter.drawRect(QRect(0, 0, 826, 532));
}

void BillPage::on_accountComboBox_currentIndexChanged(const QString &arg)
{
	if (inited)
	{
		accountName = arg;
		currentPageIndex = 1;
		ui->pageLineEdit->setText(QString::number(currentPageIndex));
		DataMgr::getInstance()->setCurrentAccount(accountName);

		clearTransactionsTableWidget();
		updateTransactions();
		updateAccountBalance();
	}
}

void BillPage::on_assetComboBox_currentIndexChanged(int index)
{
	if (inited)
	{
		int currencyId = ui->assetComboBox->itemData(index).toInt();
		const CurrencyInfo& currencyInfo = DataMgr::getInstance()->getCurrencyById(currencyId);

		if (currencyInfo.id != DataMgr::getCurrentCurrency().id)
		{
			DataMgr::getInstance()->setCurrentCurrency(currencyInfo);
			setAssertType();

			clearTransactionsTableWidget();
			updateTransactions();
			updateAccountBalance();
		}
	}
}

void BillPage::retranslator()
{
	ui->retranslateUi(this);
}

void BillPage::updateAccountBalance()
{
	QString amount("0");
	QString asset_type = DataMgr::getCurrCurrencyName();

	if (DataMgr::getInstance()->getAccountInfo()->isEmpty() == false)
	{
		amount = DataMgr::getInstance()->getAccountInfo()->value(accountName).getBalance(asset_type);
	}

	ui->balanceLabel->setText("<body><font style=\"font-size:18px\" color=#000000>" + checkZero(amount.toDouble()) + "</font><font style=\"font-size:12px\" color=#000000> "
		+ asset_type + "</font></body>");
}

void BillPage::displayTokenTrxVector()
{
	int i = 0;
	int size = DataMgr::getInstance()->getTokenTrxConnect()->trxvector.totaRecords;
	int currentPage = DataMgr::getInstance()->getTokenTrxConnect()->trxvector.currentPage;
	int totalPage = DataMgr::getInstance()->getTokenTrxConnect()->trxvector.totalPage;
	ui->prePageBtn->show();
	ui->numberLabel->show();
	ui->pageLineEdit->show();
	ui->pageLabel->show();
	ui->nextPageBtn->show();
	ui->numberLabel->setText(tr("total ") + QString::number(size) + tr(" ,"));
	ui->pageLineEdit->setText(QString::number(currentPage));

	int trxSize = DataMgr::getInstance()->getTokenTrxConnect()->trxvector.trx.size();
	if (currentPage == 1 && trxSize == 0)
		ui->initLabel->show();
	else
		ui->initLabel->hide();

	ui->accountTransactionsTableWidget->setRowCount(trxSize);
	ui->pageLabel->setText("/" + QString::number(totalPage));

	const QString& currencyName = DataMgr::getInstance()->getCurrCurrencyName();

	for (Transaction trx : DataMgr::getInstance()->getTokenTrxConnect()->trxvector.trx)
	{
		ui->accountTransactionsTableWidget->setRowHeight(i, 35);
		QString fromAddr = trx.from_addr;
		QString toAddr = trx.to_addr;
		QString fromAccount = DataMgr::getInstance()->getAddrAccont(fromAddr);
		QString toAccount = DataMgr::getInstance()->getAddrAccont(toAddr);
		QString current_addr = DataMgr::getInstance()->getAccountAddr(accountName);

		QDateTime time = QDateTime::fromString(trx.trx_time, "yyyy-MM-dd hh:mm:ss");
		time.setTimeZone(QTimeZone::utc());
		time = time.toTimeZone(QTimeZone::systemTimeZone());
		QString currentDateTime = time.toString("yyyy-MM-dd hh:mm:ss");
		ui->accountTransactionsTableWidget->setItem(i, 0, new QTableWidgetItem(currentDateTime));

		QTableWidgetItem* item;
		bool isOut = false;
		bool isSelf = false;

		if ((fromAddr == toAddr) && (fromAddr == current_addr)) {
			isSelf = true;
		}

		if (fromAddr != current_addr) {
			// 如果 fromaccount 不为本账户，则 为对方账户
			if (fromAccount.isEmpty()) {
				item = new QTableWidgetItem(fromAddr);
			}
			else {
				item = new QTableWidgetItem(fromAccount);
				item->setTextColor(QColor(64, 154, 255));
			}
		}
		else {
			// 如果 fromaccount 为本账户， 则toaccount  为对方账户
			if (toAccount.isEmpty()) {
				item = new QTableWidgetItem(toAddr);
			}
			else {
				item = new QTableWidgetItem(toAccount);
				item->setTextColor(QColor(64, 154, 255));
			}
			isOut = true;
		}
		ui->accountTransactionsTableWidget->setItem(i, 1, item);

		QString showAmount;

		if (currencyName == COMMONASSET && currencyName != trx.cointype)
		{
			showAmount = QString("-" + checkZero(trx.fee.toDouble()));
			QTableWidgetItem* item = new QTableWidgetItem(showAmount);
			item->setTextColor(QColor(255, 80, 63));
			ui->accountTransactionsTableWidget->setItem(i, 2, item);

			ui->accountTransactionsTableWidget->setItem(i, 3, new QTableWidgetItem("--"));
		}
		else
		{
			if (isSelf) {
				showAmount = QString("-/+" + checkZero(trx.amount.toDouble()));
				QTableWidgetItem* item = new QTableWidgetItem(showAmount);
				item->setTextColor(QColor(255, 80, 63));
				ui->accountTransactionsTableWidget->setItem(i, 2, item);
			}
			else {
				// amount
				if (isOut) {
					showAmount = QString("-" + checkZero(trx.amount.toDouble()));
					QTableWidgetItem* item = new QTableWidgetItem(showAmount);
					item->setTextColor(QColor(255, 80, 63));
					ui->accountTransactionsTableWidget->setItem(i, 2, item);
				}
				else {
					showAmount = QString("+" + checkZero(trx.amount.toDouble()));
					QTableWidgetItem* item = new QTableWidgetItem(showAmount);
					item->setTextColor(QColor(71, 178, 156));
					ui->accountTransactionsTableWidget->setItem(i, 2, item);
				}
			}

			//fee
			if (isOut)
				ui->accountTransactionsTableWidget->setItem(i, 3, new QTableWidgetItem(checkZero(trx.fee.toDouble())));
			else
				ui->accountTransactionsTableWidget->setItem(i, 3, new QTableWidgetItem("--"));
		}

		i++;
	}
}

void BillPage::clearTransactionsTableWidget()
{
	DataMgr::getInstance()->getTokenTrxConnect()->clearTrxVector();
	ui->accountTransactionsTableWidget->clear();
}

void BillPage::updateTransactions()
{
	const QString& contractId = DataMgr::getInstance()->getCurrentCurrency().contractId;
	const QString& account_address = DataMgr::getInstance()->getAccountInfo()->value(accountName).address;
	DataMgr::getInstance()->getTokenTrxConnect()->connectToBlockBrower(account_address, contractId, currentPageIndex);
}

void BillPage::on_prePageBtn_clicked() {
	ui->accountTransactionsTableWidget->scrollToTop();

	if (currentPageIndex == 1)
		return;

	currentPageIndex--;
	clearTransactionsTableWidget();
	updateTransactions();
	ui->pageLineEdit->setText(QString::number(currentPageIndex));

	CurrencyInfo currencyInfo = DataMgr::getInstance()->getCurrentCurrency();
	if (currencyInfo.isAsset() == false) {
		ui->prePageBtn->hide();
		ui->numberLabel->hide();
		ui->pageLineEdit->hide();
		ui->pageLabel->hide();
		ui->nextPageBtn->hide();
	}
}

void BillPage::on_nextPageBtn_clicked() {
	int totalPageNum = ui->pageLabel->text().remove("/").toInt();

	if (currentPageIndex >= totalPageNum)
		return;

	currentPageIndex++;
	clearTransactionsTableWidget();
	updateTransactions();
	ui->pageLineEdit->setText(QString::number(currentPageIndex));
	ui->accountTransactionsTableWidget->scrollToTop();

	CurrencyInfo currencyInfo = DataMgr::getInstance()->getCurrentCurrency();
	if (currencyInfo.isAsset() == false) {
		ui->prePageBtn->hide();
		ui->numberLabel->hide();
		ui->pageLineEdit->hide();
		ui->pageLabel->hide();
		ui->nextPageBtn->hide();
	}
}

void BillPage::on_pageLineEdit_editingFinished()
{
	if (ui->pageLineEdit->text().isEmpty()) {
		ui->pageLineEdit->setText(QString::number(currentPageIndex));
	}

	currentPageIndex = ui->pageLineEdit->text().toInt();
}

void BillPage::on_pageLineEdit_textEdited(const QString &arg1)
{
	if (arg1.isEmpty()) {
		return;
	}

	if (arg1.at(0) == '0') {
		ui->pageLineEdit->setText(arg1.mid(1));
		return;
	}

	int totalPageNum = ui->pageLabel->text().remove("/").toInt();

	if (arg1.toInt() > totalPageNum) {
		ui->pageLineEdit->setText(QString::number(totalPageNum));
		return;
	}
}

void BillPage::setAssertType()
{
	QString curText = ui->label_12->text();
	int sep = curText.indexOf('(');
	if (sep > 0) {
		ui->label_12->setText(curText.left(sep + 1) + DataMgr::getCurrCurrencyName() + ")");
	}

	curText = ui->label_14->text();
	sep = curText.indexOf('(');
	if (sep > 0) {
		ui->label_14->setText(curText.left(sep + 1) + DataMgr::getCurrCurrencyName() + ")");
	}
}

void BillPage::on_toBrowserBtn_clicked()
{
	QDesktopServices::openUrl(QUrl(ACHAIN_BROWSER_URL));
}
