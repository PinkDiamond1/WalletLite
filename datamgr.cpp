#include "datamgr.h"
#include <assert.h>
#include <algorithm>
#include <unordered_set>
#include "misc.h"
#include "macro.h"

#include <qmessagebox.h>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <QVariantList>
#include <QDir>

#include "database.h"
#include "achainlightwalletapi.h"
#include "fc/crypto/base58.hpp"
#include "blockchain/AccountEntry.hpp"

using thinkyoung::blockchain::PublicKeyType;

DataMgr* DataMgr::_data_mgr = nullptr;
CurrencyInfo DataMgr::_current_currency = { -1, "-1", "-1", "-1" };
QMap<QString, QString> DataMgr::_trx_data;

DataMgr* DataMgr::getDataMgr() {
	if (!_data_mgr) {
        _data_mgr = new DataMgr;
	}
	return _data_mgr;
}

DataMgr* DataMgr::getInstance()
{
	return DataMgr::getDataMgr();
}

void DataMgr::deleteDataMgr() {
	if (_data_mgr) {
		delete _data_mgr;
	}
	_data_mgr = nullptr;
}

DataMgr::DataMgr() :
    _app_work_path(""),
    timerGetAsset(nullptr),
    isRequestingBalance(false)
{
    initSettings();
    fetchAssetInfo();
}

DataMgr::~DataMgr() {
    if (nullptr != timerGetAsset) {
        timerGetAsset->stop();
        delete timerGetAsset;
        timerGetAsset = nullptr;
    }

	unInitSettings();
}

void sign_broadcast(const QString& from_account_name, thinkyoung::blockchain::SignedTransaction& trx)
{
    const QString prikey = DataBase::getInstance()->queryPrivateKey(from_account_name);
    const auto okey =  thinkyoung::blockchain::wif_to_key(prikey.toStdString());
	const auto chain_id = thinkyoung::blockchain::DigestType(std::string(CHAIN_ID));
    if (okey.valid()){
        trx.sign(*okey, chain_id);
    }

    QString broadcast_str = thinkyoung::blockchain::signedtransaction_to_json(trx);
    //qDebug() << "\nbroadcast_str:" << broadcast_str.toStdString().data() << "\n";
    DataMgr::getInstance()->broadcast(broadcast_str);
}

bool currencyListCmpare(const CurrencyInfo& p1, const CurrencyInfo&  p2)
{
	return p1.name < p2.name;
}

void DataMgr::parseContract(QString& json_str)
{
	QJsonParseError json_error;
	QJsonDocument json_doucment = QJsonDocument::fromJson(json_str.toUtf8(), &json_error);
	QJsonObject json_object = json_doucment.object();
	QJsonValue value = json_object.value(QString("result"));
	QJsonArray json_array;
	if (value.isArray()) {
		json_array = value.toArray();
    }

	_currency_list.clear();
	_currency_name_map.clear();

	CurrencyInfo currencyInfo;
	QJsonObject con_object;
	int count = json_array.count();
	if (count > 0)
	{
		for (int i = 0; i < count; i++) {
			value = json_array.at(i);
			if (value.isObject()) {
				con_object = value.toObject();

				currencyInfo.id = con_object.value("id").toInt();
				currencyInfo.name = con_object.value("name").toString();

				if (currencyInfo.name == "ECT")
					continue;

				currencyInfo.contractId = con_object.value("contractId").toString();
				currencyInfo.coinType = con_object.value("coinType").toString();

				_currency_name_map.insert(currencyInfo.coinType, true);
				_currency_list.push_back(currencyInfo);
			}
        }
	}

	qSort(_currency_list.begin(), _currency_list.end(), currencyListCmpare);

	currencyInfo = { 0, "0", COMMONASSET, COMMONASSET };
	_currency_list.push_front(currencyInfo);
	_currency_name_map.insert(COMMONASSET, true);
}

void DataMgr::broadcast(const QString& json)
{
    //next version contract from net
    QNetworkRequest request;

    QString url(BROADCAST_URL);
    request.setUrl(QUrl(url));
    if (url.startsWith("https")) {
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_0);
        request.setSslConfiguration(config);
    }

    const QByteArray bytePost = ("message=" + json).toUtf8();
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, bytePost.length());
	broadcastReply = netmgr.post(request, bytePost);
	connect(broadcastReply, SIGNAL(finished()), this, SLOT(onbroadcastFinished()));
	connect(broadcastReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onbroadcastError(QNetworkReply::NetworkError)));
}

void DataMgr::onbroadcastFinished()
{
	if (broadcastReply == nullptr)
		return;

	const QByteArray& reply_content = broadcastReply->readAll();
	QString json_str(reply_content);

	if (QNetworkReply::NoError != broadcastReply->error())
	{
		qDebug() << "DataMgr::onbroadcastFinished_ error == " << json_str;
		emit onWalletTransferToAddressWithId(false, "", "");
		return;
	}

	QJsonParseError json_error;
	QJsonDocument json_doucment = QJsonDocument::fromJson(reply_content, &json_error);

	if (json_doucment.isEmpty() == false)
	{
		QJsonObject json_object = json_doucment.object();
		QJsonObject errorObj = json_object.value(QString("error")).toObject();
		if (errorObj.isEmpty() == false)
		{
			qDebug() << "DataMgr::onbroadcastFinished error == " << json_str;
			QString msg = errorObj.value("message").toString();
			emit onWalletTransferToAddressWithId(false, "", msg);
			return;
		}
	}

	emit onWalletTransferToAddressWithId(true, "", "");
}

void DataMgr::onbroadcastError(QNetworkReply::NetworkError errorCode)
{
	qDebug() << errorCode;
	qDebug() << broadcastReply->errorString();

	if (broadcastReply != nullptr)
	{
		broadcastReply->deleteLater();
		broadcastReply = nullptr;
	}

	emit onWalletTransferToAddressWithId(false, "", "");
}

void DataMgr::fetchAssetInfo()
{
	//next version contract from net
	QNetworkRequest request;

	QString url(CON_CONFOG_URL);
	request.setUrl(QUrl(url));
    if (url.startsWith("https")) {
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}
	fetchAssetInfoReply = netmgr.get(request);
	connect(fetchAssetInfoReply, SIGNAL(finished()), this, SLOT(onFetchAssetInfoFinished()));
	connect(fetchAssetInfoReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onFetchAssetInfoError(QNetworkReply::NetworkError)));
}

void DataMgr::onFetchAssetInfoFinished()
{
	if (fetchAssetInfoReply == nullptr)
	{
		if (timerGetAsset == nullptr)
		{
			timerGetAsset = new QTimer(this);
			connect(timerGetAsset, SIGNAL(timeout()), this, SLOT(fetchAssetInfo()));
			timerGetAsset->start(10 * 60 * 1000);
		}
	}
	else
	{
		QByteArray reply_content = fetchAssetInfoReply->readAll();
		QString json_str(reply_content);

		if (fetchAssetInfoReply != nullptr)
		{
			fetchAssetInfoReply->deleteLater();
			fetchAssetInfoReply = nullptr;
		}

		parseContract(json_str);
		//saved result
		DataBase::getInstance()->updateCurrencyList(_currency_list);

		if (timerGetAsset != nullptr)
		{
			timerGetAsset->stop();
			delete timerGetAsset;
			timerGetAsset = nullptr;
		}

		emit assetTypeGet();
	}
}

void DataMgr::onFetchAssetInfoError(QNetworkReply::NetworkError errorCode)
{
    qDebug() << errorCode;
	qDebug() << fetchAssetInfoReply->errorString();

	if (fetchAssetInfoReply != nullptr)
	{
		fetchAssetInfoReply->deleteLater();
		fetchAssetInfoReply = nullptr;
	}

    if (nullptr == timerGetAsset)
	{
        timerGetAsset = new QTimer(this);
        connect(timerGetAsset, SIGNAL(timeout()), this, SLOT(fetchAssetInfo()));
        timerGetAsset->start(10 * 60 * 1000);
    }
}

void DataMgr::walletUnlock(const QString& password) {
	bool ret = DataBase::getInstance()->checkPassword(password);

	emit onWalletUnlock(ret);
}

bool DataMgr::walletCheckPriKeyValid(const QString& prikey)
{
    auto okey =  thinkyoung::blockchain::wif_to_key(prikey.toStdString());
    return okey.valid();
}

bool DataMgr::walletCheckNameExists(const QString& name)
{
    return DataBase::getInstance()->checkNameExists(name);
}

bool DataMgr::walletCheckPriKeyExists(const QString& prikey, QString* outname)
{
    return DataBase::getInstance()->checkPriKeyExists(prikey, outname);
}

void DataMgr::walletImportPrivateKey(const QString& wif_key_to_import, const QString& account_name)
{
    auto okey =  thinkyoung::blockchain::wif_to_key(wif_key_to_import.toStdString());
    if (okey.valid()) {
        fc::ecc::private_key private_key = (*okey);

        QString prikey = QString::fromStdString(thinkyoung::blockchain::key_to_wif(private_key));
        QString pubkey = QString::fromStdString(std::string(PublicKeyType(private_key.get_public_key())));

        thinkyoung::blockchain::Address addr(private_key.get_public_key());
        QString address = QString::fromStdString(addr.AddressToString());

        bool ret = DataBase::getInstance()->insertAccount(account_name, address, pubkey, prikey);
		if (ret)
		{
			walletListAccounts();
		}

		DataBase::getInstance()->deleteAbandonAccount(account_name);
    } else {
        qDebug() << "private invalid!";
    }

    emit onWalletImportPrivateKey(okey.valid());
}

QString DataMgr::walletExportPrivateKey(const QString& account_name)
{
    return DataBase::getInstance()->queryPrivateKey(account_name);;
}

bool DataMgr::walletSavePassphrase(const QString& new_password)
{
	return DataBase::getInstance()->savePassword(new_password);
}

void DataMgr::walletChangePassphrase(const QString& old_password, const QString& new_password) {
	bool ret = walletCheckPassphrase(old_password);
	if (ret)
	{
		DataBase::getInstance()->updatePassword(new_password);
	}

	emit onWalletChangePassphrase(ret);
}

bool DataMgr::walletCheckPassphrase(const QString& password) {
	return DataBase::getInstance()->checkPassword(password);
}

bool DataMgr::walletCheckAddress(const QString& address) {
	std::string strToAccount;
	std::string strSubAccount;
	thinkyoung::blockchain::accountsplit(address.toStdString(), strToAccount, strSubAccount);
	return thinkyoung::blockchain::Address().is_valid(strToAccount);
}

bool DataMgr::walletAccountCreate(QString& account_name)
{
    fc::ecc::private_key private_key = fc::ecc::private_key::generate();
    QString prikey = QString::fromStdString(thinkyoung::blockchain::key_to_wif(private_key));
    QString pubkey = QString::fromStdString(std::string(PublicKeyType(private_key.get_public_key())));
    thinkyoung::blockchain::Address addr(private_key.get_public_key());
    QString address = QString::fromStdString(addr.AddressToString());

    bool ret = DataBase::getInstance()->insertAccount(account_name, address, pubkey, prikey);
	if (ret)
	{
		walletListAccounts();
	}

	return ret;
}

void DataMgr::walletTransferToAddressWithId(
    const QString& amount_to_transfer, int asset_id,
    const QString& from_account_name, const QString& to_address,
    const QString& memo_message)
{
    const std::string str_amount_to_transfer = amount_to_transfer.toStdString();
    const std::string str_from_address = getInstance()->getAccountAddr(from_account_name).toStdString();
    const std::string str_to_address = to_address.toStdString();
    const std::string str_memo_message = memo_message.toStdString();

	auto trx = thinkyoung::blockchain::wallet_transfer_to_address(
		str_amount_to_transfer,
		asset_id, str_from_address,
		str_to_address, str_memo_message);

	if (voteForDelegates && asset_id == 0)
	{
		setDelegateSlate(trx);
	}

	sign_broadcast(from_account_name, trx);
}

void DataMgr::walletTransferToAddressWithId(
	const QString& amount_to_transfer,
	int asset_id,
	const QString& from_account_name,
	const QString& to_address,
	const QVector<BalanceInfo>& balances)
{
	const std::string str_amount_to_transfer = amount_to_transfer.toStdString();
	const std::string str_from_address = getInstance()->getAccountAddr(from_account_name).toStdString();
	const std::string str_to_address = to_address.toStdString();

	std::unordered_set<thinkyoung::blockchain::Address> required_signatures;

	int signaturesCount = 0;

	auto trx = thinkyoung::blockchain::wallet_transfer_to_address(
		str_amount_to_transfer,
		asset_id, str_from_address,
		str_to_address,
		balances,
		signaturesCount);

	if (voteForDelegates && asset_id == 0)
	{
		setDelegateSlate(trx);
	}

	const QString prikey = DataBase::getInstance()->queryPrivateKey(from_account_name);
	const auto okey = thinkyoung::blockchain::wif_to_key(prikey.toStdString());
	const auto chain_id = thinkyoung::blockchain::DigestType(std::string(CHAIN_ID));
	if (okey.valid())
	{
		for (int i = 0; i < signaturesCount; i++)
			trx.sign(*okey, chain_id);
	}

	QString broadcast_str = thinkyoung::blockchain::signedtransaction_to_json(trx);
	DataMgr::getInstance()->broadcast(broadcast_str);
}

void DataMgr::setDelegateSlate(blockchain::SignedTransaction& transaction)
{
	std::vector<blockchain::AccountIdType> for_candidates;

	auto delegateAccounts = getVoteDelegateAccounts();

	//const std::unordered_map<int32_t, blockchain::AccountEntry> account_items;

	for (int i = 0; i < delegateAccounts->size(); i++)
	{
		for_candidates.push_back(delegateAccounts->at(i).id);
	}

	//for (const auto& item : delegateAccounts) {
	//	const auto account_entry = item.second;
	//	for_candidates.push_back(account_entry.id);
	//}

	std::random_shuffle(for_candidates.begin(), for_candidates.end());

	blockchain::SlateEntry entry;

	for (const blockchain::AccountIdType id : for_candidates)
		entry.slate.insert(id);

	blockchain::SlateIdType slate_id = entry.id();
	if (slate_id != 0)
	{
		transaction.define_slate(entry.slate);
		transaction.set_slates(slate_id);
	}
}

const QVector<DelegateAccount>* DataMgr::getVoteDelegateAccounts()
{
	if (pVoteDelegateAccounts == nullptr)
	{
		pVoteDelegateAccounts = DataBase::getInstance()->queryDelegateAccountList();
		sortVoteDelegateAccount();
	}

	return pVoteDelegateAccounts;
}

bool DataMgr::saveVoteDelegateAccount(const DelegateAccount& delegateAccount)
{
	for (int i = 0; i < pVoteDelegateAccounts->size(); i++)
	{
		if (pVoteDelegateAccounts->at(i).id == delegateAccount.id)
		{
			qDebug() << "DataMgr::saveDelegateAccount error: account id already exists == " << delegateAccount.id;
			return false;
		}
	}

	pVoteDelegateAccounts->push_back(delegateAccount);
	sortVoteDelegateAccount();

	return DataBase::getInstance()->saveDelegateAccount(delegateAccount);
}

bool DataMgr::deleteVoteDelegateAccount(int accountId)
{
	for (int i = 0; i < pVoteDelegateAccounts->size(); i++)
	{
		if (pVoteDelegateAccounts->at(i).id == accountId)
		{
			pVoteDelegateAccounts->remove(i);
			break;
		}
	}

	return DataBase::getInstance()->deleteDelegateAccount(accountId);
}

bool DataMgr::isVoteDelegate(int accountId)
{
	for (int i = 0; i < pVoteDelegateAccounts->size(); i++)
	{
		if (pVoteDelegateAccounts->at(i).id == accountId)
			return true;
	}

	return false;
}

int delegateAccountCompar(const DelegateAccount& a, const DelegateAccount& b)
{
	return a.id < b.id;
}

void DataMgr::sortVoteDelegateAccount()
{
	qSort(pVoteDelegateAccounts->begin(), pVoteDelegateAccounts->end(), delegateAccountCompar);
}

void DataMgr::walletAccountRegiste(QString& account_name, 
	QString& pay_from_account, QString& public_data, 
	int delegate_pay_rate, QString& account_type) {
}

void DataMgr::walletAccountRename(QString& current_name, QString& new_name) {
}

void DataMgr::walletAccountBalance(const QString& account_name)
{
    queryBalanceQueue.push_back(account_name);

    startQueryBalance();
}

bool DataMgr::canRequestBalance()
{
	return isRequestingBalance == false;
}

void DataMgr::startQueryBalance()
{
    if(isRequestingBalance)
        return;

    //qDebug() << "startQueryBalance" << queryBalanceQueue.size();

    if(queryBalanceQueue.size() > 0)
    {
        QMap<QString, CommonAccountInfo>::iterator itr;
        itr = _account_info_map.find(queryBalanceQueue.at(0));
        if(itr == _account_info_map.end())
        {
            queryBalanceQueue.pop_front();
            return;
        }

        isRequestingBalance = true;

        QString url = BALANCE_REQ + itr.value().address;
        //qDebug() << "startQueryBalance ==" << url;

        QNetworkRequest request;

        if (url.startsWith("https")) {
            QSslConfiguration config;
            config.setPeerVerifyMode(QSslSocket::VerifyNone);
            config.setProtocol(QSsl::TlsV1_0);
            request.setSslConfiguration(config);
        }
        request.setUrl(QUrl(url));
        queryBalanceReply = netmgr.get(request);
        connect(queryBalanceReply, &QNetworkReply::finished, this, &DataMgr::qureyBalanceHttpFinished);
        connect(queryBalanceReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(qureyBalanceHttpError(QNetworkReply::NetworkError)));
    }
}

void DataMgr::qureyBalanceHttpFinished()
{
	if (isRequestingBalance == false)
		return;

	if (queryBalanceReply == nullptr)
		return;

	QString content = QString(queryBalanceReply->readAll());
	queryBalanceReply->deleteLater();
	queryBalanceReply = NULL;

    QJsonParseError json_error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &json_error);
    //qDebug() << "qureyBalanceHttpFinished" << json_error.errorString();
    if (!doc.isEmpty())
	{
		QString accountName = queryBalanceQueue.at(0);
		queryBalanceQueue.pop_front();

        QJsonObject result = doc.object();
        auto balanceArray = result.value("data").toArray();

        QMap<QString, QString> balanceMap;

        for(int i = 0; i < balanceArray.size(); i++)
        {
            auto balanceObj = balanceArray[i].toObject();
            const QString& coinType = balanceObj.value("coinType").toString();

			if (_currency_name_map.find(coinType) == _currency_name_map.end())
				continue;

			double balance = balanceObj.value("balance").toDouble();
			if (balanceMap.contains(coinType))
				balanceMap.insert(coinType, QString::number((balance + balanceMap[coinType].toDouble()), 'f', 5));
			else
				balanceMap.insert(coinType, QString::number((balance), 'f', 5));
        }

        DataBase::getInstance()->updateAccount(accountName, balanceMap);
    }

    if (queryBalanceQueue.size() == 0)
	{
		isRequestingBalance = false;
		emit onWalletAccountBalance();
	}
	else
	{
		QTimer::singleShot(100, this, SLOT(delayQueryBalance()));
	}
}

void DataMgr::delayQueryBalance()
{
	isRequestingBalance = false;
	startQueryBalance();
}

void DataMgr::qureyBalanceHttpError(QNetworkReply::NetworkError errorCode)
{
    qDebug() << "DataMgr::qureyBalanceHttpError" << errorCode;
	if (queryBalanceReply == nullptr)
	{
		return;
	}
	qDebug() << "DataMgr::qureyBalanceHttpError" << queryBalanceReply->errorString();

	queryBalanceReply->deleteLater();
	queryBalanceReply = nullptr;
	netmgr.clearAccessCache();

	QTimer::singleShot(100, this, SLOT(delayQueryBalance()));
}

QString DataMgr::walletGetFormerNameByPriKey(const QString& pri_key)
{
	QString formerName("");

	auto okey = thinkyoung::blockchain::wif_to_key(pri_key.toStdString());
	if (okey.valid())
	{
		fc::ecc::private_key private_key = (*okey);
		thinkyoung::blockchain::Address addr(private_key.get_public_key());
		QString address = QString::fromStdString(addr.AddressToString());
		formerName = DataBase::getInstance()->queryFormerNameByAddress(address);
	}
	else
	{
		qDebug() << "private invalid!";
	}

	return formerName;
}

void DataMgr::tokenTransferTo(QString call_name, QString assetType, QString to_address, double amount, double fee, QString remark)
{
    QString call_addr;
	for (auto con : _currency_list) {
		if (con.name == DataMgr::getCurrCurrencyName()) {
            call_addr = con.contractId;
		}
	}
	QString param = to_address + "|" + QString::number(amount, 'f', 5) + "|" + remark;
    QString str("transfer_to");
    QString temp("");
    QString act("ACT");
    DataMgr::getInstance()->callContract(call_addr, call_name, str, param, act, fee);
}

void DataMgr::callContract(QString& contract, QString& call_name,
	QString& function_name, QString& params,
    QString& asset_symbol, double call_limit) {

    using thinkyoung::blockchain::ContractIdType;
    using thinkyoung::blockchain::AddressType;
    using std::string;

    const string caller = getInstance()->getAccountAddr(call_name).toStdString();
    const ContractIdType contract_id = ContractIdType(contract.toStdString(),
                                                      AddressType::contract_address);
    const string method = function_name.toStdString();
    const string arguments = params.toStdString();
    const string str_asset_symbol = asset_symbol.toStdString();
    double cost_limit = call_limit;
    auto trx = thinkyoung::blockchain::call_contract(caller, contract_id,
                                          method, arguments,
                                          str_asset_symbol, cost_limit);
    sign_broadcast(call_name, trx);
}

void DataMgr::changeToToolConfigPath() {
	QFile file("config.ini");
	if (!file.exists())     return;
	QFile file2(_tool_config_path + "/config.ini");
	qDebug() << file2.exists() << _tool_config_path + "/config.ini";
	if (file2.exists())
	{
		qDebug() << "remove config.ini : " << file.remove();
		return;
	}

	qDebug() << "copy config.ini : " << file.copy(_tool_config_path + "/config.ini");
	qDebug() << "remove old config.ini : " << file.remove();
}

void DataMgr::initSettings()
{
	getSystemEnvironmentPath();
    changeToToolConfigPath();

	_config_file = new QSettings(_tool_config_path + "/config.ini", QSettings::IniFormat);
    _config_file->setIniCodec("utf8");
    _current_account = _config_file->value("/settings/currentAccount").toString();

	if (_config_file->value("/settings/language").isNull())
		language = "English";
	else
		language = _config_file->value("/settings/language").toString();

	if (_config_file->value("/settings/lockMinutes").isNull())
		lockMinutes = 5;
	else
		lockMinutes = _config_file->value("/settings/lockMinutes").toInt();

	if (_config_file->value("/settings/notAutoLock").isNull())
		notAutoLock = false;
	else
		notAutoLock = _config_file->value("/settings/notAutoLock").toBool();

	if (_config_file->value("/settings/showPrivateTips").isNull())
		showPrivateTips = true;
	else
		showPrivateTips = _config_file->value("/settings/showPrivateTips").toBool();

	if (_config_file->value("/settings/hiddenTotalAssetVal").isNull())
		hiddenTotalAssetVal = false;
	else
		hiddenTotalAssetVal = _config_file->value("/settings/hiddenTotalAssetVal").toBool();

	if (_config_file->value("/settings/voteForDelegates").isNull())
		voteForDelegates = false;
	else
		voteForDelegates = _config_file->value("/settings/voteForDelegates").toBool();

	QFile file(_tool_config_path + "/log.txt");
	file.open(QIODevice::Truncate | QIODevice::WriteOnly);
	file.close();
}

void DataMgr::unInitSettings() {
	if (_config_file) {
		delete _config_file;
		_config_file = nullptr;
	}
}

void DataMgr::setLanguage(const QString& lang)
{
	language = lang;
	_config_file->setValue("/settings/language", language);
	_config_file->sync();
}

bool DataMgr::isHiddenTotalAsset()
{
	return hiddenTotalAssetVal;
}

void DataMgr::changeHiddenTotalAssetState()
{
	hiddenTotalAssetVal = !hiddenTotalAssetVal;
	_config_file->setValue("/settings/hiddenTotalAssetVal", hiddenTotalAssetVal);
	_config_file->sync();
}

bool DataMgr::isVoteForDelegates()
{
	return voteForDelegates;
}

void DataMgr::changeVoteForDelegatesState()
{
	voteForDelegates = !voteForDelegates;
	_config_file->setValue("/settings/voteForDelegates", voteForDelegates);
	_config_file->sync();
}

void DataMgr::getSystemEnvironmentPath() {
	QStringList environment = QProcess::systemEnvironment();

#ifdef WIN32
    for(QString str : environment) {
		if (str.startsWith("APPDATA=")) {
            _app_data_path = str.mid(8) + "\\AchainWalletLite";
            _tool_config_path = str.mid(8) + "\\AchainWalletLite";
			qDebug() << "appDataPath:" << _app_data_path;
			break;
		}
	}
#elif defined(TARGET_OS_MAC)
    for(QString str : environment) {
		if (str.startsWith("HOME=")) {
            _app_data_path = str.mid(5) + "/Library/Application Support/AchainWalletLite";
            _tool_config_path = str.mid(5) + "/Library/Application Support/AchainWalletLite";
			qDebug() << "appDataPath:" << _app_data_path;
			break;
		}
    }
    _app_work_path = QCoreApplication::applicationDirPath() + "/";
    qDebug() << "_app_work_path : " << _app_work_path;
    //QMessageBox::information(NULL, "Title", _app_work_path, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
#else
    for(QString str : environment) {
		if (str.startsWith("HOME=")) {
            _app_data_path = str.mid(5) + "/AchainWalletLite";
            _tool_config_path = str.mid(5) + "/AchainWalletLite";
			qDebug() << "appDataPath:" << _app_data_path;
			break;
		}
	}
#endif
}

void DataMgr::initCurrencyListFromDB()
{
	_currency_list = DataBase::getInstance()->queryCurrencyList();
	if (_currency_list.size() == 0)
	{
		CurrencyInfo currencyInfo = { 0, "0", COMMONASSET, COMMONASSET };
		_currency_list.push_back(currencyInfo);
	}

	qSort(_currency_list.begin(), _currency_list.end(), currencyListCmpare);


	for (int i = 0; i < _currency_list.size(); i++)
	{
		if (_currency_list.at(i).name == COMMONASSET)
		{
            if (i > 0)
				_currency_list.move(i, 0);
			break;
		}
	}

	DataMgr::getInstance()->setCurrentCurrency(_currency_list[0]);
}

QSettings* DataMgr::getSettings()
{
	return _config_file;
}

QString DataMgr::getAppDataPath()
{
	return _app_data_path;
}

QString DataMgr::getWorkPath()
{
	return _app_work_path;
}

const QVector<CurrencyInfo>& DataMgr::getCurrencyList()
{
	return _currency_list;
}

void DataMgr::setCurrentAccount(QString accountName)
{
	_current_account = accountName;
	_config_file->setValue("/settings/currentAccount", accountName);
}

QString DataMgr::getCurrentAccount()
{
	return  _current_account;
}

void DataMgr::setCurrentCurrency(const CurrencyInfo& currencyInfo)
{
	_current_currency = currencyInfo;
}

CurrencyInfo DataMgr::getCurrentCurrency()
{
	return  _current_currency;
}

QString DataMgr::getCurrCurrencyName()
{
	return _current_currency.name;
}

CurrencyInfo DataMgr::getCurrencyById(int id)
{
	CurrencyInfo currencyInfo;

	for (auto info : _currency_list)
	{
		if (info.id == id)
		{
			currencyInfo = info;
			break;
		}
	}
	
	return currencyInfo;
}

QMap<QString, QVector<TrxResult>>* DataMgr::getTrxResults()
{
	return &_trx_result_map;
}

QString DataMgr::getAccountAddr(QString account)
{
	return _account_info_map.value(account).address;
}

QString DataMgr::getAddrAccont(QString addr)
{
	for (CommonAccountInfo acount : _account_info_map) {
		if (addr == acount.address) {
			return acount.name;
		}
	}
	return QString("");
}

void DataMgr::walletListAccounts()
{
    auto accountInfos = DataBase::getInstance()->queryAllAccount();
	_account_info_map.clear();
	for (int i = 0; i < accountInfos.size(); i++)
	{
		_account_info_map.insert(accountInfos[i].name, accountInfos[i]);
	}

	if (_account_info_map.size() > 0 && getCurrentAccount().isEmpty())
	{
		setCurrentAccount(_account_info_map.keys().at(0));
	}
}

void DataMgr::walletUpdateAccounts()
{
	const QVector<CommonAccountInfo>& accountInfos = DataBase::getInstance()->queryAllAccount();
	_account_info_map.clear();
	for (int i = 0; i < accountInfos.size(); i++)
	{
		_account_info_map.insert(accountInfos[i].name, accountInfos[i]);
	}
}

QMap<QString, CommonAccountInfo>* DataMgr::getAccountInfo()
{
	return &_account_info_map;
}

QString DataMgr::getAccountPublicKey(const QString& account_name)
{
    QMap<QString, CommonAccountInfo>::iterator itr = _account_info_map.find(account_name);
    if (itr != _account_info_map.end())
    {
        return itr->ownerKey;
    }

    qDebug() << "DataMgr::getAccountPublicKey error, no accountInfo is found. account_name =" << account_name;

    return "";
}

QString DataMgr::getAccountPublicKeyFromAddr(const QString& address)
{
    for (const auto& map : _account_info_map){
        if (address == map.address){
            return map.ownerKey;
        }
    }
    qDebug() << "DataMgr::getAccountPublicKeyFromAddr error, no accountInfo is found. address =" << address;

    return "";
}

bool DataMgr::walletAccountDelete(QString& account_name)
{
	QMap<QString, CommonAccountInfo>::iterator itr = _account_info_map.find(account_name);
	if (itr != _account_info_map.end())
	{
		_account_info_map.remove(account_name);
		bool ret = DataBase::getInstance()->abandonAccount(account_name);
		if (ret)
		{
			if (getCurrentAccount() == account_name)
			{
				if (_account_info_map.size() > 0)
					setCurrentAccount(_account_info_map.keys().at(0));
				else
					setCurrentAccount("");
			}
		}
		return ret;
	}

	return false;
}

void DataMgr::walletDelegateAccounts(int page_n, int per_page_n)
{
	QNetworkRequest request;

	QString url = DELEGATE_ACCOUNT_LIST_URL;
	url += "page=" + QString::number(page_n);
	url += "&per_page=" + QString::number(per_page_n);

	if (url.startsWith("https")) {
		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}
	request.setUrl(QUrl(url));
	delegateAccountsReply = netmgr.get(request);
	connect(delegateAccountsReply, SIGNAL(finished()), this, SLOT(onDelegateAccountsFinished()));
	connect(delegateAccountsReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onDelegateAccountsError(QNetworkReply::NetworkError)));
}

void DataMgr::onDelegateAccountsFinished()
{
	if (delegateAccountsReply == nullptr)
		return;

	QByteArray content = delegateAccountsReply->readAll();
	delegateAccountsReply->deleteLater();
	delegateAccountsReply = nullptr;

	QJsonParseError json_error;
	QJsonDocument doc = QJsonDocument::fromJson(content, &json_error);

	QVector<DelegateAccount> vDelegateAccounts;

	if (!doc.isEmpty())
	{
		QJsonObject result = doc.object();
		auto delegatesObj = result.value("result").toObject();

		int totalPage = delegatesObj.value("totalPage").toInt();
		int currentPage = delegatesObj.value("currentPage").toInt();

		auto delegatesArray = delegatesObj.value("datalist").toArray();

		DelegateAccount delegateAccount;

		for (int i = 0; i < delegatesArray.size(); i++)
		{
			auto delegateObj = delegatesArray[i].toObject();

			delegateAccount.id = delegateObj.value("id").toInt();
			delegateAccount.name = delegateObj.value("name").toString();
			delegateAccount.votes_num = delegateObj.value("approval").toString();
			delegateAccount.votes_percent = delegateObj.value("payRate").toString();

			vDelegateAccounts.push_back(delegateAccount);
		}

		emit onWalletDelegateAccounts(true, currentPage, totalPage, vDelegateAccounts);
	}
}

void DataMgr::onDelegateAccountsError(QNetworkReply::NetworkError errorCode)
{
	qDebug() << "DataMgr::onDelegateAccountsError errorCode == " << QString::number(errorCode);

	if (delegateAccountsReply != nullptr)
	{
		delegateAccountsReply->deleteLater();
		delegateAccountsReply = nullptr;
	}

	emit onWalletDelegateAccounts(false, -1, -1, QVector<DelegateAccount>());
}

void DataMgr::walletDelegateAccountsByIds(const QVector<int>& ids)
{
	QNetworkRequest request;

	QString url(DELEGATE_ACCOUNT_DETAIL_URL);
	request.setUrl(QUrl(url));
	if (url.startsWith("https")) {
		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}

	QJsonObject sendData;
	QJsonArray idsArray;
	for (int i = 0; i < ids.size(); i++)
	{
		idsArray.append(ids.at(i));
	}
	sendData.insert("ids", idsArray);

	QJsonDocument doc(sendData);
	const QByteArray bytePost = QString(doc.toJson(QJsonDocument::Compact)).toUtf8();
	request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
	request.setHeader(QNetworkRequest::ContentLengthHeader, bytePost.length());
	delegateAccountsByIdsReply = netmgr.post(request, bytePost);
	connect(delegateAccountsByIdsReply, SIGNAL(finished()), this, SLOT(onDelegateAccountsByIdsFinished()));
	connect(delegateAccountsByIdsReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onDelegateAccountsByIdsError(QNetworkReply::NetworkError)));
}

void DataMgr::onDelegateAccountsByIdsFinished()
{
	if (delegateAccountsByIdsReply == nullptr)
		return;

	QByteArray content = delegateAccountsByIdsReply->readAll();
	delegateAccountsByIdsReply->deleteLater();
	delegateAccountsByIdsReply = nullptr;

	QJsonParseError json_error;
	QJsonDocument doc = QJsonDocument::fromJson(content, &json_error);

	QVector<DelegateAccount> vDelegateAccounts;

	if (!doc.isEmpty())
	{
		QJsonObject result = doc.object();
		auto delegatesArray = result.value("result").toArray();

		DelegateAccount delegateAccount;

		for (int i = 0; i < delegatesArray.size(); i++)
		{
			auto delegateObj = delegatesArray[i].toObject();

			delegateAccount.id = delegateObj.value("id").toInt();
			delegateAccount.name = delegateObj.value("name").toString();
			delegateAccount.votes_num = delegateObj.value("approval").toString();
			delegateAccount.votes_percent = delegateObj.value("payRate").toString();

			vDelegateAccounts.push_back(delegateAccount);
		}

		emit onWalletDelegateAccountsByIds(true, vDelegateAccounts);
	}
}

void DataMgr::onDelegateAccountsByIdsError(QNetworkReply::NetworkError errorCode)
{
	qDebug() << "DataMgr::onDelegateAccountsByIdsError errorCode == " << QString::number(errorCode);

	if (delegateAccountsByIdsReply != nullptr)
	{
		delegateAccountsByIdsReply->deleteLater();
		delegateAccountsByIdsReply = nullptr;
	}

	emit onWalletDelegateAccountsByIds(false, QVector<DelegateAccount>());
}

void DataMgr::walletGetAddressBalances(const QString& address)
{
	QNetworkRequest request;

	QString url = GET_ADDRESS_BALANCES;
	url += "addr=" + address;

	if (url.startsWith("https")) {
		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}
	request.setUrl(QUrl(url));
	getAddressBalancesReply = netmgr.get(request);
	connect(getAddressBalancesReply, SIGNAL(finished()), this, SLOT(onGetAddressBalancesFinished()));
	connect(getAddressBalancesReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onGetAddressBalancesError(QNetworkReply::NetworkError)));
}

void DataMgr::onGetAddressBalancesFinished()
{
	if (getAddressBalancesReply == nullptr)
		return;

	QByteArray content = getAddressBalancesReply->readAll();
	getAddressBalancesReply->deleteLater();
	getAddressBalancesReply = nullptr;

	QJsonParseError json_error;
	QJsonDocument doc = QJsonDocument::fromJson(content, &json_error);

	QVector<BalanceInfo> vBalanceInfos;

	if (!doc.isEmpty())
	{
		auto balancesArray = doc.array();

		BalanceInfo balanceInfo;

		for (int i = 0; i < balancesArray.size(); i++)
		{
			auto balanceObj = balancesArray[i].toArray();
			auto detailObj = balanceObj[1].toObject();

			balanceInfo.assetId = detailObj.value("condition").toObject().value("asset_id").toInt();
			if (balanceInfo.assetId == 0)
			{
				balanceInfo.balance_id = balanceObj[0].toString();
				balanceInfo.slateId = detailObj.value("condition").toObject().value("slate_id").toString().toULongLong();
				balanceInfo.owner = detailObj.value("condition").toObject().value("data").toObject().value("owner").toString();

				QString aaa = detailObj.value("balance").toString();

				if (detailObj.value("balance").isString())
					balanceInfo.balance = detailObj.value("balance").toString().toLongLong();
				else
					balanceInfo.balance = detailObj.value("balance").toDouble();

				vBalanceInfos.push_back(balanceInfo);
			}
		}

		emit onWalletGetAddressBalances(true, vBalanceInfos);
	}
}

void DataMgr::onGetAddressBalancesError(QNetworkReply::NetworkError errorCode)
{
	qDebug() << "DataMgr::onDelegateAccountsError errorCode == " << QString::number(errorCode);

	if (getAddressBalancesReply != nullptr)
	{
		getAddressBalancesReply->deleteLater();
		getAddressBalancesReply = nullptr;
	}

	emit onWalletGetAddressBalances(false, QVector<BalanceInfo>());
}
