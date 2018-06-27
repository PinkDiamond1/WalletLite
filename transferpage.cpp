#include <QDebug>
#include <QPainter>
#include <QTextCodec>
#include <QDir>
#include <QClipboard>

#ifdef WIN32 
#include <windows.h>
#endif

#include "macro.h"
#include "datamgr.h"
#include "transferpage.h"
#include "ui_transferpage.h"
#include "debug_log.h"
#include "gop_common_define.h"
#include "contactdialog.h"
#include "remarkdialog.h"
#include "commondialog.h"
#include "transferconfirmdialog.h"
#include "waitingpage.h"
#include "goopal.h"
#include "achainlightwalletapi.h"
#include "frame.h"

TransferPage::TransferPage(QString name,QWidget *parent) :
    QWidget(parent),
    accountName(name),
    inited(false),
    transfer_time(0),
    waitingPage(nullptr),
    ui(new Ui::TransferPage)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    ui->setupUi(this);
    setAutoFillBackground(true);

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


	if (accountName.isEmpty())  // 如果是点击账单跳转
	{
		if (DataMgr::getInstance()->getAccountInfo()->size() > 0) {
			accountName = DataMgr::getInstance()->getAccountInfo()->keys().at(0);
		}
		else {
			// 如果还没有账户
			emit back();    // 跳转在functionbar  这里并没有用
			return;
		}
	}

	// 账户下拉框按字母顺序排序
	QStringList keys = DataMgr::getInstance()->getAccountInfo()->keys();
	ui->accountComboBox->addItems(keys);
	if (accountName.isEmpty()) {
		ui->accountComboBox->setCurrentIndex(0);
	}
	else {
		ui->accountComboBox->setCurrentText(accountName);
	}

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


	QString address = DataMgr::getInstance()->getAccountInfo()->value(accountName).address;
    ui->addressLabel->setText(address);

    ui->amountLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->amountLineEdit->setTextMargins(8,0,0,0);

    QRegExp rx1("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{1,5})?$|(^\\t?$)");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
    ui->amountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->sendtoLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->sendtoLineEdit->setTextMargins(8,0,0,0);
    QRegExp regx("[a-zA-Z0-9\-\.\ \n]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->sendtoLineEdit->setValidator( validator );
    ui->sendtoLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->tipLabel3->hide();
    ui->tipLabel4->hide();
    ui->tipLabel5->hide();

    ui->tipLabel5->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/wrong.png"));

    ui->sendBtn->setStyleSheet("QToolButton{background-color:#4dc8cf;color:#ffffff;border:0px solid rgb(64,153,255);border-radius:3px;}"
                               "QToolButton:hover{background-color:#7dd9df;}"
                               "QToolButton:disabled{background-color:#cecece;}");
    ui->sendBtn->setText(tr("Transfer"));

    ui->copyBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/copyBtn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
        "QToolButton:hover{background-image:url(%2pic2/copyBtn_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));

#ifdef WIN32
    ui->copyBtn->move(ui->addressLabel->x() + ui->addressLabel->text().length() * 10 + 18, 140);
#else
    ui->copyBtn->move(ui->addressLabel->x() + ui->addressLabel->text().length() * 7.5 + 18, 140);
#endif
    ui->copyBtn->setToolTip(tr("copy to clipboard"));

	inited = true;

    connect(DataMgr::getInstance(), &DataMgr::onWalletTransferToAddressWithId, this, &TransferPage::walletTransferToAddress);
	connect(DataMgr::getDataMgr(), &DataMgr::onCallContract, this, &TransferPage::tokenTransferTo);
	connect(DataMgr::getInstance(), &DataMgr::onWalletCheckAddress, this, &TransferPage::walletCheckAddress);

	Frame* frame = dynamic_cast<Frame*>(Goopal::getInstance()->mainFrame);
	connect(frame, &Frame::updateAccountBalance, this, &TransferPage::updateBalance);

	showBalance();
	setAssertType();
	checkAmountValid(ui->amountLineEdit->text().toDouble());

    DLOG_QT_WALLET_FUNCTION_END;
}

void TransferPage::setbalanceLabel(QString balance, QString token)
{
    ui->balanceLabel->setText("<body><font style=\"font-size:18px\" color=#000000>" + balance + "</font><font style=\"font-size:12px\" color=#000000>" +token+ "</font></body>" );
}

TransferPage::~TransferPage()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    delete ui;
    DLOG_QT_WALLET_FUNCTION_END;
}

void TransferPage::walletCheckAddress(QString address)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    address = address.mid(address.indexOf(":") + 1, address.length());

    if (DataMgr::getInstance()->walletCheckAddress(address))
	{
		QString amount_text(ui->amountLineEdit->text());
        QString account_text(ui->sendtoLineEdit->text());

		if (amount_text.toDouble() > -0.0000001 && amount_text.toDouble() < 0.0000001) {
			CommonDialog tipDialog(CommonDialog::OkOnly);
			tipDialog.setText(tr("The amount can not be 0"));
			tipDialog.pop();
			return;
        }

        //检查费用是否够
        QString strBalanceTemp = DataMgr::getInstance()->getAccountInfo()->value(accountName).getBalance(COMMONASSET);
        double dBalance = strBalanceTemp.toDouble();
        CurrencyInfo currencyInfo = DataMgr::getInstance()->getCurrentCurrency();

        if (currencyInfo.isAsset() == false)
		{
            if (dBalance < 0.05) {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText(tr("The ACT amount must be greater than 0.05 ."));
                tipDialog.pop();
                return;
            }
        }
		else
		{
            if (dBalance < 0.01) {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText(tr("The ACT amount must be greater than 0.01 ."));
                tipDialog.pop();
                return;
            }
        }

		QString fee_text = "0.01";
        TransferConfirmDialog transferConfirmDialog(account_text, amount_text, fee_text, "");
		bool yOrN = transferConfirmDialog.pop();

		if (yOrN) {
			CurrencyInfo currencyInfo = DataMgr::getInstance()->getCurrentCurrency();
			if (currencyInfo.isAsset())
			{
				DataMgr::getInstance()->walletTransferToAddressWithId(amount_text, currencyInfo.assetId(), accountName, account_text, "");
                qDebug() << "walletTransferToAddressWithId" + accountName;
                showWaitingPage();
			}
			else {
				DataMgr::getInstance()->tokenTransferTo(accountName, currencyInfo.name, account_text, amount_text.toDouble(), 0.03, "");
                showWaitingPage();
			}
        }
	}
	else
	{
		CommonDialog tipDialog(CommonDialog::OkOnly);
		tipDialog.setText(tr("Wrong address!"));
		tipDialog.pop();
	}
    DLOG_QT_WALLET_FUNCTION_END;
}

void TransferPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    if(!inited)  return;

    accountName = arg1;
    DataMgr::getInstance()->setCurrentAccount(accountName);

	QString address = DataMgr::getInstance()->getAccountInfo()->value(accountName).address;
    ui->addressLabel->setText(address);

	showBalance();
	checkAmountValid(ui->amountLineEdit->text().toDouble());

    DLOG_QT_WALLET_FUNCTION_END;
}

void TransferPage::on_sendBtn_clicked()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    if(ui->sendtoLineEdit->text().size() == 0) {      
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText( tr("Input Address."));
        tipDialog.pop();
        return;
    }

    if (ui->amountLineEdit->text().size() == 0) {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText(tr("Please enter the amount."));
        tipDialog.pop();
        return;
    }

	//验证地址有效性
    QString address = ui->sendtoLineEdit->text();
    walletCheckAddress(address);

    DLOG_QT_WALLET_FUNCTION_END;
}

void TransferPage::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBrush(QColor(255, 255, 255));
	painter.setPen(QPen(QColor(187, 187, 187), 2));
	painter.drawRect(QRect(0, 0, 826, 532));
}

void TransferPage::on_amountLineEdit_textChanged(const QString &arg1)
{
    double amount = ui->amountLineEdit->text().toDouble();
	checkAmountValid(amount);
}

void TransferPage::checkAmountValid(double amount)
{
	double fee = 0.01f;

	QString asset_type = DataMgr::getCurrCurrencyName();
	QString strBalanceTemp = DataMgr::getInstance()->getAccountInfo()->value(accountName).getBalance(asset_type);
	double dBalance = strBalanceTemp.remove(",").toDouble();

	if (asset_type == COMMONASSET && amount + fee > dBalance + 0.000001) {
		ui->tipLabel3->show();
		ui->tipLabel5->show();
		ui->sendBtn->setEnabled(false);
	}
	else if (amount > dBalance) {
		ui->tipLabel3->show();
		ui->tipLabel5->show();
		ui->sendBtn->setEnabled(false);
	}
	else {
		ui->tipLabel3->hide();
		ui->tipLabel5->hide();
		ui->sendBtn->setEnabled(true);
	}
}

void TransferPage::on_sendtoLineEdit_textChanged(const QString &arg1)
{
    if( ui->sendtoLineEdit->text().contains(" ") || ui->sendtoLineEdit->text().contains("\n")) { 
		// 不判断就remove的话 右键菜单撤销看起来等于不能用
        ui->sendtoLineEdit->setText( ui->sendtoLineEdit->text().simplified().remove(" "));
    }

//    ui->sendtoLineEdit->setText( ui->sendtoLineEdit->text().remove("\n"));
    if( ui->sendtoLineEdit->text().isEmpty() || ui->sendtoLineEdit->text().mid(0,3) == "ACT") {
        ui->tipLabel4->hide();
        return;
    }

    if( ui->sendtoLineEdit->text().toInt() == 0)   // 不能是纯数字
    {
		//Goopal::getInstance()->postRPC( toJsonFormat( "id_blockchain_get_account_" + ui->sendtoLineEdit->text(), "blockchain_get_account", QStringList() << ui->sendtoLineEdit->text() ));
    } else {
        ui->tipLabel4->setText(tr("Invalid address"));
        ui->tipLabel4->show();
    }
}

void TransferPage::tokenTransferTo(QString result)
{
	if (result.mid(0, 18) == "\"result\":{\"index\":") {
		CommonDialog tipDialog(CommonDialog::OkOnly);
		tipDialog.setText(tr("The transfer request has been submitted."));
		tipDialog.pop();
		emit showBillPage();
	} else {

	}
}

QString TransferPage::getCurrentAccount()
{
    return accountName;
}

void TransferPage::setAddress(QString address)
{
    ui->sendtoLineEdit->setText(address);
}

void TransferPage::walletTransferToAddress(bool resp_success, QString id, QString msg)
{
    hideWaitingPage();

    if(resp_success)
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText(tr("The transfer request has been submitted."));
        tipDialog.pop();
		emit recreateTransferPage();
    }
    else
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
		if (msg.size() > 0)
		{
			for (int i = msg.size() - 1; i >= 0; i--)
			{
                if (msg.at(i) == '\n')
					msg.remove(i, 1);
				else
					break;
			}
			msg = "\n" + msg;
		}

		tipDialog.setText(tr("Transaction sent failed") + msg);
        tipDialog.pop();
    }
}

void TransferPage::updateBalance()
{
	showBalance();
}

void TransferPage::showBalance()
{
    QString asset_type = DataMgr::getCurrCurrencyName();
    QString balance = DataMgr::getInstance()->getAccountInfo()->value(accountName).getBalance(asset_type);

    if (!balance.isEmpty()){
        setbalanceLabel(checkZero(balance.toDouble()), " "+ DataMgr::getInstance()->getCurrCurrencyName());
    } else {
        setbalanceLabel("0", " " + DataMgr::getInstance()->getCurrCurrencyName());
    }
}

void TransferPage::on_copyBtn_clicked()
{
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->setText(ui->addressLabel->text());

    CommonDialog commonDialog(CommonDialog::OkOnly);
    commonDialog.setText(tr("Copy to clipboard"));
    commonDialog.pop();
}

void TransferPage::setAssertType()
{
    ui->unitLabel->setText(DataMgr::getCurrCurrencyName());

	if (DataMgr::getInstance()->getCurrentCurrency().isAsset())
		ui->tipLabel2->setText(tr("Trx fee is") + " 0.01 ACT");
	else
	{
		QString tip = tr("Trx fee is") + " 0.01 ~ 0.05 ACT\n";
		ui->tipLabel2->setText(tip + tr("Poundage is estimated, please refer to the actual occurrence."));
	}
}

void TransferPage::showWaitingPage()
{
    if(waitingPage == nullptr)
    {
        waitingPage = new WaitingPage(Goopal::getInstance()->mainFrame);
        waitingPage->move(0, 0);
        waitingPage->show();
    }
}

void TransferPage::hideWaitingPage()
{
    if(waitingPage != nullptr)
    {
        waitingPage->close();
        delete waitingPage;
        waitingPage = nullptr;
    }
}

void TransferPage::on_assetComboBox_currentIndexChanged(int index)
{
	if (!inited)
		return;

	int currencyId = ui->assetComboBox->itemData(index).toInt();
	const CurrencyInfo& currencyInfo = DataMgr::getInstance()->getCurrencyById(currencyId);

	if (currencyInfo.id != DataMgr::getCurrentCurrency().id)
	{
		DataMgr::getInstance()->setCurrentCurrency(currencyInfo);
		setAssertType();
		showBalance();
		checkAmountValid(ui->amountLineEdit->text().toDouble());
	}
}
