#include "quotationpage.h"

#include <QPainter>
#include <QTimer>
#include <QScrollBar>

#include "ui_quotationpage.h"
#include "quotationlistview.h"
#include "thirddatamgr.h"
#include "datamgr.h"
#include "quotationlistmodel.h"

QuotationPage::QuotationPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QuotationPage)
{
	ui->setupUi(this);

	ui->nameBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/zhangfu.png"));
	ui->nameBtn->setStyleSheet("border:none");

	ui->sortPriceBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/zhangfu.png"));
	ui->sortPriceBtn->setStyleSheet("border:none");

	ui->sortChangeRateBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/zhangfu.png"));
	ui->sortChangeRateBtn->setStyleSheet("border:none");

	ui->sortChangeBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/zhangfu.png"));
	ui->sortChangeBtn->setStyleSheet("border:none");

	sortColumn = Column::NONE;
	sortOrder[1] = SortOrder::ASC;
	sortOrder[2] = SortOrder::ASC;
	sortOrder[3] = SortOrder::DESC;
	sortOrder[4] = SortOrder::DESC;

	quotationListView = new QuotationListView(this);
	quotationListView->setGeometry(40, 58, 784, 450);

	refreshQuotation();

	dataFromLabel = new QLabel(this);
	dataFromLabel->setGeometry(QRect(300, 507, 463, 24));
	dataFromLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	dataFromLabel->setFont(QFont("Microsoft Yahei", 10));
	QPalette palette;
	palette.setColor(QPalette::WindowText, QColor(153, 153, 153));
	dataFromLabel->setPalette(palette);

	retranslator(DataMgr::getInstance()->getLanguage());
}

QuotationPage::~QuotationPage()
{
    delete ui;
}

void QuotationPage::retranslator(QString language)
{
	ui->retranslateUi(this);

    const QString& dataFromText = QString(tr("Tihs data comes from %1, only for reference.")).arg("https://coinmarketcap.com");
	dataFromLabel->setText(dataFromText);
}

void QuotationPage::sortList(Column col)
{
	if (sortColumn == col)
	{
		sortOrder[col] = (sortOrder[col] == SortOrder::ASC) ? SortOrder::DESC : SortOrder::ASC;
		quotationListView->sortList(col, sortOrder[col]);
	}
	else
	{
		quotationListView->sortList(col, sortOrder[col]);
		sortColumn = col;
	}
	//quotationListView->verticalScrollBar()->setValue(10);
	quotationListView->verticalScrollBar()->setValue(0);
}

void QuotationPage::on_nameBtn_clicked()
{
	sortList(Column::NAME);
}

void QuotationPage::on_sortPriceBtn_clicked()
{
	sortList(Column::PRICE);
}

void QuotationPage::on_sortChangeRateBtn_clicked()
{
	sortList(Column::PERCENT_RATE);
}

void QuotationPage::on_sortChangeBtn_clicked()
{
	sortList(Column::PRICE_CHANGE);
}

void QuotationPage::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setBrush(QColor(255, 255, 255));
	painter.setPen(QPen(QColor(187, 187, 187), 2));
	painter.drawRect(QRect(0, 0, 826, 532));

	painter.setBrush(QColor(245, 245, 245));
	painter.setPen(Qt::PenStyle::NoPen);
	painter.drawRect(QRect(27, 25, 780, 29));
}

void QuotationPage::refreshQuotation()
{
	auto quotationInfos = ThirdDataMgr::getInstance()->getQuotationInfoList();
	quotationListView->setQuotationInfos(quotationInfos);
	quotationListView->refreshSortList();
	quotationListView->refresh();
}
