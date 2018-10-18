#ifndef __DATA_MGR_H__
#define __DATA_MGR_H__

#include "misc.h"

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>
#include <QRect>

#include "tokentransaction.h"
#include "blockchain/SlateEntry.hpp"
#include "blockchain/Types.hpp"
#include "blockchain/Transaction.hpp"

using namespace thinkyoung;

enum TransactionType {
	normal_transaction = 0,
	withdraw_pay_transaction = 1,
	register_account_transaction = 2,
	register_delegate_transaction = 3,
	upgrade_account_transaction = 4,
	update_account_transaction = 5,

	register_contract_transaction = 10,
	transfer_contract_transaction = 11,
	upgrade_contract_transaction = 12,
	destroy_contract_transaction = 13,
	call_contract_transaction = 14
};

struct CommonAccountInfo {
	enum ChainType{ TestChain = 1, FormalChain };
	ChainType type;
	QString name;
	QString address;
	QMap<QString, QString> balances;
	QString registerTime;
	QString ownerKey;
	bool isDelegate = false;

	QString getBalance(const QString& assetName) const
	{
		QMap<QString, QString>::const_iterator itr = balances.find(assetName);
		if (itr != balances.end())
		{
			return itr.value();
		} 
		return "0";
	}
};

struct Amount {
	qint64 amount;
	int asset_id;
};

struct LedgerEntries {
	QString from_account;//addr
	QString from_account_name; //name
	QString to_account; //addr
	QString to_account_name;//name
	QString memo;
	//QString running_balances;
	QMap<QString, QMap<QString, Amount>> running_balances;
	Amount amount;

	QString getBalance(QString account, QString asset_type) const
	{
		QMap<QString, QMap<QString, Amount>>::const_iterator itr = running_balances.find(account);
		if (itr != running_balances.end())
		{
			QMap<QString, Amount>::const_iterator _itr = (*itr).find(asset_type);
			if (_itr != (*itr).end())
				return QString::number(_itr->amount, 'f', 5);
		}

		return "0";
	}
};

//commasset
struct TrxResult {
	bool vir;
	bool confiremed;
	bool market;
	bool market_cancel;
	QString trx_id;
	int block_num;
	int block_position;
	int trx_type;
	QString time_stamp;
	QString expiration_timestamp;
	Amount fee;
	QVector<LedgerEntries> entries;
};

struct TokenAccountInfo {
	QString name;
	QString address;
	QString balance;
};

struct DelegateAccount
{
	int idx;
	int id;
	QString name;
	QString votes_num;
	QString votes_percent;

	QRect checkboxRect; //用于检测是否点中复选框
};
Q_DECLARE_METATYPE(DelegateAccount*)

struct BalanceInfo
{
	int assetId;
	uint64_t slateId;
	QString balance_id;
	QString owner;
	int64_t balance;
};

class DataMgr : public QObject {
	Q_OBJECT
public:
	static DataMgr* getDataMgr();
	static DataMgr* getInstance();
	static void    deleteDataMgr();

	//QString language;
	int lockMinutes;
    bool notAutoLock;
	bool showPrivateTips;

private:
	DataMgr();
	~DataMgr();

public:
    //TokenAsset
	QString getAccountAddr(QString account);
	QString getAddrAccont(QString addr);
	TokenTransaction* getTokenTrxConnect() { return &tokenTrxConnect; }
	TrxVector* getTokenTrx() { return &(tokenTrxConnect.trxvector); }

	// wallet
    void walletUnlock(const QString& password);
    bool walletCheckPriKeyValid(const QString& prikey);
    bool walletCheckNameExists(const QString& name);
    bool walletCheckPriKeyExists(const QString& prikey, QString* outname);
    void walletImportPrivateKey(const QString& wif_key_to_import, const QString& account_name);
	QString walletExportPrivateKey(const QString& account_name);
	QString walletGetFormerNameByPriKey(const QString& pri_key);

    bool walletSavePassphrase(const QString& new_password);

    void walletChangePassphrase(const QString& old_password, const QString& new_password);
	bool walletCheckPassphrase(const QString& password);
    bool walletCheckAddress(const QString& address);
    bool walletAccountCreate(QString& account_name);
	void walletGetAddressBalances(const QString& address);
	void walletTransferToAddressWithId(const QString& amount_to_transfer, int asset_id, const QString& from_account_name, const QString& to_address, const QString& memo_message);
	void walletTransferToAddressWithId(const QString& amount_to_transfer, int asset_id, const QString& from_account_name, const QString& to_address, const QVector<BalanceInfo>& balances);
	void walletAccountRegiste(QString& account_name, QString& pay_from_account, QString& public_data, int delegate_pay_rate, QString& account_type);
	void walletListAccounts();
    void walletUpdateAccounts();
	void walletAccountRename(QString& current_name, QString& new_name);
    void walletAccountBalance(const QString& account_name);
	bool walletAccountDelete(QString& account_name);

	void tokenTransferTo(QString call_name, QString tokenType, QString to_address, double amount, double fee, QString remark);

	// contract
	void callContract(QString& contract, QString& call_name,
		QString& function_name, QString& params,
		QString& asset_symbol, double call_limit);

	void walletDelegateAccounts(int page_n, int per_page_n = 10);
	void walletDelegateAccountsByIds(const QVector<int>& ids);

public:
	QSettings* getSettings();
	QString getAppDataPath();
	QString getWorkPath();

    void initCurrencyListFromDB();
	const QVector<CurrencyInfo>& getCurrencyList();
    QString getContractId(const QString& asset_type);

	void setCurrentAccount(QString accountName);
	QString getCurrentAccount();

	void setCurrentCurrency(const CurrencyInfo& currencyInfo);
	static CurrencyInfo getCurrentCurrency();
	static QString getCurrCurrencyName();
	CurrencyInfo getCurrencyById(int id);
	
    QMap<QString, QString>* getCurrentTrx() { return &_trx_data; }

	QMap<QString, QVector<TrxResult>>* getTrxResults();
    QMap<QString, CommonAccountInfo>* getAccountInfo();
    QString getAccountPublicKeyFromAddr(const QString& address);
    QString getAccountPublicKey(const QString& account_name);

	void setLanguage(const QString& lang);
    QString getLanguage() { return language; }
    QString getConfigPath() { return _tool_config_path + "/config.ini"; }

	bool isHiddenTotalAsset();
	void changeHiddenTotalAssetState();

	bool isVoteForDelegates();
	void changeVoteForDelegatesState();

    void broadcast(const QString& json);

	bool canRequestBalance();

	const QVector<DelegateAccount>* getVoteDelegateAccounts();
	bool saveVoteDelegateAccount(const DelegateAccount& delegateAccount);
	bool deleteVoteDelegateAccount(int accountId);
	bool isVoteDelegate(int accountId);
	void sortVoteDelegateAccount();

private:
    void parseContract(QString& json);
	void initSettings();
	void unInitSettings();
	void getSystemEnvironmentPath();
	void changeToToolConfigPath();
	void startQueryBalance();

	void setDelegateSlate(blockchain::SignedTransaction& transaction);

public slots:
	void onFetchAssetInfoFinished();
	void onFetchAssetInfoError(QNetworkReply::NetworkError errorCode);
    void onbroadcastError(QNetworkReply::NetworkError errorCode);
    void fetchAssetInfo();
    void onbroadcastFinished();

    void qureyBalanceHttpFinished();
	void qureyBalanceHttpError(QNetworkReply::NetworkError errorCode);
	void delayQueryBalance();

	void onDelegateAccountsFinished();
	void onDelegateAccountsError(QNetworkReply::NetworkError errorCode);

	void onDelegateAccountsByIdsFinished();
	void onDelegateAccountsByIdsError(QNetworkReply::NetworkError errorCode);

	void onGetAddressBalancesFinished();
	void onGetAddressBalancesError(QNetworkReply::NetworkError errorCode);

signals:
	void finished();
	void error(QNetworkReply::NetworkError);

    void onCallContract(QString);
	void onWalletTransferToContract(QString);
	void onWalletTransferToContractTesting(QString);
	void onGetContractInfo(QString);

    void onAbout(QString);
    void assetTypeGet();

	void onValidateAddress(QString);
	void onRpcSetUserName(QString);
	void onRpcSetPassword(QString);
	void onRpcStartServer(QString);
	void onNtpUpdateTime(QString);
	void onDiskUsage(QString);
	void onNetworkAddNode(QString);
	void onNetworkGetInfo(QString);
	void onNetworkGetConnectionCount(QString);
	void onNetworkGetPeerInfo(QString);
	void onNetworkListPotentialPeers(QString);
	void onNetworkGetUpnpInfo(QString);
	void onExecuteScript(QString);
	void onDelegateGetConfig(QString);
	void onDelegateSetNetworkMinConnectionCount(QString);
	void onDelegateSetBlockMaxTransactionCount(QString);
	void onDelegateSetBlockMaxSize(QString);
	void onDelegateSetTransactionMaxSize(QString);
    void onDelegateSetTransactionMinFee(QString);
	void onExecuteConsoleCommand(QString);

    void onBlockChainGetBlockTransactions(QString);
	void onBlockChainGetInfo(QString);
	void onBlockChainGenerateSnapshot(QString);
	void onBlockChainIsSynced(QString);
	void onBlockChainGetBlockCount(QString);
	void onBlockChainGetBalance(QString);
	void onBlockChainListKeyBalances(QString);
	void onBlockChainListActiveDelegates(QString);
	void onBlockChainListAccounts(QString);
	void onBlockChainListPendingTransactions(QString);
	void onBlockChainGetTransaction(QString);
	void onBlockChainListDelegates(QString);
	void onBlockChainListBlocks(QString);
	void onBlockChainGetBlockSignee(QString);
	void onBlockChainGetBlock(QString);
	void onBlockChainListAssets();

	void onWalletCreate(QString);
	void onWalletGetInfo(QString);
	void onWalletClose(QString);
	void onWalletOpen(QString);
	void onWalletUnlock(bool);
    void onWalletImportPrivateKey(bool);
	void onWalletBackupCreate(QString);
	void onWalletBackupRestore(QString);
	void onWalletSetAutomaticBackups(QString);
	void onWalletSetTransactionExpirationTime(QString);
	void onWalletAccountTransactionHistory(QString);
	void onWalletGetPendingTransactionErrors(QString);
	void onWalletChangePassphrase(bool);
    void onWalletCheckAddress(QString);
	void onWalletAccountSetApproval(QString);
	void onWalletTransferToAddress(QString);
    void onWalletTransferToAddressWithId(bool, QString, QString);
	void onWalletTransferToPublicAccount(QString);
    void onWalletTransferToPublicAccountWithId(bool, QString, QString);
	void onWalletRescanBlockchain(QString);
	void onWalletCancelScan(QString);
	void onWalletGetTransaction(QString);
	void onWalletAccountRegiste(QString);
    void onWalletListMyAddresses(QString);
	void onWalletListUnregisteredAccounts(QString);
	void onWalletListMyAccounts(QString);
	void onWalletGetAccountPublicAddress(QString);
	void onWalletAccountRename(QString);
    void onWalletAccountBalance();
	void onWalletDelegateWithdrawPay(QString);
	void onWalletDelegatePayBalanceQuery(QString);
	void onWalletGetDelegateStatue(QString);
	void onWalletSetTransactionFee(QString);
	void onWalletGetTransactionFee(QString);
	void onWalletSetTransactionScanning(QString);
	void onWalletDumpPrivateKey(QString);
    void onWalletDelegateSetBlockProduction(QString);
	void onWalletAccountUpdateRegistration(QString);
	void onWalletDelegateAccounts(bool, int, int, const QVector<DelegateAccount>&);
	void onWalletDelegateAccountsByIds(bool, const QVector<DelegateAccount>&);
	void onWalletGetAddressBalances(bool, const QVector<BalanceInfo>&);

private:
	static DataMgr* _data_mgr;
	static CurrencyInfo _current_currency;
	static QMap<QString, QString> _trx_data;

private:
	QString _current_account;
	QSettings* _config_file;

	QString _app_data_path;
    QString _app_work_path;
	QString _tool_config_path;

	QVector<CurrencyInfo> _currency_list;
	QMap<QString, bool> _currency_name_map;

	QMap<QString, QVector<TrxResult>> _trx_result_map;
    QMap<QString, CommonAccountInfo>  _account_info_map;
	TokenTransaction tokenTrxConnect;
    QString language;

    QTimer* timerGetAsset;

    QVector<QString> queryBalanceQueue;
    QNetworkReply* queryBalanceReply;
    QNetworkAccessManager netmgr;
    bool isRequestingBalance;

	bool hiddenTotalAssetVal = false;
	bool voteForDelegates = false;

	QVector<DelegateAccount>* pVoteDelegateAccounts = nullptr;

	QNetworkReply* fetchAssetInfoReply = nullptr;
	QNetworkReply* broadcastReply = nullptr;
	QNetworkReply* delegateAccountsReply = nullptr;
	QNetworkReply* delegateAccountsByIdsReply = nullptr;
	QNetworkReply* getAddressBalancesReply = nullptr;
};
#endif // Misc_H
