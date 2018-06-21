#include "quotationlistview.h"
#include "quotationlistmodel.h"
#include "quotationlistdelegate.h"

QuotationListView::QuotationListView(QWidget *parent)
	: QListView(parent)
{
	setStyleSheet(QString(
		"QListView{background-color:rgb(255, 255, 255);"
		"border:0px solid rgb(255, 255, 255);"
		"border-right-width:1px;}"));

	m_listModel = new QuotationListModel(this);
	m_listDelegate = new QuotationListDelegate(this);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setModel(m_listModel);
	setItemDelegate(m_listDelegate);
}

QuotationListView::~QuotationListView()
{
	m_listModel->refreshSortList();
}

void QuotationListView::setQuotationInfos(const QVector<QuotationInfo*>* quotationInfos)
{
	m_listModel->setQuotationInfos(quotationInfos);
}

void QuotationListView::refresh()
{
	m_listModel->refresh();
}

void QuotationListView::sortList(Column col, SortOrder order)
{
	m_listModel->sortList(col, order);
	m_listModel->refresh();
}

void QuotationListView::refreshSortList()
{
	m_listModel->refreshSortList();
}

void QuotationListView::reverseSortList(Column col)
{
	m_listModel->reverseSortList(col);
	m_listModel->refresh();
}
