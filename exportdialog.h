#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog( QString name, QWidget *parent = 0);
    ~ExportDialog();

    void pop();
	void hiddenCancelBtn();

private slots:
    void on_pathBtn_clicked();

    void on_cancelBtn_clicked();

    void on_exportBtn_clicked();

    void dumpAccountPrivateKey();

private:
    Ui::ExportDialog *ui;
    QString accountName;
    QString privateKey;
    QString exportPath;
};

#endif // EXPORTDIALOG_H
