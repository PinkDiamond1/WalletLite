#include "delegateslistmodel.h"

DelegatesListModel::DelegatesListModel(QObject *parent)
	: QAbstractListModel(parent)
{
}

DelegatesListModel::~DelegatesListModel()
{
	clearDelegateAccounts();
}

int DelegatesListModel::rowCount(const QModelIndex &parent) const
{
	return delegateAccounts.size();
}

QVariant DelegatesListModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::UserRole)
		return QVariant::fromValue(delegateAccounts.at(index.row()));
	else if (role == Qt::DisplayPropertyRole)
		return QVariant::fromValue(pageNum);
	else
		return QVariant();
}

void DelegatesListModel::setDelegateAccounts(const QVector<DelegateAccount>& accounts)
{
	clearDelegateAccounts();

	for (int i = 0; i < accounts.size(); i++)
	{
		auto pAccount = new DelegateAccount;
		*pAccount = accounts.at(i);
		delegateAccounts.push_back(pAccount);
	}

	refresh();
}

void DelegatesListModel::setPageNum(int num)
{
	pageNum = num;
}

void DelegatesListModel::clearDelegateAccounts()
{
	for (size_t i = 0; i < delegateAccounts.size(); i++)
	{
		delete delegateAccounts.at(i);
	}

	delegateAccounts.clear();
}

void DelegatesListModel::refresh()
{
	beginResetModel();
	endResetModel();
}

DelegateAccount* DelegatesListModel::getDelegateAccount(int accountId)
{
	for (int i = 0; i < delegateAccounts.size(); i++)
	{
		if (delegateAccounts.at(i)->id == accountId)
			return delegateAccounts.at(i);
	}

	return nullptr;
}
