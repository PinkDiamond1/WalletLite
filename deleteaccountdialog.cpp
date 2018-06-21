#include "deleteaccountdialog.h"
#include "ui_deleteaccountdialog.h"
#include "goopal.h"
#include "debug_log.h"
#include "datamgr.h"

#include <QDebug>
#include <QFocusEvent>

#include "waitingpage.h"

DeleteAccountDialog::DeleteAccountDialog(QString name , QWidget *parent) :
    QDialog(parent),
    accountName(name),
    yesOrNo( false),
    ui(new Ui::DeleteAccountDialog)
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

	auto tip = QString(tr("You will want to hide the account \"%1\".\nKeep your private key in mind for easy retrieval.")).arg(accountName);
	ui->tipLabel->setText(tip);

    ui->pwdLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->pwdLineEdit->setPlaceholderText( tr("Login password"));
    ui->pwdLineEdit->setTextMargins(8,0,0,0);
    ui->pwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->okBtn->setStyleSheet("QToolButton{background-color:#4dc8cf;color:#ffffff;border:0px solid rgb(64,153,255);border-radius:3px;}"
                             "QToolButton:hover{background-color:#7dd9df;}"
                             "QToolButton:disabled{background-color:#cecece;}");
    ui->okBtn->setText(tr("Ok"));

    ui->cancelBtn->setStyleSheet("QToolButton{background-color:#ffffff;color:#282828;border:1px solid #7dd9df;border-radius:3px;}"
                                 "QToolButton:hover{color:#7dd9df;}");
    ui->cancelBtn->setText(tr("Cancel"));

    ui->okBtn->setEnabled(false);
    ui->pwdLineEdit->setFocus();

    DLOG_QT_WALLET_FUNCTION_END;
}

DeleteAccountDialog::~DeleteAccountDialog()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    delete ui;
    DLOG_QT_WALLET_FUNCTION_END;
}

void DeleteAccountDialog::on_okBtn_clicked()
{
	const QString& password = ui->pwdLineEdit->text();
	if (password.isEmpty())
		return;

	bool ret = DataMgr::getInstance()->walletCheckPassphrase(password);
	if (ret)
	{
		yesOrNo = DataMgr::getInstance()->walletAccountDelete(accountName);
		close();
	}
	else
	{
		ui->tipLabel1->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/wrong.png"));
		ui->tipLabel2->setText("<body><font style=\"font-size:12px\" color=#FF8880>" + tr("Wrong Password!") + "</font></body>");
	}
}

void DeleteAccountDialog::on_cancelBtn_clicked()
{
    yesOrNo = false;
    close();
}

void DeleteAccountDialog::on_pwdLineEdit_textChanged(const QString &arg1)
{
    if( arg1.indexOf(' ') > -1)
    {
        ui->pwdLineEdit->setText( ui->pwdLineEdit->text().remove(' '));
        return;
    }

    if( arg1.isEmpty())
    {
        ui->okBtn->setEnabled(false);
    }
    else
    {
        ui->okBtn->setEnabled(true);
    }

    ui->tipLabel1->setPixmap(QPixmap(""));
    ui->tipLabel2->setText("");
}

bool DeleteAccountDialog::pop()
{
    ui->pwdLineEdit->grabKeyboard();

    move(0,0);
    exec();

    return yesOrNo;
}

void DeleteAccountDialog::on_pwdLineEdit_returnPressed()
{
    on_okBtn_clicked();
}
