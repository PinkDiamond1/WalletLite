#ifndef FRAME_H
#define FRAME_H
#include <QFrame>
#include <QWidget>
#include <QMap>
#include <QTranslator>
#include <QSystemTrayIcon>

#ifdef WIN32 
#include "windows.h"
#endif //zxlwin

namespace Ui {
   class Frame;
}

class HomePage;
class FirstLogin;
class NormalLogin;
class BillPage;
class AccountPage;
class TransferPage;
class BottomBar;
class LockPage;
class DelegatePage;
class TitleBar;
class QMenu;
class FunctionBar;
class ContactPage;
class UpgradePage;
class ApplyDelegatePage;
class SelectGopPathWidget;
class ShowBottomBarWidget;
class ShadowWidget;
class QuotationPage;

class Frame:public QFrame
{
    Q_OBJECT
public:
    Frame();
    ~Frame();

protected:
    void mousePressEvent(QMouseEvent*event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
	void showFirstLoginPage();
	void showLoginPage();
	void showTitleBar();
	void showFunctionBar();

public slots:
	void getAccountInfo();
    void autoRefresh();
	void autoRefreshQuotation();

	void reqQuotationFinished(bool firstTime);

	void shadowWidgetShow();
	void shadowWidgetHide();

    void setLanguage(QString);

    void walletAccountBalance();
    void walletLock();
	void assetTypeGet();

	void appQuit();

signals:
    void delegateListUpdated();
	void updateAccountBalance();

private slots:
    void alreadyLogin();

	void showHomePage();
	void showBillPage(QString);
	void showTransferPage();
	void showTransferPage(QString);
	void showAccountPage();
	void showQuotationPage();

    void showLockPage();
	void autoLock();
	void runUpdate();
    void unlock();
    void updateTimer();
    void settingSaved();

    void jsonDataUpdated(QString id);
    void showTransferPageWithAddress(QString);
	
    void iconIsActived(QSystemTrayIcon::ActivationReason reason);
    void showNormalAndActive();
	void tokenTransferTo(QString);

private:
	void createTrayIconActions();
	void retranslateTrayIconUi();
	void createTrayIcon();

	void startTimerForAutoRefresh(); // 自动刷新
	void startTimerForQuotation();

	void closeCurrentPage();
	bool eventFilter(QObject *watched, QEvent *e);
	void closeEvent(QCloseEvent* e);

	void startAutoUpdate();

private:
    bool mouse_press;
	QPoint move_point;

	HomePage* homePage = nullptr;
	BillPage* billPage = nullptr;
	TransferPage* transferPage = nullptr;
	AccountPage* accountPage = nullptr;
	QuotationPage* quotatioPage = nullptr;
	FunctionBar* functionBar = nullptr;
	LockPage*  lockPage = nullptr;
	TitleBar* titleBar = nullptr;

    QString lastPage;
    int currentPageId; //-1:none 0:homePage 1:billPage 2:transferPage 3:accountPage 4:quotationPage

    ShadowWidget* shadowWidget;
    QSystemTrayIcon* trayIcon;
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QMenu *trayIconMenu;
    QMenu *trayIconMenuEmpty;

#ifdef WIN32
    RECT rtConfined;   // 由于定义了 framelesswindowhint 为了不让鼠标拖动时能移到任务栏下  //zxlwin
    RECT rtDefault;  //zxlwin
#endif // WIN32

	QTimer* timer;
	QTimer* timerUpdate;
	QTimer* timerForAutoRefresh;
	QTimer* timerForQuotation = nullptr;

    QTranslator translator;         //  选择语言
    QTranslator menuTranslator;     //  右键菜单语言
    QTranslator translatorForTextBrowser;   // QTextBrowser的右键菜单翻译

	QTimer* timerUpdateAssetList;
};

#endif // FRAME_H
