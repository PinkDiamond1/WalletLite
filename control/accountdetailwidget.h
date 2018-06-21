#ifndef ACCOUNTDETAILWIDGET_H
#define ACCOUNTDETAILWIDGET_H

#include <QWidget>
#include "qrcodewidget.h"

namespace Ui {
class AccountDetailWidget;
}

class AccountDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AccountDetailWidget( QWidget *parent = 0);
    ~AccountDetailWidget();

    void setAccount(QString name);


    QString accountName;

public slots:
    void dynamicShow();
    void dynamicHide();

private slots:
    void on_closeBtn_clicked();

    void on_exportBtn_clicked();

    void on_copyBtn_clicked();

    void on_upgradeBtn_clicked();

    void moveEnd();

signals:
    void back();

    void upgrade(QString);

    void applyDelegate(QString);

private:
    Ui::AccountDetailWidget *ui;
    double salary;
    QRCodeWidget* qrCodeWidget;
    bool produceOrNot;
    QString delegateLabelString;
    QString registeredLabelString;
};

#endif // ACCOUNTDETAILWIDGET_H
