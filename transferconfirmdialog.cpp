#include "transferconfirmdialog.h"
#include "ui_transferconfirmdialog.h"
#include "goopal.h"
#include "debug_log.h"
#include "datamgr.h"

TransferConfirmDialog::TransferConfirmDialog(QString address, QString amount, QString fee, QString remark, QWidget *parent) :
    QDialog(parent),
    address(address),
    amount(amount),
    fee(fee),
    yesOrNo(false),
    ui(new Ui::TransferConfirmDialog)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    ui->setupUi(this);

	setParent(Goopal::getInstance()->mainFrame);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");
	if (address.length() >= 60) {
		address = address.mid(0, 60) + "\r\n" + address.mid(60, address.length());
	} else {
		ui->addressLabel->setGeometry(QRect(240, 100, 521, 21));
	}
	ui->addressLabel->setText(address);
	ui->addressLabel->setMaximumWidth(480);
    ui->amountLabel->setText( "<body><B>" + amount + "</B>" + " "+DataMgr::getCurrCurrencyName()+"</body>");
    ui->feeLabel->setText( "<body><font color=#409AFF>" + fee + "</font>" + " ACT</body>");
    ui->okBtn->setStyleSheet("QToolButton{background-color:#4dc8cf;color:#ffffff;border:0px solid rgb(64,153,255);border-radius:3px;}"
                             "QToolButton:hover{background-color:#7dd9df;}"
                             "QToolButton:disabled{background-color:#cecece;}");
    ui->okBtn->setText(tr("Ok"));
    ui->cancelBtn->setStyleSheet("QToolButton{background-color:#ffffff;color:#282828;border:1px solid #7dd9df;border-radius:3px;}"
                                 "QToolButton:hover{color:#7dd9df;}");
    ui->cancelBtn->setText(tr("Cancel"));
    ui->pwdLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->pwdLineEdit->setPlaceholderText( tr("Enter login password"));
    ui->pwdLineEdit->setTextMargins(8,0,0,0);
    ui->pwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->pwdLineEdit->setFocus();
    if( amount.toDouble() < 1000) {
        ui->pwdLabel->hide();
        ui->pwdLineEdit->hide();
    } else {
        ui->okBtn->setEnabled(false);
    }
	
    DLOG_QT_WALLET_FUNCTION_END;
}

TransferConfirmDialog::~TransferConfirmDialog()
{
    delete ui;
}

bool TransferConfirmDialog::pop()
{
    ui->pwdLineEdit->grabKeyboard();
    move(0,0);
    exec();

    return yesOrNo;
}

void TransferConfirmDialog::on_okBtn_clicked()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    if( amount.toDouble() < 1000)
    {
        yesOrNo = true;
        close();

    } else {

        if(ui->pwdLineEdit->text().isEmpty())
			return;

        QString password = ui->pwdLineEdit->text();

        if (password.size() < 8) {
            ui->tipLabel1->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/wrong.png"));
            ui->tipLabel2->setText("<body><font style=\"font-size:12px\" color=#FF8880>" + tr("At least 8 letters!") + "</font></body>" );
            return;
		}

		bool validPassword = DataMgr::getInstance()->walletCheckPassphrase(password);

		if (validPassword) {
			yesOrNo = true;
			close();
		}
		else {
			ui->tipLabel1->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/wrong.png"));
			ui->tipLabel2->setText("<body><font style=\"font-size:12px\" color=#FF8880>" + tr("Wrong password!") + "</font></body>");
			return;
		}
    }

    DLOG_QT_WALLET_FUNCTION_END;
}

void TransferConfirmDialog::on_cancelBtn_clicked()
{
    yesOrNo = false;
    close();
}

void TransferConfirmDialog::on_pwdLineEdit_textChanged(const QString &arg1)
{
    if( arg1.indexOf(' ') > -1) {
        ui->pwdLineEdit->setText( ui->pwdLineEdit->text().remove(' '));
        return;
    }

    if( arg1.isEmpty()) {
        ui->okBtn->setEnabled(false);
    } else {
        ui->okBtn->setEnabled(true);
    }
}
