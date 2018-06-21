#include <QDebug>

#include "titlebar.h"
#include "ui_titlebar.h"
#include "debug_log.h"
#include <QPainter>
#include "setdialog.h"
#include "noticedialog.h"
#include "goopal.h"
#include "newsdialog.h"
#include "commondialog.h"
#include "datamgr.h"

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    ui->setupUi(this);
    setAutoFillBackground(true);

	ui->logoLabel->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/logo.png"));
	ui->logoLabel->setScaledContents(true);

	ui->noticeBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/email_btn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()));
	ui->setupBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/set_btn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()));

    ui->minBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/minimize.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                              "QToolButton:hover{background-image:url(%2pic2/minimize_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));
    ui->closeBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/close.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                "QToolButton:hover{background-image:url(%2pic2/close_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));


    ui->divLineLabel->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/divLine.png"));
    ui->divLineLabel->setScaledContents(true);

	ui->noticeBtn->hide();

    DLOG_QT_WALLET_FUNCTION_END;
}

TitleBar::~TitleBar()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    delete ui;

    DLOG_QT_WALLET_FUNCTION_END;
}

void TitleBar::on_minBtn_clicked()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

	if (false)
    {
        emit tray();
    }
    else
    {  
        emit minimum();
    }

    DLOG_QT_WALLET_FUNCTION_END;
}

void TitleBar::on_closeBtn_clicked()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

	if (false)
    {
        emit tray();
    }
    else
    {

		if (DataMgr::getInstance()->showPrivateTips) {
			CommonDialog commonDialog(CommonDialog::OkOnly);
			commonDialog.setText(tr("Keep your private key properly before closing Achain Wallet Lite"));
			commonDialog.showTips();
			bool choice = commonDialog.pop();

			if(choice) {
				DataMgr::getInstance()->showPrivateTips = false;
				DataMgr::getInstance()->getSettings()->setValue("/settings/showPrivateTips", false);

			} else {
				DataMgr::getInstance()->showPrivateTips = true;
			}
		}

		CommonDialog commonDialog(CommonDialog::OkAndCancel);
		commonDialog.setText( tr( "Close Achain Wallet Lite?"));
		bool choice = commonDialog.pop();

		if( choice) {
			emit closeGOP();
		} else {
			return;
		}
    }

    DLOG_QT_WALLET_FUNCTION_END;
}

void TitleBar::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBrush(QColor(229, 229, 229));
	painter.setPen(QPen(QColor(187, 187, 187), 2));
    painter.drawRect(QRect(0, 0, 960, 50));
}

void TitleBar::on_setupBtn_clicked()
{
    SetDialog setDialog;
    connect(&setDialog,SIGNAL(settingSaved()),this,SLOT(saved()));
    setDialog.pop();
}

void TitleBar::saved()
{
    emit settingSaved();
}

void TitleBar::retranslator()
{
    ui->retranslateUi(this);
}

void TitleBar::on_noticeBtn_clicked()
{
	NoticeDialog noticeDialog;
	noticeDialog.pop();
}
