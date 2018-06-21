#ifndef QUOTATIONPAGE_H
#define QUOTATIONPAGE_H

#include <QWidget>
#include <QLabel>

#include "quotationlistmodel.h"

namespace Ui {
class QuotationPage;
}

class QuotationListView;

class QuotationPage : public QWidget
{
    Q_OBJECT

public:
    explicit QuotationPage(QWidget *parent = 0);
	~QuotationPage();
	void retranslator(QString language);

private slots:
    void on_sortPriceBtn_clicked();
    void on_sortChangeRateBtn_clicked();
	void on_sortChangeBtn_clicked();
	void refreshQuotation();

    void on_nameBtn_clicked();

private:
	void paintEvent(QPaintEvent*);
	void sortList(Column col);

private:
    Ui::QuotationPage *ui;
	QuotationListView* quotationListView = nullptr;
	QLabel* dataFromLabel = nullptr;

	Column sortColumn;
	SortOrder sortOrder[5];
};

#endif // QUOTATIONPAGE_H
