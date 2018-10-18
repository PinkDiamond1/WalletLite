#ifndef DELEGATESPAGE_H
#define DELEGATESPAGE_H

#include <QWidget>

namespace Ui {
	class DelegatesPage;
}

class DelegatesListView;
class DelegatesListModel;
struct DelegateAccount;
class WaitingPage;

#define ALL_DELEGATES_TAB 1
#define SUPPORTED_DELEGATES_TAB 2

class DelegatesPage : public QWidget
{
    Q_OBJECT
public:
	explicit DelegatesPage(QWidget *parent = 0);
	~DelegatesPage();

	void onCheckboxClicked(int accountId);

private:
	void paintEvent(QPaintEvent*);
	void resetPageNum();
	void goToAllPage(int num);
	void goToSupportedPage(int num);
	void showWaitingPage();
	void hideWaitingPage();

public slots :
	void onDelegateAccountsCallback(bool success, int currentPage, int totalPage, const QVector<DelegateAccount>& accounts);
	void onDelegateAccountsByIdsCallback(bool success, const QVector<DelegateAccount>& accounts);

private slots:
    void on_allDelegatesBtn_clicked();
    void on_supportedDelegatesBtn_clicked();
    void on_prePageBtn_clicked();
    void on_nextPageBtn_clicked();
    void on_goToBtn_clicked();

    void on_pageLineEdit_textChanged(const QString &arg1);

private:
	Ui::DelegatesPage* ui;
	WaitingPage* waitingPage = nullptr;

	int pageNum = 1;
	int totalPageNum = 1;

	int tabFlag = ALL_DELEGATES_TAB;

	DelegatesListView* delegatesListView = nullptr;
	DelegatesListModel* delegatesListModel = nullptr;
};

#endif // DELEGATESPAGE_H
