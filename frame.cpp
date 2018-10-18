#include "frame.h"

#include <QPainter>
#include <QLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>

#include <random>
#include <ctime>

#include "firstlogin.h"
#include "normallogin.h"
#include "billpage.h"
#include "accountpage.h"
#include "transferpage.h"
#include "bottombar.h"
#include "lockpage.h"
#include "titlebar.h"
#include "goopal.h"
#include "debug_log.h"
#include "functionbar.h"
#include "contactpage.h"
#include "upgradepage.h"
#include "applydelegatepage.h"
#include "selectgoppathwidget.h"
#include "control/shadowwidget.h"
#include "control/showbottombarwidget.h"
#include "database.h"
#include "datamgr.h"
#include "misc.h"
#include "macro.h"
#include "homepage.h"
#include "quotationpage.h"
#include "thirddatamgr.h"
#include "trackmgr.h"
#include "delegatespage.h"

#ifdef WIN32
#include "qt_windows.h"
#endif
#include <qmessagebox.h>

#define TRX_LOG   "TOKEN:SMC:TRANSLOG:201611111111:asd:afd:10123:END"
#define BALANCE_1 "TOKEN:SMC:BLC:asd:0:END"
#define BALANCE_2 "TOKEN:SMC:BLC:asd:0:END"

Frame::Frame():
	billPage(NULL),
	accountPage(NULL),
	transferPage(NULL),
	lockPage(NULL),
	timer(NULL),
	timerUpdate(NULL),
	titleBar(NULL),
	shadowWidget(NULL),
	timerForAutoRefresh(NULL),
	functionBar(NULL),
	mouse_press(false),
	currentPageId(-1),
	lastPage(""),
	quotatioPage(nullptr)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    setWindowFlags(Qt::Window|Qt::FramelessWindowHint |Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint);
#ifdef WIN32
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &this->rtConfined, 0);     //zxlwin
    ::GetWindowRect(::GetDesktopWindow(), &this->rtDefault);   //zxlwin
#endif // WIN32
    setFrameShape(QFrame::NoFrame);

    //Get default config language
	setLanguage(DataMgr::getInstance()->getLanguage());

	QDesktopWidget* desktopWidget = QApplication::desktop();
	const QRect& screenRect = desktopWidget->screenGeometry();
	int posX = (screenRect.width() - WALLET_WIDTH) / 2;
	int posY = (screenRect.height() - WALLET_HEIGHT) / 2;
	setGeometry(posX, posY, WALLET_WIDTH, WALLET_HEIGHT);

    shadowWidget = new ShadowWidget(this);
    shadowWidget->init(this->size());
    shadowWidget->hide();

	//放在托盘提示信息、托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon ->setToolTip(QString("Achain Wallet Lite ") + ACHAIN_WALLET_VERSION_STR);
    trayIcon ->setIcon(QIcon(DataMgr::getDataMgr()->getWorkPath() + "pic2/achain.ico"));
    //点击托盘执行的事件
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconIsActived(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
    createTrayIconActions();
    createTrayIcon();
    trayIconMenuEmpty = new QMenu(this);//zxlrun
    setStyleSheet("Frame{background-color:white; border: 4px solid #CCCCCC;border-radius:5px;}"
                  "QScrollBar:vertical"
                  "{"
                  "width:8px;"
                  "background:rgba(130,137,143,20%);"
                  "margin:0px,0px,0px,0px;"
                  "padding-top:2px;"
                  "padding-bottom:3px;"
                  "}"
                  "QScrollBar::handle:vertical"
                  "{"
                  "width:8px;"
                  "background:rgba(130,137,143,20%);"
                  " border-radius:4px;"
                  "min-height:20;"
                  "}"
                  "QScrollBar::handle:vertical:hover"
                  "{"
                  "width:8px;"
                  "background:rgba(130,137,143,100%);"
                  " border-radius:4px;"
                  "min-height:20;"
                  "}"
                  "QScrollBar::add-line:vertical"
                  "{"
                  "height:9px;width:8px;"
                  "border-image:url(:/images/a/3.png);"
                  "subcontrol-position:bottom;"
                  "}"
                  "QScrollBar::sub-line:vertical"
                  "{"
                  "height:9px;width:8px;"
                  "border-image:url(:/images/a/1.png);"
                  "subcontrol-position:top;"
                  "}"
                  "QScrollBar::add-line:vertical:hover"
                  "{"
                  "height:9px;width:8px;"
                  "border-image:url(:/images/a/4.png);"
                  "subcontrol-position:bottom;"
                  "}"
                  "QScrollBar::sub-line:vertical:hover"
                  "{"
                  "height:9px;width:8px;"
                  "border-image:url(:/images/a/2.png);"
                  "subcontrol-position:top;"
                  "}"
                  "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical"
                  "{"
                  "background:rgba(0,0,0,0%);"
                  "border-radius:4px;"
                  "}"
                 );

	if (DataBase::getInstance()->existUser())
		showLoginPage();
	else
        showFirstLoginPage();

	startAutoUpdate();

	connect(DataMgr::getDataMgr(), &DataMgr::onCallContract, this, &Frame::tokenTransferTo);
	connect(DataMgr::getDataMgr(), &DataMgr::onWalletAccountBalance, this, &Frame::walletAccountBalance);
	connect(DataMgr::getInstance(), &DataMgr::assetTypeGet, this, &Frame::assetTypeGet);
	connect(ThirdDataMgr::getInstance(), &ThirdDataMgr::onReqQuotationFinished, this, &Frame::reqQuotationFinished);

    DLOG_QT_WALLET_FUNCTION_END;
}

Frame::~Frame() {
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    qDebug() << "~Frame begin;";
    
    if (trayIcon) {
        delete trayIcon;
        trayIcon;
    }
    
    if (titleBar) {
        delete titleBar;
        titleBar = NULL;
    }
    
    if (timer) {
        delete timer;
        timer = NULL;
    }
    
    if (timerUpdate) {
        timerUpdate->stop();
        delete timerUpdate;
        timerUpdate = NULL;
    }

	if (timerForQuotation != nullptr)
	{
		timerForQuotation->stop();
		delete timerForQuotation;
		timerForQuotation = nullptr;
	}
    
    if (lockPage) {
        delete lockPage;
        lockPage = NULL;
    }
    
    if( functionBar) {
        delete functionBar;
        functionBar = NULL;
    }

    qDebug() << "~Frame end;";
    DLOG_QT_WALLET_FUNCTION_END;
}

void Frame::alreadyLogin()
{
	if (timer != nullptr) {
		timer->stop();
		delete timer;
	}
    timer = new QTimer(this); //  自动锁定
    connect(timer, SIGNAL(timeout()), this, SLOT(autoLock()));
    if(!DataMgr::getInstance()->notAutoLock) {
        timer->start(DataMgr::getInstance()->lockMinutes * 60000);
    }

	// 自动刷新
    startTimerForAutoRefresh();
	startTimerForQuotation();

	showTitleBar();
	showFunctionBar();
	showHomePage();

	TrackMgr::getInstance()->login();
}

void Frame::getAccountInfo()
{
	if (DataMgr::getInstance()->canRequestBalance())
	{
		if (DataMgr::getDataMgr()->getAccountInfo()->size() == 0)
			return;

		if (homePage != nullptr || accountPage != nullptr)
		{
			for (CommonAccountInfo account : *(DataMgr::getInstance()->getAccountInfo()))
				DataMgr::getDataMgr()->walletAccountBalance(account.name);

			timerForAutoRefresh->stop();
		}
		else if (billPage != nullptr || transferPage != nullptr)
		{
			const QString& currentAcount = DataMgr::getDataMgr()->getCurrentAccount();
			DataMgr::getDataMgr()->walletAccountBalance(currentAcount);

			if (billPage != nullptr)
			{
				billPage->updateTransactions();
			}

			timerForAutoRefresh->stop();
		}
	}
}

void Frame::reqQuotationFinished(bool firstTime)
{
	if (homePage != nullptr)
	{
		if (firstTime)
			homePage->refreshPage();
	}
	else if (accountPage != nullptr)
	{
		if (firstTime)
			accountPage->refreshPage();
	}
	else if (quotatioPage != nullptr)
	{
		quotatioPage->refreshQuotation();
	}
}

void Frame::showLockPage() {
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    timer->stop();
    walletLock();
    qDebug() << "lock ";
    DLOG_QT_WALLET_FUNCTION_END;
}
void Frame::autoLock() {
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    timer->stop();
    walletLock();
    qDebug() << "autolock ";
    DLOG_QT_WALLET_FUNCTION_END;
}
void writeLog(QString log) {
    QFile file("./Achain.log");
    
    //方式：Append为追加，WriteOnly，ReadOnly
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::critical(NULL, "提示", "无法创建文件");
        return;
    }
    
    QTextStream out(&file);
    out << log << endl;
    out.flush();
    file.close();
}

void Frame::runUpdate()
{
#ifdef WIN32
	QString app_path = QCoreApplication::applicationDirPath();
	//writeLog("app_path " + app_path);
	QString upPath = app_path + "/" + UPDATE_TOOL_NAME;
	QStringList args = QStringList() << "download" << DataMgr::getInstance()->getConfigPath();
	bool result = QProcess::startDetached(upPath, args);
	writeLog(result ? "successed" : "failed");
#endif
}

void Frame::startAutoUpdate()
{
	if (timerUpdate == nullptr)
	{
		timerUpdate = new QTimer(this);
		connect(timerUpdate, SIGNAL(timeout()), this, SLOT(runUpdate()));
		timerUpdate->start(15 * 60 * 1000);
	}
	runUpdate();
}

void Frame::unlock() {
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    if(!DataMgr::getInstance()->notAutoLock)
        timer->start(DataMgr::getInstance()->lockMinutes * 60000);

    titleBar->show();
    qDebug() << "lockPage " << lockPage;
    
    if( lockPage) {
        lockPage->close();
        lockPage = NULL;
    }
    
    qDebug() << "unlock showCurrentDialog";
    DLOG_QT_WALLET_FUNCTION_END;
}
void Frame::updateTimer() {
    if( timer != NULL && lockPage == NULL) {
        if(!DataMgr::getInstance()->notAutoLock) {
            timer->stop();
            timer->start(DataMgr::getInstance()->lockMinutes * 60000);
        }
    }
}
void Frame::settingSaved() {
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    
    if(DataMgr::getInstance()->notAutoLock)
        timer->stop();
    else
        timer->start( DataMgr::getInstance()->lockMinutes * 60000);

	setLanguage(DataMgr::getInstance()->getLanguage());

    DLOG_QT_WALLET_FUNCTION_END;
}

void Frame::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        mouse_press = true;
        //鼠标相对于窗体的位置（或者使用event->globalPos() - this->pos()）
        move_point = event->pos();
    }
    
#ifdef WIN32
    ::ClipCursor(&rtConfined);  //zxlwin
#endif // WIN32
}
void Frame::mouseMoveEvent(QMouseEvent *event) {
    //若鼠标左键被按下
    if(mouse_press) {
        //鼠标相对于屏幕的位置
        QPoint move_pos = event->globalPos();
        //移动主窗体位置
        this->move(move_pos - move_point);
    }
}
void Frame::mouseReleaseEvent(QMouseEvent *) {
    mouse_press = false;
#ifdef WIN32
    ::ClipCursor(&rtDefault);  //zxlwin
#endif // WIN32
}

void Frame::startTimerForAutoRefresh()
{
    if( timerForAutoRefresh != NULL) {
        timerForAutoRefresh->stop();
        delete timerForAutoRefresh;
    }
    
    timerForAutoRefresh = new QTimer(this);
    connect(timerForAutoRefresh, SIGNAL(timeout()), this, SLOT(autoRefresh()));
    timerForAutoRefresh->start(AUTO_REFRESH_TIME);
}

void Frame::startTimerForQuotation()
{
	if (timerForQuotation != nullptr) {
		timerForQuotation->stop();
		delete timerForQuotation;
	}

	timerForQuotation = new QTimer(this);
	connect(timerForQuotation, SIGNAL(timeout()), this, SLOT(autoRefreshQuotation()));
	timerForQuotation->start(30 * 60 * 1000);
}

void Frame::autoRefreshQuotation()
{
	ThirdDataMgr::getInstance()->requestQuotationInfo();
}

void Frame::closeCurrentPage()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    qDebug() << " closeCurrentPage :" << currentPageId;
    
    switch (currentPageId) {
        case 0:
			homePage->close();
			homePage = NULL;
            break;
        case 1:
			billPage->close();
			billPage = NULL;
            break;
		case 2:
			transferPage->close();
			transferPage = NULL;
            break;
		case 3:
			accountPage->close();
			accountPage = NULL;
			break;
		case 4:
			delegatesPage->close();
			delegatesPage = NULL;
			break;
		case 5:
			quotatioPage->close();
			quotatioPage = NULL;
            break;
        default:
            break;
    }
    
    DLOG_QT_WALLET_FUNCTION_END;
}

void Frame::autoRefresh()
{
	getAccountInfo();
}

void Frame::showFirstLoginPage()
{
    auto firstLogin = new FirstLogin(this);
	firstLogin->show();
    connect(firstLogin, SIGNAL(login()), this, SLOT(alreadyLogin()));
#ifdef WIN32
	connect(firstLogin, SIGNAL(minimum()), this, SLOT(showMinimized()));
#else
	connect(firstLogin, SIGNAL(minimum()), this, SLOT(hide()));
#endif // WIN32//zxlrun
	connect(firstLogin, SIGNAL(closeGOP()), this, SLOT(appQuit()));
	connect(firstLogin, SIGNAL(tray()), this, SLOT(hide()));
	connect(firstLogin, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(firstLogin, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));
}

void Frame::showLoginPage()
{
    auto normalLogin = new NormalLogin(this);
	normalLogin->show();
    connect(normalLogin, SIGNAL(login()), this, SLOT(alreadyLogin()));
#ifdef WIN32
	connect(normalLogin, SIGNAL(minimum()), this, SLOT(showMinimized()));
#else
	connect(normalLogin, SIGNAL(minimum()), this, SLOT(hide()));
#endif // WIN32//zxlrun
	connect(normalLogin, SIGNAL(closeGOP()), this, SLOT(appQuit()));
	connect(normalLogin, SIGNAL(tray()), this, SLOT(hide()));
	connect(normalLogin, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(normalLogin, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));
}

void Frame::showTitleBar()
{
	titleBar = new TitleBar(this);
	titleBar->move(0, 0);
#ifdef WIN32
	connect(titleBar, SIGNAL(minimum()), this, SLOT(showMinimized()));
#else
	connect(titleBar, SIGNAL(minimum()), this, SLOT(hide()));
#endif // WIN32//zxlrun
	connect(titleBar, SIGNAL(closeGOP()), this, SLOT(appQuit()));
	connect(titleBar, SIGNAL(tray()), this, SLOT(hide()));
	connect(titleBar, SIGNAL(settingSaved()), this, SLOT(settingSaved()));
	connect(titleBar, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(titleBar, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));
	titleBar->show();
}

void Frame::showFunctionBar()
{
	functionBar = new FunctionBar(this);
	functionBar->move(0, 48);

	connect(functionBar, SIGNAL(showHomePage()), this, SLOT(showHomePage()));
	connect(functionBar, SIGNAL(showBillPage(QString)), this, SLOT(showBillPage(QString)));
	connect(functionBar, SIGNAL(showTransferPage()), this, SLOT(showTransferPage()));
	connect(functionBar, SIGNAL(showAccountPage()), this, SLOT(showAccountPage()));
	connect(functionBar, SIGNAL(showDelegatesPage()), this, SLOT(showDelegatesPage()));
	connect(functionBar, SIGNAL(showQuotationPage()), this, SLOT(showQuotationPage()));

	connect(functionBar, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(functionBar, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));

	functionBar->show();
}

void Frame::showAccountPage()
{
	closeCurrentPage();

	accountPage = new AccountPage(this);
	accountPage->move(134, 48);
	accountPage->setAttribute(Qt::WA_DeleteOnClose);

	connect(accountPage, SIGNAL(openAccountPage(QString)), this, SLOT(showAccountPage()));
	connect(accountPage, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(accountPage, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));
	connect(accountPage, SIGNAL(refreshAccountInfo()), this, SLOT(getAccountInfo()));
	connect(accountPage, SIGNAL(showTransferPage(QString)), this, SLOT(showTransferPage(QString)));

	accountPage->show();
	currentPageId = 3;

	getAccountInfo();
}

void Frame::showTransferPage()
{
	const QString& accountName = DataMgr::getInstance()->getCurrentAccount();
	showTransferPage(accountName);
}

void Frame::showTransferPage(QString accountName)
{
    closeCurrentPage();

	transferPage = new TransferPage(accountName, this);
	transferPage->move(134, 48);
    transferPage->setAttribute(Qt::WA_DeleteOnClose);

    connect(transferPage, SIGNAL(accountChanged(QString)), this, SLOT(showTransferPage(QString)));
    connect(transferPage, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(transferPage, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));
	connect(transferPage, SIGNAL(recreateTransferPage()), this, SLOT(showTransferPage()));

	transferPage->show();
	currentPageId = 2;

	getAccountInfo();
}

void Frame::showDelegatesPage()
{
	closeCurrentPage();

	delegatesPage = new DelegatesPage(this);
	delegatesPage->move(134, 48);
	delegatesPage->setAttribute(Qt::WA_DeleteOnClose);
	delegatesPage->show();

	currentPageId = 4;
}

void Frame::showQuotationPage()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;
	closeCurrentPage();

	quotatioPage = new QuotationPage(this);
	quotatioPage->move(134, 48);
	quotatioPage->setAttribute(Qt::WA_DeleteOnClose);
	quotatioPage->show();

	currentPageId = 5;
	DLOG_QT_WALLET_FUNCTION_END;
}

void Frame::assetTypeGet()
{
	auto currencyList = DataMgr::getInstance()->getCurrencyList();
	if (currencyList.size() > 0)
	{
		DataMgr::getInstance()->setCurrentCurrency(currencyList[0]);
	}
}

void Frame::showHomePage()
{
	closeCurrentPage();

	homePage = new HomePage(this);
	homePage->move(134, 48);
	homePage->setAttribute(Qt::WA_DeleteOnClose);
	homePage->show();

	currentPageId = 0;

	getAccountInfo();
}

void Frame::showBillPage(QString accountName)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

	closeCurrentPage();

	billPage = new BillPage(accountName, this);
	billPage->move(134, 48);
	billPage->setAttribute(Qt::WA_DeleteOnClose);

	//connect(billPage, SIGNAL(back()), this, SLOT(showMainPage()));
	connect(billPage, SIGNAL(accountChanged(QString)), this, SLOT(showAccountPage()));
	connect(billPage, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(billPage, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));

	billPage->show();
	currentPageId = 1;

	getAccountInfo();

	DLOG_QT_WALLET_FUNCTION_END;
}

bool Frame::eventFilter(QObject* watched, QEvent* e) {
    if( (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::KeyPress)  ) {
        updateTimer();
        
    } else if ( isHidden() && e->type() == QEvent::ApplicationActivate) { //zxlrun
        qDebug() << "e->type() : " << e->type();
        showNormalAndActive();
    }
    
    return false;
}
void Frame::shadowWidgetShow() {
    qDebug() << "shadowWidgetShow";
    shadowWidget->raise();
    shadowWidget->show();
}
void Frame::shadowWidgetHide() {
    qDebug() << "shadowWidgetHide";
    shadowWidget->hide();
}

void Frame::showTransferPageWithAddress(QString address)
{
    closeCurrentPage();

    const QString& accountName = DataMgr::getInstance()->getCurrentAccount();
	transferPage = new TransferPage(accountName, this);
	transferPage->move(134, 48);
    transferPage->setAttribute(Qt::WA_DeleteOnClose);
    transferPage->setAddress(address);

    connect(transferPage, SIGNAL(accountChanged(QString)), this, SLOT(showTransferPage(QString)));
    connect(transferPage, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
    connect(transferPage, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));
	connect(this, &Frame::updateAccountBalance, transferPage, &TransferPage::updateBalance);

    transferPage->show();
    currentPageId = 2;

	getAccountInfo();
}

void Frame::setLanguage(QString language) {
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    if (language == "Simplified Chinese") {
        menuTranslator.load(DataMgr::getDataMgr()->getWorkPath() + "language/qt_zh_CN.qm");
        translatorForTextBrowser.load(DataMgr::getDataMgr()->getWorkPath() + "language/widgets.qm");
        translator.load(DataMgr::getDataMgr()->getWorkPath() + "language/gop_simplified_Chinese.qm");
        QApplication::installTranslator(&menuTranslator);
        QApplication::installTranslator(&translatorForTextBrowser);
    } else if( language == "English") {
        translator.load(DataMgr::getDataMgr()->getWorkPath() + "language/gop_English.qm");
        QApplication::removeTranslator(&menuTranslator);
        QApplication::removeTranslator(&translatorForTextBrowser);
    }

    QApplication::installTranslator(&translator);
    
    if( titleBar != NULL) {     // 已经登录
        functionBar->retranslator();
        titleBar->retranslator();
        shadowWidget->retranslator();
        retranslateTrayIconUi();

		//-1:none 0:homePage 1:billPage 2:transferPage 3:accountPage 4:quotationPage

        switch (currentPageId) {
            case 0:
				showHomePage();
                break;
			case 1:
			{
				const QString& accountName = DataMgr::getInstance()->getCurrentAccount();
				showBillPage(accountName);
			}
                break;
            case 2:
				showTransferPage();
                break;
            case 3:
                showAccountPage();
                break;
            case 4:
				showQuotationPage();
                break;
            default:
                break;
        }
    }
    
    DLOG_QT_WALLET_FUNCTION_END;
}

void Frame::walletAccountBalance()
{
	DataMgr::getInstance()->walletUpdateAccounts();
	emit updateAccountBalance();

	static std::default_random_engine generater(time(nullptr));
	static int range = AUTO_REFRESH_TIME / 5;
	static std::uniform_int_distribution<int> dis(-range, range);
	int interval = AUTO_REFRESH_TIME + dis(generater);

	timerForAutoRefresh->start(interval);
}

void Frame::walletLock() {
    if( lockPage ) {
        qDebug() << "already exist a lockpage";
        return;
    }

    lockPage = new LockPage(this);
    lockPage->setAttribute(Qt::WA_DeleteOnClose);
    connect( lockPage, SIGNAL(unlock()), this, SLOT(unlock()));
#ifdef WIN32
    connect( lockPage, SIGNAL(minimum()), this, SLOT(showMinimized()));
#else
    connect( lockPage, SIGNAL(minimum()), this, SLOT(hide()));
#endif // WIN32//zxlrun
	connect(lockPage, SIGNAL(closeGOP()), this, SLOT(appQuit()));
	connect(lockPage, SIGNAL(tray()), this, SLOT(hide()));
	connect(lockPage, SIGNAL(showShadowWidget()), this, SLOT(shadowWidgetShow()));
	connect(lockPage, SIGNAL(hideShadowWidget()), this, SLOT(shadowWidgetHide()));
    lockPage->show();
}
void Frame::jsonDataUpdated(QString id) {
}
void Frame::closeEvent(QCloseEvent *e) {
    hide();
    e->ignore();
}
void Frame::iconIsActived(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
        //点击托盘显示窗口
        case QSystemTrayIcon::Trigger: {
#ifdef WIN32
            showNormalAndActive();
#else
            static bool needReset = false;//zxlrun
            bool isHide = isHidden();
            if (isHide) {
                qDebug() << "isHide : " << isHide;
                trayIcon->setContextMenu(trayIconMenuEmpty);
                needReset = true;
            } else if (needReset) {
                trayIcon->setContextMenu(trayIconMenu);
                needReset = false;
            }
            showNormalAndActive();
#endif // WIN32//zxlrun
            break;
        }
        
        default:
            break;
    }
}

void Frame::createTrayIconActions() {
    minimizeAction = new QAction(tr("Minimize"), this);
#ifdef WIN32
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(showMinimized()));
#else
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
#endif // WIN32//zxlrun
    restoreAction = new QAction(tr("Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormalAndActive()));
    quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(appQuit()));
}

void Frame::retranslateTrayIconUi() {
    if (minimizeAction != nullptr)
        minimizeAction->setText(tr("Minimize"));
    if (restoreAction != nullptr)
        restoreAction->setText(tr("Restore"));
    if (quitAction != nullptr)
        quitAction->setText(tr("Quit"));
}

void Frame::createTrayIcon() {
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
}

void Frame::showNormalAndActive() {
    showNormal();
    activateWindow();
}

void Frame::tokenTransferTo(QString result) {
}

void Frame::appQuit()
{
	qApp->quit();
}
