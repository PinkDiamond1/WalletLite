#include "importdialog.h"
#include "ui_importdialog.h"
#include "goopal.h"
#include "commondialog.h"
#include "namedialog.h"
#include "datamgr.h"

#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QFile>

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    setParent(Goopal::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");

    ui->containerWidget->setObjectName("containerWidget");
    ui->containerWidget->setStyleSheet("#containerWidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

    ui->importBtn->setEnabled(false);

    ui->importBtn->setStyleSheet("QToolButton{background-color:#4dc8cf;color:#ffffff;border:0px solid rgb(64,153,255);border-radius:3px;}"
                                 "QToolButton:hover{background-color:#7dd9df;}"
                                 "QToolButton:disabled{background-color:#cecece;}");
    ui->cancelBtn->setStyleSheet("QToolButton{background-color:#ffffff;color:#282828;border:1px solid #7dd9df;border-radius:3px;}"
                                 "QToolButton:hover{color:#7dd9df;}");

    ui->privateKeyLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->privateKeyLineEdit->setTextMargins(8,0,0,0);
    ui->privateKeyLineEdit->setFocus();

    connect(DataMgr::getInstance(), &DataMgr::onWalletImportPrivateKey, this, &ImportDialog::walletImportPrivateKey);
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::pop()
{
    move(0,0);
    exec();
}

void ImportDialog::on_cancelBtn_clicked()
{
    close();
}

void ImportDialog::on_pathBtn_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Choose your private key file."),"","(*.gkey)");
#ifdef WIN32
    file.replace("/","\\");
#endif // WIN32 //zxlrun
    ui->privateKeyLineEdit->setText(file);
}

void ImportDialog::on_importBtn_clicked()
{
    QString privatekey;

    // 如果输入框中是 私钥文件的地址
	if (ui->privateKeyLineEdit->text().contains(".gkey"))
	{
		QFile file(ui->privateKeyLineEdit->text());
        if( file.open(QIODevice::ReadOnly))
		{
            QByteArray ba = QByteArray::fromBase64( file.readAll());
            privatekey = QString(ba);
            file.close();
        }
		else
		{
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Wrong file path."));
            commonDialog.pop();
            return;
        }
    }
	else
	{
        // 如果输入框中是 私钥
        privatekey = ui->privateKeyLineEdit->text();
    }

    ui->importBtn->setEnabled(false);

    QString accountName;

    if(!DataMgr::getInstance()->walletCheckPriKeyValid(privatekey))
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("Illegal private key!"));
        commonDialog.pop();

        ui->importBtn->setEnabled(true);
    }
    else if(!DataMgr::getInstance()->walletCheckPriKeyExists(privatekey, &accountName))
    {
        NameDialog nameDialog;
		
		QString formerName = DataMgr::getInstance()->walletGetFormerNameByPriKey(privatekey);
		nameDialog.setName(formerName);
        QString name = nameDialog.pop();

        if (name.isEmpty())
        {
            ui->importBtn->setEnabled(true);
            return;
        }

        DataMgr::getInstance()->walletImportPrivateKey(privatekey, name);
    }
    else
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(accountName + tr(" already existes"));
        commonDialog.pop();
    }
}

void ImportDialog::walletImportPrivateKey(bool result)
{
    ui->importBtn->setEnabled(true);

    if(result) {
		emit accountImported();

		CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr(" Import success!    "));
        commonDialog.pop();

		close();

    } else {
		CommonDialog commonDialog(CommonDialog::OkOnly);
		commonDialog.setText( tr("Illegal private key!"));
		commonDialog.pop();
	}
}

bool ImportDialog::accountNameAlreadyExisted(QString name)
{
	return !DataMgr::getInstance()->getAccountAddr(name).isEmpty();
}

void ImportDialog::on_privateKeyLineEdit_textChanged(const QString &arg1)
{
    QString str = arg1.simplified();
    if( str.isEmpty()) {
        ui->importBtn->setEnabled(false);
    } else {
        ui->importBtn->setEnabled(true);
    }
}

void ImportDialog::on_privateKeyLineEdit_returnPressed()
{
    if( ui->importBtn->isEnabled()) {
        on_importBtn_clicked();
    }
}


