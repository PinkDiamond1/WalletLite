#ifndef __MISC_H__
#define __MISC_H__

#include <QStringList>
#include <QProcess>
#include <QVector>

//#define TEST_CHAIN

#define WALLET_WIDTH 960
#define WALLET_HEIGHT 580

#define CONTRACT_FILE_NAME "\\contract_config.dat"
#define UNLOCK_CMD_POSTFIX  "_not_first_login";
#define CONFIG_INI_FILENAME "config.ini"
#define IDE_PATH "AchainDevelopmentTool"
#define ACHAIN "ACHAIN"

#define MODULE_ACHAIN "ACHAINDEVELOPMENTTOOL"

#ifdef WIN32
#define ACHAIN_WALLET_VERSION "2.0.0"
#else
#define ACHAIN_WALLET_VERSION "2.0.0"
#endif
#define ACHAIN_WALLET_VERSION_STR "v" ACHAIN_WALLET_VERSION

#define AUTO_REFRESH_TIME 13000
#define PWD_LOCK_TIME  7200
#define CHAIN_RPC_PORT 60010
#define ACHAIN_ACCOUNT_MAX 50

#ifdef TEST_CHAIN
#define TOKEN_DOMAIN ""
#define ACHAIN_BROWSER_URL ""
#define QUOTATION_URL ""
#define EXCHANGE_RATE_URL ""
#else
#define TOKEN_DOMAIN ""
#define ACHAIN_BROWSER_URL ""
#define QUOTATION_URL ""
#define EXCHANGE_RATE_URL ""
#endif

#define REPORT_REQ TOKEN_DOMAIN "wallets/report/"
#define BALANCE_REQ TOKEN_DOMAIN "wallets/api/browser/act/contract/balance/query/"
#define TRANSACTION_REQ TOKEN_DOMAIN "wallets/api/browser/act/contractTransactionAll?"
#define OTHER_TOKEN_REQ TOKEN_DOMAIN "wallets/api/browser/act/contractTransactions?"
#define CON_CONFOG_URL TOKEN_DOMAIN "wallets/api/browser/act/getAllContracts?status=2"
#define BROADCAST_URL TOKEN_DOMAIN "wallets/api/wallet/act/network_broadcast_transaction"

#define COMMONASSET "ACT"

#define VERSION_CONFIG "version.ini"
#define UPDATE_TOOL_NAME "AchainUp.exe"

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



