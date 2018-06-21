#ifndef QUOTATIONLISTMODEL_H
#define QUOTATIONLISTMODEL_H

#include <QAbstractListModel>
#include <vector>

struct QuotationInfo;

class QuotationItemData : public QObject
{
	Q_OBJECT
public:
	QString asset_name;
	double price;
	double percent_change;
	double price_change;
};

enum Column
{
	NONE = 0,
	NAME,
	PRICE,
	PERCENT_RATE,
	PRICE_CHANGE
};

enum SortOrder
{
	DESC = 0,
	ASC
};

class QuotationListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	QuotationListModel(QObject *parent);
	~QuotationListModel();

	void setQuotationInfos(const QVector<QuotationInfo*>* quotationInfos);

	void refresh();
	void sortList(Column col, SortOrder sortorder);
	void refreshSortList();
	void reverseSortList(Column col);

public:
	int rowCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;

private:
	void refreshList();
	void clearItemDataList();

private:
	QVector<QuotationItemData*> m_itemDataList;

	Column m_sortColumn = Column::PRICE;
	SortOrder m_sortOrder = SortOrder::DESC;
};

#endif // QUOTATIONLISTMODEL_H
