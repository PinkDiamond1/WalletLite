#include "noticedialog.h"
#include "ui_noticedialog.h"

#include <QFile>
#include <QDateTime>

#include "goopal.h"
#include "datamgr.h"
#include "macro.h"

NoticeDialog::NoticeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NoticeDialog)
{
    ui->setupUi(this);

	setParent(Goopal::getInstance()->mainFrame);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setWindowFlags(Qt::FramelessWindowHint);

	ui->bgWidget->setObjectName("bgwidget");
	ui->bgWidget->setStyleSheet("#bgwidget {background-color:rgba(10, 10, 10,100);}");

	ui->containerWidget->setObjectName("containerwidget");
	ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

	ui->closeBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/close4.png"));
	ui->closeBtn->setStyleSheet("border:none");

	webview = new QWebEngineView(this);
	webview->setGeometry(140, 110, 670, 430);

	QDateTime datetime = QDateTime::currentDateTime();
	QString datetimeStr = datetime.toString("hh:mm:ss.zzz");

	QString url(CDN_URL);
	QString lang = DataMgr::getInstance()->getLanguage();
	if (lang == "English")
		url = url + "wallet_notice/notice" + "_en" + ".html?time=" + datetimeStr;
	else if (lang == "Simplified Chinese")
		url = url + "wallet_notice/notice" + "_cn" + ".html?time=" + datetimeStr;

	webview->setUrl(QUrl(url));
	webview->showMaximized();
	webview->show();
}

NoticeDialog::~NoticeDialog()
{
	if (webview != nullptr)
	{
		webview->deleteLater();
		webview = nullptr;
	}
    delete ui;
}

void NoticeDialog::pop()
{
	move(0, 0);
	exec();
}

void NoticeDialog::on_closeBtn_clicked()
{
	close();
}
