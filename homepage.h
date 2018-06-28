#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QChartView>
#include <QLabel>
#include <QPieSeries>
#include <QChart>
#include <QChartView>
#include <QGraphicsOpacityEffect>
#include <QTimer>

#include "homepagescrollarea.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class HomePage;
}

class AssetsListView;
class AssetsListModel;

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = 0);
    ~HomePage();

private:
	void initView();
	void paintEvent(QPaintEvent*);
	void refreshAssetslistView();
	void refreshPieChart();
	void startBtnAni(QGraphicsOpacityEffect* effect);
	void stopBtnAni();
	void setTotalAssetVal(double val);

public slots:
	void refreshPage();
	void onListHeightChanged(int height);
	void scrollMoveEvent(HomePageScrollArea::MoveEvent e);
	void btnAniUpdate();

private slots:
    void on_currencyTypeBox_currentIndexChanged(const QString &arg);
    void on_forwardDownBtn_clicked();
	void on_forwardUpBtn_clicked();
	void hiddenTotalBtn_clicked();

private:
	Ui::HomePage *ui;
	bool inited = false;

	QPieSeries *pieSeries = nullptr;
	QChart *chart = nullptr;
	QChartView* chartView = nullptr;
	QLabel* totalAssetsLabel = nullptr;
	QLabel* priceTitle = nullptr;
	AssetsListView* assetsListView = nullptr;

	int scrollAreaWidth = 0;
	int scrollAreaHeight = 0;

	AssetsListModel* assetsListModel = nullptr;
	QTimer* btnAniTimer = nullptr;
	QGraphicsOpacityEffect* upBtnAniEffect = nullptr;
	QGraphicsOpacityEffect* downBtnAniEffect = nullptr;
	QGraphicsOpacityEffect* aniBtnEffect = nullptr;
	double btnAniTime = 0.0;
	double btnAniOpacity = 0.0;

	QVector<QColor> chartColorVec;

	double totalAssetVal = 0.0;
};

#endif // HOMEPAGE_H
