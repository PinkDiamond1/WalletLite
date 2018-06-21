
#include "quotationlistmodel.h"

#include <QFile>
#include <QDomDocument>
#include <qdebug.h>

#include "datamgr.h"
#include "thirddatamgr.h"

//QMap<asset_name, QMap<account_name, balance>>
#define AssetMap QMap<QString, QMap<QString, double>*>

QuotationListModel::QuotationListModel(QObject *parent)
	: QAbstractListModel(parent)
{
	m_itemDataList.clear();
}

QuotationListModel::~QuotationListModel()
{
	qDebug() << "Deconstruction QuotationListModel.";

	clearItemDataList();
}

void QuotationListModel::setQuotationInfos(const QVector<QuotationInfo*>* quotationInfos)
{
	clearItemDataList();

	double exchangeRate = ThirdDataMgr::getInstance()->getCurrentExchangeRate();

	for (int i = 0; i < quotationInfos->size(); i++)
	{
		QuotationItemData* itemData = new QuotationItemData();

		itemData->asset_name = quotationInfos->at(i)->symbol;
		itemData->price = quotationInfos->at(i)->price_usd * exchangeRate;
		itemData->percent_change = quotationInfos->at(i)->percent_change_24h;
		itemData->price_change = itemData->price / (1 + itemData->percent_change / 100) * itemData->percent_change / 100;

		m_itemDataList.push_back(itemData);
	}
}

int QuotationListModel::rowCount(const QModelIndex &parent) const
{
	return m_itemDataList.size();
}

QVariant QuotationListModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::UserRole)
		return QVariant::fromValue(m_itemDataList.at(index.row()));
	else
		return QVariant();
}

void QuotationListModel::refreshList()
{
}

void QuotationListModel::clearItemDataList()
{
	for (int i = m_itemDataList.size() - 1; i >= 0; i--)
		delete m_itemDataList.at(i);
	m_itemDataList.clear();
}

void QuotationListModel::refresh()
{
	beginResetModel();
	endResetModel();
}

bool sortByNameASC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->asset_name < v2->asset_name;
}

bool sortByNameDESC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->asset_name > v2->asset_name;
}

bool sortByPriceASC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->price < v2->price;
}

bool sortByPriceDESC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->price > v2->price;
}

bool sortByChangeASC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->percent_change < v2->percent_change;
}

bool sortByChangeDESC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->percent_change > v2->percent_change;
}

bool sortByPriceChangeASC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->price_change < v2->price_change;
}

bool sortByPriceChangeDESC(const QuotationItemData* v1, const QuotationItemData* v2)
{
	return v1->price_change > v2->price_change;
}

void QuotationListModel::sortList(Column col, SortOrder sortorder)
{
	m_sortColumn = col;
	m_sortOrder = sortorder;

	if (col == Column::NAME)
	{
		if (sortorder == SortOrder::ASC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByNameASC);
		else if (sortorder == SortOrder::DESC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByNameDESC);
	}
	else if (col == Column::PRICE)
	{
		if (sortorder == SortOrder::ASC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByPriceASC);
		else if (sortorder == SortOrder::DESC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByPriceDESC);
	}
	else if (col == Column::PERCENT_RATE)
	{
		if (sortorder == SortOrder::ASC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByChangeASC);
		else if (sortorder == SortOrder::DESC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByChangeDESC);
	}
	else if (col == Column::PRICE_CHANGE)
	{
		if (sortorder == SortOrder::ASC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByPriceChangeASC);
		else if (sortorder == SortOrder::DESC)
			qSort(m_itemDataList.begin(), m_itemDataList.end(), sortByPriceChangeDESC);
	}
}

void QuotationListModel::refreshSortList()
{
	sortList(m_sortColumn, m_sortOrder);
}

void QuotationListModel::reverseSortList(Column col)
{
	m_sortColumn = col;
	m_sortOrder = (m_sortOrder == SortOrder::ASC) ? SortOrder::DESC : SortOrder::ASC;

	sortList(m_sortColumn, m_sortOrder);
}
