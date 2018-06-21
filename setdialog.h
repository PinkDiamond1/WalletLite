#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QDialog>
#include <QWebEngineView>

namespace Ui {
class SetDialog;
}

class WaitingPage;

class SetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetDialog(QWidget *parent = 0);
    ~SetDialog();

//    virtual void setVisible(bool visiable);

    void pop();

signals:
    void settingSaved();
	void refreshCombobox();

private slots:
    void on_closeBtn_clicked();

    void on_saveBtn_clicked();

    void on_nolockCheckBox_clicked();

    void on_lockTimeSpinBox_valueChanged(const QString &arg1);

    void on_generalBtn_clicked();

    void on_safeBtn_clicked();

	void on_aboutBtn_clicked();

    void on_confirmBtn_clicked();

    void on_newPwdLineEdit_textChanged(const QString &arg1);

    void on_confirmPwdLineEdit_textChanged(const QString &arg1);

    void on_oldPwdLineEdit_textChanged(const QString &arg1);

    void walletChangePassword(bool result);

private:
	Ui::SetDialog *ui;
	QWebEngineView* aboutWebview = nullptr;
};

#endif // SETDIALOG_H
