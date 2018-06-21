#ifndef ASSETSLISTMODEL_H
#define ASSETSLISTMODEL_H

#include <QAbstractListModel>
#include <vector>

struct CommonAccountInfo;

class AssetItemData : public QObject
{
	Q_OBJECT
public:
	QString asset_name;
	QString account_name;
	double balance;
	double price;
	int level;
	bool collapse;
	bool theLast;
	std::vector<AssetItemData*> children;
};

class AssetsListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	AssetsListModel(QObject *parent);
	~AssetsListModel();

	void setAccountInfo(QMap<QString, CommonAccountInfo>* accountInfos);
	void refresh();
	const std::vector<AssetItemData*>* getItemDataList();
	int getListHeight();

public:
	int rowCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;

public slots:
	void collapse(const QModelIndex& index);

signals:
	void listHeightChanged(int);
	
private:
	void refreshList();
	void clearItemDataList();

private:
	std::vector<AssetItemData*>* m_itemDataList;
	std::vector<AssetItemData*>* m_displayItemDataList;

	QMap<QString, bool> itemCollapseState;

	int listHeight = 0;
	double totalPrice = 0;
};

#endif // ASSETSLISTMODEL_H
