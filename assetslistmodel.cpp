
#include "assetslistmodel.h"

#include <QFile>
#include <QDomDocument>
#include <qdebug.h>

#include "datamgr.h"
#include "thirddatamgr.h"

//QMap<asset_name, QMap<account_name, balance>>
#define AssetMap QMap<QString, QMap<QString, double>*>

AssetsListModel::AssetsListModel(QObject *parent)
	: QAbstractListModel(parent)
{
	m_itemDataList = new std::vector<AssetItemData*>();
	m_itemDataList->clear();

	m_displayItemDataList = new std::vector<AssetItemData*>();
	m_displayItemDataList->clear();
}

AssetsListModel::~AssetsListModel()
{
	qDebug() << "Deconstruction AssetsListModel.";

	clearItemDataList();

	if (m_itemDataList != nullptr)
	{
		delete m_itemDataList;
		m_itemDataList = nullptr;
	};

	if (m_displayItemDataList != nullptr)
	{
		delete m_displayItemDataList;
		m_displayItemDataList = nullptr;
	}
}

bool sortByName(const AssetItemData* v1, const AssetItemData* v2)
{
	return v1->asset_name < v2->asset_name;
}

void AssetsListModel::setAccountInfo(QMap<QString, CommonAccountInfo>* accountInfos)
{
	clearItemDataList();
	totalPrice = 0;

	//QMap<asset_name, QMap<account_name, balance>>
	AssetMap assetMap;

	QMap<QString, CommonAccountInfo>::iterator accountInfoItr = accountInfos->begin();
	for (; accountInfoItr != accountInfos->end(); accountInfoItr++)
	{
		const QString accountName = accountInfoItr->name;

		QMap<QString, QString>::iterator accountBalanceItr = accountInfoItr->balances.begin();
		for (; accountBalanceItr != accountInfoItr->balances.end(); accountBalanceItr++)
		{
			const QString& asset_name = accountBalanceItr.key();
			double balance = accountBalanceItr.value().toDouble();

			if (balance < 0.0000001)
				continue;

			AssetMap::iterator assetMapItr = assetMap.find(asset_name);
			QMap<QString, double>* balances = nullptr;

			if (assetMapItr != assetMap.end())
			{
				balances = assetMapItr.value();
			}
			else
			{
				balances = new QMap<QString, double>();
				assetMap[asset_name] = balances;
			}

			QMap<QString, double>::iterator balanceItr = balances->find(accountName);
			if (balanceItr != balances->end())
				(*balances)[accountName] += balance;
			else
				(*balances)[accountName] = balance;
		}
	}

	AssetMap::iterator itr = assetMap.begin();
	for (; itr != assetMap.end(); itr++)
	{
		QString asset_name = itr.key();
		QMap<QString, double>* balances = itr.value();

		AssetItemData* itemData = new AssetItemData();
		itemData->asset_name = asset_name;
		itemData->level = 1;
		itemData->balance = 0;
		itemData->price = 0;

		if (itemCollapseState.find(itemData->asset_name) != itemCollapseState.end())
			itemData->collapse = itemCollapseState[itemData->asset_name];
		else
			itemData->collapse = true;

		double unitPrice = ThirdDataMgr::getInstance()->getAssetPrice(asset_name);
		if (unitPrice < 0)
			itemData->price = -1;

		QMap<QString, double>::iterator balanceItr = balances->begin();
		for (; balanceItr != balances->end(); balanceItr++)
		{
			AssetItemData* childData = new AssetItemData();
			childData->level = 2;
			childData->asset_name = asset_name;
			childData->account_name = balanceItr.key();
			childData->balance = balanceItr.value();

			itemData->balance += childData->balance;

			if (unitPrice >= 0)
			{
				childData->price = childData->balance * unitPrice;
				itemData->price += childData->price;
				totalPrice += childData->price;
			}
			else
			{
				childData->price = -1;
			}

			itemData->children.push_back(childData);
		}

		std::sort(itemData->children.begin(), itemData->children.end(), sortByName);
		
		m_itemDataList->push_back(itemData);
	}

	std::sort(m_itemDataList->begin(), m_itemDataList->end(), sortByName);

	refreshList();
}

int AssetsListModel::rowCount(const QModelIndex &parent) const
{
	return m_displayItemDataList->size();
}

QVariant AssetsListModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::UserRole)
		return QVariant::fromValue(m_displayItemDataList->at(index.row()));
	else if (role == Qt::DisplayPropertyRole)
		return QVariant::fromValue(totalPrice);
	else
		return QVariant();
}

void AssetsListModel::refreshList()
{
	m_displayItemDataList->clear();
	listHeight = 0;

	for (int i = 0; i < m_itemDataList->size(); i++)
	{
		AssetItemData* itemData = m_itemDataList->at(i);

		m_displayItemDataList->push_back(itemData);
		listHeight += 50;

		if (itemData->collapse)
			continue;

		for (int j = 0; j < itemData->children.size(); j++)
		{
			AssetItemData* childItemData = itemData->children[j];
			childItemData->theLast = false;
			m_displayItemDataList->push_back(childItemData);

			listHeight += 32;
		}

		if (!m_displayItemDataList->empty())
			m_displayItemDataList->back()->theLast = true;
	}

	emit listHeightChanged(listHeight);
}

void AssetsListModel::clearItemDataList()
{
	for (int i = m_itemDataList->size() - 1; i >= 0; i--)
	{
		AssetItemData* itemData = m_itemDataList->at(i);
		for (int j = itemData->children.size() - 1; j >= 0; j--)
			delete itemData->children[j];
		itemData->children.clear();
		delete itemData;
	}
	m_itemDataList->clear();

	m_displayItemDataList->clear();
}

void AssetsListModel::collapse(const QModelIndex& index)
{
	AssetItemData* itemData = m_displayItemDataList->at(index.row());

	if (itemData->children.size() == 0)
		return;

	itemData->collapse = !itemData->collapse;
	itemCollapseState.insert(itemData->asset_name, itemData->collapse);

	refreshList();

	if (!itemData->collapse)
	{
		beginInsertRows(QModelIndex(), index.row() + 1, index.row() + itemData->children.size());
		endInsertRows();
	}
	else
	{
		beginRemoveRows(QModelIndex(), index.row() + 1, index.row() + itemData->children.size());
		endRemoveRows();
	}
}

void AssetsListModel::refresh()
{
	beginResetModel();
	endResetModel();
}

const std::vector<AssetItemData*>* AssetsListModel::getItemDataList()
{
	return m_itemDataList;
}

int AssetsListModel::getListHeight()
{
	return listHeight;
}