#include "database.h"
#include "qvariant.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QDir>
#include <qsqlrecord.h>

#include "datamgr.h"
#include "fc/crypto/aes.hpp"

#define CONNECTION_NAME "achain_wallet_lite_sql_connection"
#define DATABASE_NAME "achain_wallet_lite_database.db"

DataBase* DataBase::_instance = nullptr;

DataBase* DataBase::getInstance()
{
	if (_instance == nullptr)
		_instance = new DataBase();

	return _instance;
}

void DataBase::destroyInstance()
{
	if (_instance != nullptr)
		delete _instance;
	
	_instance = nullptr;
}

DataBase::DataBase()
{
	createConnection();
	initTables();
	existUser();
}

DataBase::~DataBase()
{
	database.close();
}

bool DataBase::createConnection()
{
    QString dbPath = DataMgr::getInstance()->getAppDataPath();
    QDir dir(dbPath);
    if(!dir.exists())
    {
        dir.mkdir(dbPath);
    }

	database = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    database.setDatabaseName(dbPath + "/" + DATABASE_NAME);

	if (!database.open())
    {
        qDebug() << "open database error == " << database.lastError();
		return false;
	}

	return true;
}

bool DataBase::initTables()
{
	//init user table
	QSqlQuery sql_query(database);
	bool success = sql_query.exec("select * from sqlite_master where name = 'user';");
	if (success)
	{
        if (!sql_query.next())
		{
			success = sql_query.exec("create table user (id int primary key, password varchar(256))");
			if (!success)
			{
				qDebug() << "Error: Fail to create user table." << sql_query.lastError();
			}
		}
	}
	else
	{
		qDebug() << "Error: Fail to check user table." << sql_query.lastError();
	}

	//init account table
	success = sql_query.exec("select * from sqlite_master where name = 'account';");
	if (success)
	{
        if (!sql_query.next())
		{
			success = sql_query.exec("create table account (id int primary key, name varchar(64), address varchar(128), pubkey varchar(128), prikey varchar(128), balance text);");
			if (!success)
			{
				qDebug() << "Error: Fail to create account table." << sql_query.lastError();
			}
		}
	}
	else
	{
		qDebug() << "Error: Fail to check account table." << sql_query.lastError();
    }

	//init abandon account table
	success = sql_query.exec("select * from sqlite_master where name = 'abandon_account';");
	if (success)
	{
		if (!sql_query.next())
		{
			success = sql_query.exec("create table abandon_account (id int primary key, name varchar(64), address varchar(128), pubkey varchar(128), prikey varchar(128), balance text);");
			if (!success)
			{
				qDebug() << "Error: Fail to create abandon_account table." << sql_query.lastError();
			}
		}
	}
	else
	{
		qDebug() << "Error: Fail to check account table." << sql_query.lastError();
	}

    //init asset type table
    success = sql_query.exec("select * from sqlite_master where name = 'currency';");
    if (success)
    {
        if (!sql_query.next())
        {
			success = sql_query.exec("create table currency(id int primary key, contractId varchar(64), name varchar(64), coinType varchar(64));");
			if (!success)
            {
                qDebug() << "Error: Fail to create currency table." << sql_query.lastError();
            }
        }
    }
    else
    {
        qDebug() << "Error: Fail to check currency table." << sql_query.lastError();
    }

	return true;
}

bool DataBase::existUser()
{
	QSqlQuery sql_query(database);
	bool success = sql_query.exec("select count(*) from user;");
	if (success)
	{
		if (sql_query.next() && sql_query.value(0).toInt() == 1)
			return true;
	}
	else
	{
		qDebug() << "Error: Fail to check user table." << sql_query.lastError();
	}

	return false;
}

bool DataBase::savePassword(const QString& password)
{
	QByteArray passwordBytes = password.toUtf8();
	QCryptographicHash *hash = new QCryptographicHash(QCryptographicHash::Sha512);
	hash->addData(passwordBytes);
	passwordBytes = hash->result();
	hash->reset();
	hash->addData(passwordBytes);
	QString passwordSha512 = QString(hash->result());

	delete hash;

	QSqlQuery sql_query(database);
	sql_query.prepare("insert into user values(0, ?);");
	sql_query.bindValue(0, passwordSha512);

	bool success = sql_query.exec();
	if (!success)
	{
		qDebug() << "Error: Fail to savePassword." << sql_query.lastError();
	}

	return success;
}

bool DataBase::checkPassword(const QString& password)
{
	QByteArray passwordBytes = password.toUtf8();
	QCryptographicHash *hash = new QCryptographicHash(QCryptographicHash::Sha512);
	hash->addData(passwordBytes);
	passwordBytes = hash->result();
	hash->reset();
	hash->addData(passwordBytes);
	QString pwSha512 = QString(hash->result());

	delete hash;

	QSqlQuery sql_query(database);
	bool success = sql_query.exec("select password from user where id = 0;");
	if (success)
	{
        if (sql_query.next() && sql_query.value("password").toString() == pwSha512)
		{
            passwordSha512 = QString(passwordBytes).toStdString();
			return true;
		}
	}
	else
	{
		qDebug() << "Error: Fail to check user table." << sql_query.lastError();
	}

	return false;
}

bool DataBase::updatePassword(const QString& password)
{
    QByteArray passwordBytes = password.toUtf8();
    QCryptographicHash *hash = new QCryptographicHash(QCryptographicHash::Sha512);
    hash->addData(passwordBytes);
    passwordBytes = hash->result();
    hash->reset();
    hash->addData(passwordBytes);
	QString pwSha512 = QString(hash->result());

	delete hash;

	QSqlQuery sql_query(database);
	sql_query.prepare("update user set password = ? where id = 0;");
	sql_query.bindValue(0, pwSha512);
	bool success = sql_query.exec();
    if (success)
        passwordSha512 = QString(passwordBytes).toStdString();
    else
        qDebug() << "Error: Fail to updatePassword." << sql_query.lastError();

	return success;
}

bool DataBase::checkNameExists(const QString& name)
{
	QSqlQuery sql_query(database);
	sql_query.prepare("select name from account where name = ?;");
	sql_query.bindValue(0, name);
	bool success = sql_query.exec();
	if (success)
    {
        if (sql_query.next() && sql_query.value("name").toString() == name)
            return true;
	}
	else
	{
        qDebug() << "Error: Fail to checkNameExists." << sql_query.lastError();
        return true;
	}

    return false;
}

bool DataBase::checkPriKeyExists(const QString& prikey, QString* outname)
{
    QSqlQuery sql_query(database);
    sql_query.prepare("select name from account where prikey = ?;");

    std::vector<char> vprikey;
    const std::string& sprikey = prikey.toStdString();
    vprikey.resize(sprikey.size() + 1);
	vprikey[sprikey.size()] = 0;
    vprikey.assign(sprikey.begin(), sprikey.end());
    std::vector<char> encryptedprikey = fc::aes_encrypt(fc::sha512().hash(passwordSha512), vprikey);
    sql_query.bindValue(0, QByteArray(&encryptedprikey[0], encryptedprikey.size()).toBase64());

    bool success = sql_query.exec();
    if (success)
    {
        if (sql_query.next())
        {
            *outname = sql_query.value("name").toString();
            return true;
        }
    }
    else
    {
        qDebug() << "Error: Fail to checkPriKeyExists." << sql_query.lastError();
        return true;
    }

    return false;
}

bool DataBase::insertAccount(const QString& name, const QString& address, const QString& pubkey, const QString& prikey)
{
	QSqlQuery sql_query(database);
	sql_query.prepare("insert into account(name, address, pubkey, prikey, balance) values(?, ?, ?, ?, '{}');");
	sql_query.bindValue(0, name);
	sql_query.bindValue(1, address);
    sql_query.bindValue(2, pubkey);

    std::vector<char> vprikey;
    const std::string& sprikey = prikey.toStdString();
    vprikey.resize(sprikey.size() + 1);
    vprikey[sprikey.size()] = 0;
    vprikey.assign(sprikey.begin(), sprikey.end());
    std::vector<char> encryptedprikey = fc::aes_encrypt(fc::sha512().hash(passwordSha512), vprikey);
    sql_query.bindValue(3, QByteArray(&encryptedprikey[0], encryptedprikey.size()).toBase64());

	bool success = sql_query.exec();
	if (!success)
	{
		qDebug() << "Error: Fail to insertAccount." << sql_query.lastError();
	}

    return success;
}

bool DataBase::updateAccount(const QString& name, const QMap<QString, QString>& balances)
{
	QMap<QString, QVariant> _balances = QMap<QString, QVariant>();

	QMap<QString, QString>::const_iterator itr = balances.begin();
	for (; itr != balances.end(); itr++)
	{
		_balances.insert(itr.key(), itr.value());
	}

	QJsonDocument doc = QJsonDocument::fromVariant(QVariant(_balances));
	QByteArray jba = doc.toJson();
    QString balanceStr = QString(jba);

	QSqlQuery sql_query(database);
	sql_query.prepare("update account set balance = ? where name = ?;");
	sql_query.bindValue(0, balanceStr);
	sql_query.bindValue(1, name);

	bool success = sql_query.exec();
	if (!success)
    {
		qDebug() << "Error: Fail to updateAccount." << sql_query.lastError();
	}

	return success;
}

QString DataBase::queryPrivateKey(const QString& name)
{
    QSqlQuery sql_query(database);
    sql_query.prepare("select prikey from account where name = ?;");
    sql_query.bindValue(0, name);

    bool success = sql_query.exec();
    if (success)
    {
        if (sql_query.next())
        {
            std::string encryptedprikey = fc::base64_decode(sql_query.value("prikey").toString().toStdString());
            std::vector<char> vencryptedprikey;
			vencryptedprikey.resize(encryptedprikey.size() + 1);
			vencryptedprikey[encryptedprikey.size()] = 0;
            vencryptedprikey.assign(encryptedprikey.begin(), encryptedprikey.end());
            std::vector<char> vprikey = fc::aes_decrypt(fc::sha512().hash(passwordSha512), vencryptedprikey);
            vprikey.resize(vprikey.size()+1);
            vprikey[vprikey.size() - 1] = 0;

            return QString::fromStdString(&vprikey[0]);
        }
    }

    return "";
}

CommonAccountInfo DataBase::queryAccount(const QString& name)
{
	CommonAccountInfo accountInfo;
	accountInfo.name = "";

	QSqlQuery sql_query(database);
	sql_query.prepare("select name, address, pubkey, balance from account where name = ?;");
	sql_query.bindValue(0, name);
	bool success = sql_query.exec();
	if (success && sql_query.next())
	{
		accountInfo.name = sql_query.value("name").toString();
		accountInfo.address = sql_query.value("address").toString();
		accountInfo.ownerKey = sql_query.value("pubkey").toString();
		QByteArray njba = sql_query.value("balance").toString().toUtf8();
		QJsonObject nobj = QJsonObject(QJsonDocument::fromJson(njba).object());

		accountInfo.balances.clear();
		QJsonObject::Iterator itr;
		for (itr = nobj.begin(); itr != nobj.end(); itr++)
		{
			accountInfo.balances.insert(itr.key(), itr.value().toString());
		}
	}
	else
	{
		qDebug() << "Error: Fail to queryAccount." << sql_query.lastError();
	}

	return accountInfo;
}

QVector<CommonAccountInfo> DataBase::queryAllAccount()
{
	QVector<CommonAccountInfo> accountVec;

	QSqlQuery sql_query(database);
    bool success = sql_query.exec("select name, address, pubkey, balance from account;");
	if (success)
    {
		CommonAccountInfo accountInfo;

		while (sql_query.next())
        {
			accountInfo.name = sql_query.value("name").toString();
			accountInfo.address = sql_query.value("address").toString();
            accountInfo.ownerKey = sql_query.value("pubkey").toString();
            QByteArray njba = sql_query.value("balance").toString().toUtf8();
			QJsonObject nobj = QJsonObject(QJsonDocument::fromJson(njba).object());

            accountInfo.balances.clear();
			QJsonObject::Iterator itr;
			for (itr = nobj.begin(); itr != nobj.end(); itr++)
            {
				accountInfo.balances.insert(itr.key(), itr.value().toString());
			}

            accountVec.push_back(accountInfo);
		}
	}
	else
	{
        qDebug() << "Error: Fail to queryAllAccount." << sql_query.lastError();
	}

	return accountVec;
}

QVector<CurrencyInfo> DataBase::queryCurrencyList()
{
    QVector<CurrencyInfo> currencyList;

    QSqlQuery sql_query(database);
    bool success = sql_query.exec("select id, contractId, name, coinType from currency;");
    if (success)
    {
		CurrencyInfo currencyInfo;
        while (sql_query.next())
        {
			currencyInfo.id = sql_query.value("id").toInt();
			currencyInfo.coinType = sql_query.value("coinType").toString();
			currencyInfo.contractId = sql_query.value("contractId").toString();
			currencyInfo.name = sql_query.value("name").toString();

			currencyList.push_back(currencyInfo);
        }
    }
    else
    {
        qDebug() << "Error: Fail to queryCurrencyList." << sql_query.lastError();
    }

	return currencyList;
}

bool DataBase::updateCurrencyList(const QVector<CurrencyInfo>& currencyList)
{
    QSqlQuery sql_delete(database);
    sql_delete.prepare("delete from currency;");
    bool success = sql_delete.exec();
    if (!success)
    {
        qDebug() << "Error: Fail to clear currency." << sql_delete.lastError();
        return false;
    }

	database.transaction();

	for (int i = 0; i < currencyList.size(); i++) {
        QSqlQuery sql_query(database);
        sql_query.prepare("insert into currency (id, contractId, name, coinType) values(?, ?, ?, ?);");
		sql_query.bindValue(0, currencyList.at(i).id);
		sql_query.bindValue(1, currencyList.at(i).contractId);
		sql_query.bindValue(2, currencyList.at(i).name);
		sql_query.bindValue(3, currencyList.at(i).coinType);
        success = sql_query.exec();
        if (!success)
        {
            qDebug() << "Error: Fail to updateCurrencyList." << sql_query.lastError();
        }
    }

	success = database.commit();

    return success;
}

bool DataBase::abandonAccount(const QString& name)
{
	const CommonAccountInfo& accountInfo = queryAccount(name);
	if (accountInfo.name == "")
	{
		qDebug() << "Error: Fail to select account.";
		return false;
	}

	QSqlQuery sql_delete(database);
	sql_delete.prepare("delete from account where name = ?;");
	sql_delete.bindValue(0, name);
	bool success = sql_delete.exec();
	if (!success)
	{
		qDebug() << "Error: Fail to delete account." << sql_delete.lastError();
		return false;
	}

	QSqlQuery sql_insert(database);
	sql_insert.prepare("insert into abandon_account(name, address, pubkey, prikey, balance) values(?, ?, ?, '', '{}');");
	sql_insert.bindValue(0, accountInfo.name);
	sql_insert.bindValue(1, accountInfo.address);
	sql_insert.bindValue(2, accountInfo.ownerKey);

	success = sql_insert.exec();
	if (!success)
	{
		qDebug() << "Error: Fail to abandon_account." << sql_insert.lastError();
	}

	return success;
}

QString DataBase::queryFormerNameByAddress(const QString& address)
{
	QSqlQuery sql_query(database);
	sql_query.prepare("select name from abandon_account where address = ?;");
	sql_query.bindValue(0, address);
	bool success = sql_query.exec();
	if (success && sql_query.next())
		return sql_query.value("name").toString();
	else
		qDebug() << "Error: Fail to queryFormerNameByAddress." << sql_query.lastError();

	return "";
}

bool DataBase::deleteAbandonAccount(const QString& name)
{
	QSqlQuery sql_delete(database);
	sql_delete.prepare("delete from abandon_account where name = ?;");
	sql_delete.bindValue(0, name);
	bool success = sql_delete.exec();
	if (!success)
	{
		qDebug() << "Error: Fail to delete abandon account." << sql_delete.lastError();
	}

	return success;
}
