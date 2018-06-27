#ifndef __MISC_H__
#define __MISC_H__

#include <QStringList>
#include <QProcess>
#include <QVector>

struct CurrencyInfo
{
	int id;
	QString contractId;
	QString name;
	QString coinType;

	bool isAsset()
	{
		return !contractId.startsWith("CON");
	}
	int assetId()
	{
		return contractId.toInt();
	}
};

enum BlockChainType {
    Test = 0,
    Formal
};

struct SmartContractInfo {
    QString address;
    QString name;
    QString level;
    QString owner;
    QString ownerAddress;
    QString ownerName;
    QString state;
    QString description;
    QString balance;
    QStringList abiList;
    QStringList eventList;
};

template <class key, class value>
key getKeyByValue(QMap<key, value> m, value v);
QString toThousandFigure( int number); // 转换为001,015 这种数字格式
QString doubleToStr(double number);
QString checkZero(double balance);

class Misc {
  public:
    static QString changePathFormat(QString path);
    static QString restorePathFormat(QString path);
    static bool isInContracts(QString filePath);
    static bool isInScripts(QString filePath);
    static qint64 write(QString cmd, QProcess* process);
    static QString read(QProcess* process);
    static QString readAll(QProcess *process);

    static bool isContractFileRegistered(QString path);
    static bool isContractFileUpgraded(QString path);
    static QString configGetContractAddress(QString path);
    static void configSetContractAddress(QString path, QString address);
};
#endif // Misc_H



