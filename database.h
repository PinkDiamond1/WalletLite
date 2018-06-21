#pragma once

#include <QObject>
#include <QSqlDatabase>

#include "datamgr.h"

class DataBase : public QObject
{
public:
	static DataBase* getInstance();
	static void destroyInstance();

	bool existUser();
	bool savePassword(const QString& password);
	bool checkPassword(const QString& password);
	bool updatePassword(const QString& password);

    bool checkNameExists(const QString& name);
    bool checkPriKeyExists(const QString& prikey, QString* outname);
	bool insertAccount(const QString& name, const QString& address, const QString& pubkey, const QString& prikey);
	bool updateAccount(const QString& name, const QMap<QString, QString>& balances);
    QString queryPrivateKey(const QString& name);
	QString queryPublicKey(const QString& name);
	CommonAccountInfo queryAccount(const QString& name);
    QVector<CommonAccountInfo> queryAllAccount();

	QVector<CurrencyInfo> queryCurrencyList();
	bool updateCurrencyList(const QVector<CurrencyInfo>& currencyList);

	bool abandonAccount(const QString& name);
	bool deleteAbandonAccount(const QString& name);
	QString queryFormerNameByAddress(const QString& address);

private:
	DataBase();
	~DataBase();

	bool createConnection();
	bool initTables();

private:
	static DataBase* _instance;
	QSqlDatabase database;
    std::string passwordSha512;
};
