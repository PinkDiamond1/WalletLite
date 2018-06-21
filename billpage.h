#ifndef BILLPAGE_H
#define BILLPAGE_H

#include <QWidget>

namespace Ui {
class BillPage;
}

class BillPage : public QWidget
{
    Q_OBJECT

public:
	explicit BillPage(QString name, QWidget *parent = 0);
	~BillPage();

	void updateTransactions();

private:
	void clearTransactionsTableWidget();

private slots:
	void retranslator();
	void displayTokenTrxVector();

	void on_prePageBtn_clicked();
	void on_nextPageBtn_clicked();
	void on_pageLineEdit_editingFinished();
	void on_pageLineEdit_textEdited(const QString &arg1);
	void updateAccountBalance();

	void on_accountComboBox_currentIndexChanged(const QString &arg);
    void on_assetComboBox_currentIndexChanged(int index);

    void on_toBrowserBtn_clicked();

signals:
	void back();
	void showUpgradePage(QString);
	void accountChanged(QString);
	void showShadowWidget();
	void hideShadowWidget();
	void showApplyDelegatePage(QString);

private:
	Ui::BillPage *ui;
	bool inited = false;
	QString accountName;
	QString address;
	int transactionType;
	int currentPageIndex;
	QString delegateLabelString;
	QString registeredLabelString;

	void paintEvent(QPaintEvent*);
	void setAssertType();
};

#endif // BILLPAGE_H
