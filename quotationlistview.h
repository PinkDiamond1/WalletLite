#ifndef QUOTATIONLISTVIEW_H
#define QUOTATIONLISTVIEW_H

#include <QtWidgets/QListView>

#include "quotationlistmodel.h"

class QuotationListDelegate;
struct QuotationInfo;

class QuotationListView : public QListView
{
	Q_OBJECT

public:
	QuotationListView(QWidget *parent);
	~QuotationListView();

	void setQuotationInfos(const QVector<QuotationInfo*>* quotationInfos);

	void refresh();
	void sortList(Column col, SortOrder order);
	void refreshSortList();
	void reverseSortList(Column col);

private:
	QuotationListModel* m_listModel = nullptr;
	QuotationListDelegate* m_listDelegate = nullptr;
};

#endif // QUOTATIONLISTVIEW_H
