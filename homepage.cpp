#include "homepage.h"
#include <math.h> 

#include <QPainter>
#include <QHBoxLayout>
#include <QPalette>
#include <QScrollBar>
#include <QToolButton>

#include "ui_homepage.h"
#include "assetslistview.h"
#include "datamgr.h"
#include "frame.h"
#include "goopal.h"
#include "assetslistmodel.h"
#include "assetslistdelegate.h"
#include "thirddatamgr.h"

#define	BTN_ANI_FPS  40.0

HomePage::HomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomePage)
{
    ui->setupUi(this);

	ui->currencyTypeBox->setStyleSheet(QString(
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
	
	scrollAreaWidth = ui->scrollArea->geometry().width();
	scrollAreaHeight = ui->scrollArea->geometry().height();

	chartColorVec.resize(5);
	chartColorVec.clear();
	chartColorVec.push_back(QColor(49, 189, 198));
	chartColorVec.push_back(QColor(158, 219, 239));
	chartColorVec.push_back(QColor(117, 121, 245));
	chartColorVec.push_back(QColor(210, 213, 232));
	chartColorVec.push_back(QColor(83, 89, 121));

	initView();

	inited = true;

	connect(ui->scrollArea, &HomePageScrollArea::scrollMoveEvent, this, &HomePage::scrollMoveEvent);

	Frame* frame = dynamic_cast<Frame*>(Goopal::getInstance()->mainFrame);
	connect(frame, &Frame::updateAccountBalance, this, &HomePage::updateBalance);
}

HomePage::~HomePage()
{
	if (upBtnAniEffect != nullptr)
		delete upBtnAniEffect;

	if (downBtnAniEffect != nullptr)
		delete downBtnAniEffect;

    delete ui;
}

void HomePage::initView()
{
	QPalette whitePalette;
	whitePalette.setColor(QPalette::Background, Qt::white);

	ui->scrollArea->setFrameShape(QFrame::NoFrame);
	ui->scrollAreaWidgetContents->setPalette(whitePalette);

	auto currencyList = ThirdDataMgr::getInstance()->getDisplayCurrencyList();
	for (int i = 0; i < currencyList.size(); i++)
	{
		ui->currencyTypeBox->insertItem(i, currencyList[i]);
	}
	ui->currencyTypeBox->setCurrentText(ThirdDataMgr::getInstance()->getCurrentCurrencyName());

	//init pie chart
	pieSeries = new QPieSeries();
	pieSeries->setHoleSize(0.36);

	chart = new QChart();
	chart->legend()->hide();
	chart->addSeries(pieSeries);
	chart->createDefaultAxes();
	chart->legend()->setVisible(false);
	chart->legend()->setAlignment(Qt::AlignBottom);
	chart->setAcceptHoverEvents(true);
	chart->setBackgroundVisible(false);
	chart->legend()->setVisible(true);

	chartView = new QChartView(chart, ui->scrollAreaWidgetContents);
	chartView->setGeometry(0, -22, scrollAreaWidth, 500);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->setPalette(whitePalette);

	auto totalAssetsTitle = new QLabel(chartView);
	totalAssetsTitle->setGeometry(QRect((scrollAreaWidth - 200) / 2, 198, 200, 20));
	totalAssetsTitle->setFont(QFont("Microsoft Yahei", 10));
	totalAssetsTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	totalAssetsTitle->setText(tr("Total Value"));

	totalAssetsLabel = new QLabel(chartView);
	totalAssetsLabel->setGeometry(QRect((scrollAreaWidth - 200) / 2, 222, 200, 20));
	totalAssetsLabel->setFont(QFont("Microsoft Yahei", 10));
	totalAssetsLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QToolButton* hiddenTotalBtn = new QToolButton(chartView);
	hiddenTotalBtn->setGeometry(QRect((scrollAreaWidth - 20) / 2, 250, 20, 14));
	hiddenTotalBtn->setCursor(QCursor(Qt::PointingHandCursor));
	hiddenTotalBtn->setFocusPolicy(Qt::NoFocus);
	hiddenTotalBtn->setStyleSheet("background:transparent;");
	hiddenTotalBtn->setIconSize(QSize(20, 14));
	hiddenTotalBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/eye.png"));
	connect(hiddenTotalBtn, &QToolButton::clicked, this, &HomePage::on_hiddenTotalBtn_clicked);

	//init list title
	auto assetNameTitle = new QLabel(ui->scrollAreaWidgetContents);
	assetNameTitle->setGeometry(QRect(104, 528, 200, 24));
	assetNameTitle->setFont(QFont("Microsoft Yahei", 11));
	assetNameTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	assetNameTitle->setText(tr("Assets"));

	auto amountTitle = new QLabel(ui->scrollAreaWidgetContents);
	amountTitle->setGeometry(QRect(232, 528, 200, 24));
	amountTitle->setFont(QFont("Microsoft Yahei", 11));
	amountTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	amountTitle->setText(tr("Amount"));

	priceTitle = new QLabel(ui->scrollAreaWidgetContents);
	priceTitle->setGeometry(QRect(395, 528, 200, 24));
	priceTitle->setFont(QFont("Microsoft Yahei", 11));
	priceTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	auto currencyName = ThirdDataMgr::getInstance()->getCurrentCurrencyName();
	priceTitle->setText(tr("Value") + "/" + currencyName);

	auto rateTitle = new QLabel(ui->scrollAreaWidgetContents);
	rateTitle->setGeometry(QRect(592, 528, 200, 24));
	rateTitle->setFont(QFont("Microsoft Yahei", 11));
	rateTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	rateTitle->setText(tr("Percentage"));

	//segmenting line
	auto segLine = new QLabel(ui->scrollAreaWidgetContents);
	segLine->setGeometry(QRect(54, 558, 716, 2));
	segLine->setStyleSheet("background-color:rgb(220,220,220);");

	//init list view
	assetsListModel = new AssetsListModel(this);
	connect(assetsListModel, SIGNAL(listHeightChanged(int)), this, SLOT(onListHeightChanged(int)));

	AssetsListDelegate* delegate = new AssetsListDelegate(this);

	assetsListView = new AssetsListView(ui->scrollAreaWidgetContents);
	assetsListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	assetsListView->move(0, 560);
	connect(assetsListView, SIGNAL(clicked(const QModelIndex &)), assetsListModel, SLOT(collapse(const QModelIndex&)));

	assetsListView->setModel(assetsListModel);
	assetsListView->setItemDelegate(delegate);

	ui->label->raise();
	ui->currencyTypeBox->raise();

	ui->forwardDownBtn->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/forward_btn.png"));
	ui->forwardDownBtn->setStyleSheet("background:transparent");
	ui->forwardDownBtn->raise();

	QMatrix matrix;
	matrix.rotate(180);
    const QPixmap& pixmap = QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/forward_btn.png");
    const QPixmap& fitpixmap = pixmap.transformed(matrix, Qt::SmoothTransformation);
	ui->forwardUpBtn->setIcon(QIcon(fitpixmap));
	ui->forwardUpBtn->setStyleSheet("background:transparent");
	ui->forwardUpBtn->raise();
	ui->forwardUpBtn->hide();

	downBtnAniEffect = new QGraphicsOpacityEffect();
	ui->forwardDownBtn->setGraphicsEffect(downBtnAniEffect);
	downBtnAniEffect->setOpacity(btnAniOpacity);

	upBtnAniEffect = new QGraphicsOpacityEffect();
	ui->forwardUpBtn->setGraphicsEffect(upBtnAniEffect);
	upBtnAniEffect->setOpacity(btnAniOpacity);
	
	startBtnAni(downBtnAniEffect);

	updateBalance();
}

void HomePage::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setBrush(QColor(255, 255, 255));
	painter.setPen(QPen(QColor(187, 187, 187), 2));
	painter.drawRect(QRect(0, 0, 826, 532));
}

void HomePage::updateBalance()
{
	refreshAssetslistView();
	refreshPieChart();
}

void HomePage::refreshAssetslistView()
{
	auto accountInfos = DataMgr::getInstance()->getAccountInfo();
	assetsListModel->setAccountInfo(accountInfos);
	assetsListModel->refresh();

	assetsListView->show();
}

struct AssetTotalPrice
{
	std::string asset_name;
	double price;
};

bool sortAssetTotalPrice(const AssetTotalPrice& v1, const AssetTotalPrice& v2)
{
	if (v1.asset_name == COMMONASSET)
		return true;
	else if (v2.asset_name == COMMONASSET)
		return false;

	return v1.price > v2.price;
}

void HomePage::refreshPieChart()
{
	auto itemDataList = assetsListModel->getItemDataList();
	std::vector<AssetTotalPrice> assetTotalPriceVec;
	totalAssetVal = 0;

	for (int i = 0; i < itemDataList->size(); i++)
	{
		auto& itemData = itemDataList->at(i);
		AssetTotalPrice assetTotalPrice = { itemData->asset_name.toStdString(), itemData->price };
		assetTotalPriceVec.push_back(assetTotalPrice);

		if (itemData->price > 0)
			totalAssetVal += itemData->price;
	}
	std::sort(assetTotalPriceVec.begin(), assetTotalPriceVec.end(), sortAssetTotalPrice);
	
	pieSeries->clear();

	if (totalAssetVal > 0)
	{
		int count = assetTotalPriceVec.size() > 5 ? 5 : assetTotalPriceVec.size();
		double othersPrice = totalAssetVal;

		for (int i = 0; i < count; i++)
		{
			auto& assetTotalPrice = assetTotalPriceVec[i];
			double price = assetTotalPrice.price >= 0 ? assetTotalPrice.price : 0;
			othersPrice -= price;
			pieSeries->append(QString::fromStdString(assetTotalPrice.asset_name), price / totalAssetVal);
			pieSeries->slices().at(i)->setColor(chartColorVec[i]);
		}

		pieSeries->append(tr("Others"), othersPrice / totalAssetVal);
		pieSeries->slices().at(count)->setColor(QColor(239, 124, 166));
	}
	else
	{
		pieSeries->append("ACT", 100);
		pieSeries->slices().at(0)->setColor(QColor(238, 238, 238));
		pieSeries->append(tr("Others"), 0);
		pieSeries->slices().at(1)->setColor(QColor(239, 124, 166));
	}

	setTotalAssetVal(totalAssetVal);

	chartView->show();
}

void HomePage::setTotalAssetVal(double val)
{
	QString currencySymbol = ThirdDataMgr::getInstance()->getCurrentCurrencySymbol();
	if (DataMgr::getDataMgr()->isHiddenTotalAsset())
	{
		totalAssetsLabel->setText("*********" + currencySymbol);
	}
	else
	{
		totalAssetsLabel->setText(doubleToStr(val) + currencySymbol);
	}
}

void HomePage::onListHeightChanged(int height)
{
	assetsListView->resize(scrollAreaWidth, height);
	ui->scrollAreaWidgetContents->resize(scrollAreaWidth, 600 + height);
	ui->forwardUpBtn->move(ui->forwardUpBtn->x(), 80 + height);
}

void HomePage::on_currencyTypeBox_currentIndexChanged(const QString &arg)
{
	if (inited)
	{
		ThirdDataMgr::getInstance()->setCurrentCurrencyName(arg);
		priceTitle->setText(tr("Value") + "/" + arg);
		updateBalance();
	}
}

void HomePage::on_forwardDownBtn_clicked()
{
	int val = ui->scrollArea->verticalScrollBar()->maximum();
	ui->scrollArea->verticalScrollBar()->setValue(val);
}

void HomePage::on_forwardUpBtn_clicked()
{
	ui->scrollArea->verticalScrollBar()->setValue(10);
	ui->scrollArea->verticalScrollBar()->setValue(0);
}

void HomePage::scrollMoveEvent(HomePageScrollArea::MoveEvent e)
{
	if (e == HomePageScrollArea::REACH_TOP)
	{
		ui->forwardDownBtn->move(384, ui->forwardDownBtn->y());
		startBtnAni(downBtnAniEffect);
	}
	else if (e == HomePageScrollArea::LEAVE_TOP)
	{
		ui->forwardDownBtn->move(2000, ui->forwardDownBtn->y());
		stopBtnAni();
	}
	else if (e == HomePageScrollArea::REACH_BOTTOM)
	{
		ui->forwardUpBtn->show();
		startBtnAni(upBtnAniEffect);
	}
	else if (e == HomePageScrollArea::LEAVE_BOTTOM)
	{
		ui->forwardUpBtn->hide();
		stopBtnAni();
	}
}

void HomePage::startBtnAni(QGraphicsOpacityEffect* effect)
{
	aniBtnEffect = effect;

	if (btnAniTimer == nullptr)
	{
		btnAniTimer = new QTimer();
		connect(btnAniTimer, SIGNAL(timeout()), this, SLOT(btnAniUpdate()));
	}
	btnAniTimer->start(1000.0 / BTN_ANI_FPS);
}

void HomePage::stopBtnAni()
{
	btnAniTimer->stop();
}

void HomePage::btnAniUpdate()
{
	if (aniBtnEffect != nullptr)
	{
		btnAniTime += 1.0 / BTN_ANI_FPS;
		btnAniOpacity = sin(btnAniTime * 3.1415926);
		btnAniOpacity = btnAniOpacity < 0 ? -btnAniOpacity : btnAniOpacity;

		aniBtnEffect->setOpacity(btnAniOpacity);
	}
}

void HomePage::on_hiddenTotalBtn_clicked()
{
	DataMgr::getInstance()->changeHiddenTotalAssetState();
	setTotalAssetVal(totalAssetVal);
}
