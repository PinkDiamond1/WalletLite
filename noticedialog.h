#ifndef NOTICEDIALOG_H
#define NOTICEDIALOG_H

#include <QDialog>
#include <QWebEngineView>

namespace Ui {
class NoticeDialog;
}

class NoticeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NoticeDialog(QWidget *parent = 0);
    ~NoticeDialog();

	void pop();

private slots:
    void on_closeBtn_clicked();

private:
    Ui::NoticeDialog *ui;
	QWebEngineView* webview = nullptr;
};

#endif // NOTICEDIALOG_H
