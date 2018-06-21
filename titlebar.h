#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class TitleBar;
}

int applyEnable();    //  返回可以申请代理的账户个数

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = 0);
    ~TitleBar();
    void retranslator();


signals:
    void minimum();
    void closeGOP();
    void tray();
    void settingSaved();
    void showShadowWidget();
    void hideShadowWidget();

private slots:
    void on_minBtn_clicked();

    void on_closeBtn_clicked();

	void on_setupBtn_clicked();

    void saved();

    void on_noticeBtn_clicked();

private:
    Ui::TitleBar *ui;

    void paintEvent(QPaintEvent*);
};

#endif // TITLEBAR_H
