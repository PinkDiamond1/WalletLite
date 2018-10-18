#ifndef DELEGATESLISTMODEL_H
#define DELEGATESLISTMODEL_H

#include <QAbstractListModel>

#include "datamgr.h"

class DelegatesListModel : public QAbstractListModel
{
public:
	DelegatesListModel(QObject *parent);
	~DelegatesListModel();

public:
	int rowCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	void setDelegateAccounts(const QVector<DelegateAccount>& accounts);
	void setPageNum(int num);
	void refresh();

	DelegateAccount* getDelegateAccount(int accountId);

private:
	void clearDelegateAccounts();

private:
	QVector<DelegateAccount*> delegateAccounts;
	int pageNum = 0;
};

#endif // DELEGATESLISTMODEL_H