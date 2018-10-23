#include "accountdetailwidget.h"
#include "ui_accountdetailwidget.h"
#include "../exportdialog.h"
#include "../commondialog.h"
#include "../chooseupgradedialog.h"
#include "../extra/dynamicmove.h"
#include "datamgr.h"

#include <QDateTime>
#include <QDebug>
#include <QClipboard>
#include <QTimeZone>

#define ACT_FEE (0.01)

AccountDetailWidget::AccountDetailWidget( QWidget *parent) :
    QWidget(parent),
    accountName(""),
    salary(0),
    produceOrNot(false),
    ui(new Ui::AccountDetailWidget) {
    ui->setupUi(this);
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(246, 247, 249));
    setPalette(palette);

	QString language = DataMgr::getInstance()->getLanguage();
    if(language == "English") {
        delegateLabelString = DataMgr::getDataMgr()->getWorkPath() + "pic2/delegateLabel_En.png";
        registeredLabelString = DataMgr::getDataMgr()->getWorkPath() + "pic2/registeredLabel_En.png";
    } else {
        delegateLabelString = DataMgr::getDataMgr()->getWorkPath() + "pic2/delegateLabel.png";
        registeredLabelString = DataMgr::getDataMgr()->getWorkPath() + "pic2/registeredLabel.png";
    }

    ui->closeBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/close4.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()));
    QLabel* exportBtnLabel = new QLabel(ui->exportBtn);
    exportBtnLabel->setGeometry(55, 5, 13, 13);
    exportBtnLabel->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/export_icon.png"));
    exportBtnLabel->show();
    ui->exportBtn->setStyleSheet("QToolButton{background-color:#ffffff;color:rgb(123,123,123);border:1px solid rgb(221,221,221);border-radius:3px;}"
                                 "QToolButton:hover{color:rgb(150,150,150);}");
    ui->copyBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/copyBtn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                       "QToolButton:hover{background-image:url(%2pic2/copyBtn_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));
    ui->copyBtn->setToolTip(tr("copy to clipboard"));
    ui->upgradeBtn->setStyleSheet("QToolButton{background-color:#4dc8cf;color:#ffffff;border:0px solid rgb(64,153,255);border-radius:3px;}"
                                  "QToolButton:hover{background-color:#7dd9df;}"
                                  "QToolButton:disabled{background-color:#cecece;}");

    ui->produceWidget->setObjectName("produceWidget");
    ui->produceWidget->setStyleSheet("#produceWidget{background-color:#ffffff;color:rgb(123,123,123);border:1px solid rgb(221,221,221);border-radius:3px;}");

    qrCodeWidget = new QRCodeWidget(this);
    qrCodeWidget->setGeometry(215, 145, 100, 100);
    ui->upgradeBtn->hide();
}

AccountDetailWidget::~AccountDetailWidget() {
    delete ui;
}

void AccountDetailWidget::setAccount(QString name) {
    if (accountName != name) { // 如果accountname 没改 produceornot保留原值
        produceOrNot = false;
    }
    
    if (name.isEmpty()) {
        return;
    }
    
    accountName = name;
    QString showName;
    
    if (accountName.size() > 13) {
        showName = accountName.left(11) + "...";
        
    } else {
        showName = accountName;
    }
    
    ui->nameLabel->setText(showName);
    ui->nameLabel->adjustSize();
    ui->nameLabel->move((this->width() - ui->nameLabel->width()) / 2, 90);
    ui->identityLabel->move(ui->nameLabel->x() - 25, ui->nameLabel->y());
    ui->delegateRankLabel->move(ui->nameLabel->x() + ui->nameLabel->width() + 5, ui->nameLabel->y() - 5);

    QString address = DataMgr::getInstance()->getAccountInfo()->value(accountName).address;
    ui->addressLabel->setText(address);
    ui->addressLabel->adjustSize();
    ui->addressLabel2->adjustSize();
    ui->addressLabel->move((this->width() - ui->addressLabel->width()) / 2, 300);
#ifdef WIN32
    ui->addressLabel2->move(ui->addressLabel->x() - 50, ui->addressLabel->y());
#else
    ui->addressLabel2->move(ui->addressLabel->x() - 55, ui->addressLabel->y());
#endif
    ui->copyBtn->move(ui->addressLabel->x() + ui->addressLabel->width() + 9, ui->addressLabel->y() - 1);

    qrCodeWidget->setString(address);

    QString asset_type = DataMgr::getCurrCurrencyName();
    QString balance = DataMgr::getInstance()->getAccountInfo()->value(accountName).getBalance(asset_type);

    if (DataMgr::getInstance()->getCurrCurrencyName() == "ACT") {
        if (balance.toDouble() < 0.010009 - 0.000001) {
            ui->upgradeBtn->setEnabled(false);
        } else if (balance.toDouble() < 0.010009 + 0.000001) {
            ui->upgradeBtn->setEnabled(true);
        } else {
            ui->upgradeBtn->setEnabled(true);
        }
    }
    else {
		QString balanceACT = DataMgr::getInstance()->getAccountInfo()->value(accountName).getBalance("ACT");
        balanceACT.remove(" ACT");
        if (balanceACT.toDouble() < 0.019 - 0.000001) {
            ui->upgradeBtn->setEnabled(false);

        } else if (balanceACT.toDouble() < 0.019 + 0.000001) {
            ui->upgradeBtn->setEnabled(true);

        } else if (balance.toDouble() < 0.000009 - 0.000001) {
            ui->upgradeBtn->setEnabled(false);

        } else if (balance.toDouble() < 0.000009 + 0.000001) {
            ui->upgradeBtn->setEnabled(true);
        } else {
            ui->upgradeBtn->setEnabled(true);
        }
    }

    ui->produceWidget->hide();
    ui->delegateRankLabel->hide();
}

void AccountDetailWidget::dynamicShow()
{
	show();
	DynamicMove* dynamicMove = new DynamicMove(this, QPoint(297, 93), 1000, this);
	dynamicMove->start();
}

void AccountDetailWidget::dynamicHide()
{
	DynamicMove* dynamicMove = new DynamicMove(this, QPoint(826, 93), 1000, this);
	connect(dynamicMove, SIGNAL(moveEnd()), this, SLOT(moveEnd()));
    dynamicMove->start();
}

void AccountDetailWidget::on_closeBtn_clicked() {
    emit back();
}

void AccountDetailWidget::on_exportBtn_clicked() {
    ExportDialog exportDialog(accountName);
    exportDialog.pop();
}

void AccountDetailWidget::on_copyBtn_clicked() {
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->setText(ui->addressLabel->text());
    CommonDialog commonDialog(CommonDialog::OkOnly);
    commonDialog.setText(tr("Copy to clipboard"));
    commonDialog.pop();
}

void AccountDetailWidget::on_upgradeBtn_clicked() {
    // 如果是已注册账户
    {
        ChooseUpgradeDialog* chooseUpgradeDialog = new ChooseUpgradeDialog(accountName, this);
        chooseUpgradeDialog->move( ui->upgradeBtn->mapToGlobal( QPoint(-40, 30) ) );
        connect( chooseUpgradeDialog, SIGNAL(upgrade(QString)), this, SIGNAL(upgrade(QString)));
        connect( chooseUpgradeDialog, SIGNAL(applyDelegate(QString)), this, SIGNAL(applyDelegate(QString)));
        chooseUpgradeDialog->exec();
    }
}

void AccountDetailWidget::moveEnd() {
    // 移出mainpage显示范围的时候 最后一帧会导致mainpage挡住bottombar
    // 先隐藏 dynamicshow时再显示
    hide();
}
