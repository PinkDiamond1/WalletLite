#ifndef FUNCTIONBAR_H
#define FUNCTIONBAR_H

#include <QWidget>

namespace Ui {
class FunctionBar;
}

class FunctionBar : public QWidget
{
    Q_OBJECT

public:
    explicit FunctionBar(QWidget *parent = 0);
    ~FunctionBar();
    void choosePage(int);
    void retranslator();

private:
	void paintEvent(QPaintEvent*);

signals:
	void showHomePage();
	void showBillPage(QString);
	void showTransferPage();
	void showAccountPage();
	void showQuotationPage();

    void showShadowWidget();
    void hideShadowWidget();

private slots:
    void on_homeBtn_clicked();
	void on_billBtn_clicked();
	void on_transferBtn_clicked();
	void on_accountBtn_clicked();
    void on_quotationBtn_clicked();

private:
    Ui::FunctionBar *ui;
    bool scan;
};

#endif // FUNCTIONBAR_H
