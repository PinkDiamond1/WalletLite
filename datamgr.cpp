#include "datamgr.h"
#include <assert.h>
#include <algorithm>
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

void sign_broadcast(const QString& from_account_name,
                    thinkyoung::blockchain::SignedTransaction& trx){

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

void DataMgr::onbroadcastFinished(){
    QNetworkReply* network_reply = qobject_cast<QNetworkReply*>(sender());
    const QByteArray& reply_content = network_reply->readAll();
    QString json_str(reply_content);

	if (QNetworkReply::NoError != network_reply->error())
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

void DataMgr::onbroadcastError(QNetworkReply::NetworkError errorCode){
    QNetworkReply *pReplay = qobject_cast<QNetworkReply*>(sender());
    qDebug() << errorCode;
    qDebug() << pReplay->errorString();

    emit onWalletTransferToAddressWithId(false, "", "");
}

void DataMgr::broadcast(const QString& json) {
    //next version contract from net
    QNetworkAccessManager* pManager = new QNetworkAccessManager(this);
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
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, bytePost.length());
    QNetworkReply *pReply = pManager->post(request, bytePost);

    connect(pReply, SIGNAL(finished()), this, SLOT(onbroadcastFinished()));
    connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(onbroadcastError(QNetworkReply::NetworkError)));
}

void DataMgr::fetchAssetInfo() {

    qDebug() << "DataMgr::fetchAssetInfo";

	//next version contract from net
	QNetworkAccessManager* pManager = new QNetworkAccessManager(this);
	QNetworkRequest request;

	QString url(CON_CONFOG_URL);
	request.setUrl(QUrl(url));
    if (url.startsWith("https")) {
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}
	QNetworkReply *pReply = pManager->get(request);
	connect(pReply, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
}

void DataMgr::onFinished() {
    QNetworkReply* network_reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray reply_content = network_reply->readAll();
    QString json_str(reply_content);

    if (QNetworkReply::NoError == network_reply->error() && 0 != json_str.size()) {
        parseContract(json_str);
        //saved result
		DataBase::getInstance()->updateCurrencyList(_currency_list);

        if (nullptr != timerGetAsset) {//timer
            timerGetAsset->stop();
            delete timerGetAsset;
            timerGetAsset = nullptr;
        }

        emit assetTypeGet();
    }
    else if (nullptr == timerGetAsset) {
        timerGetAsset = new QTimer(this);
        connect(timerGetAsset, SIGNAL(timeout()), this, SLOT(fetchAssetInfo()));
        timerGetAsset->start(10 * 60 * 1000);
    }
}

void DataMgr::onError(QNetworkReply::NetworkError errorCode){
    QNetworkReply *pReplay = qobject_cast<QNetworkReply*>(sender());
    qDebug() << errorCode;
    qDebug() << pReplay->errorString();

    if (nullptr == timerGetAsset) {
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

void DataMgr::walletAccountSetApproval(QString& account_name, int approval) {
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

    auto trx =  thinkyoung::blockchain::wallet_transfer_to_address(
                                        str_amount_to_transfer,
                                        asset_id, str_from_address,
                                        str_to_address, str_memo_message);

    sign_broadcast(from_account_name, trx);
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

            const QString& balance = QString::number((balanceObj.value("balance").toDouble()), 'f', 5);
            balanceMap.insert(coinType, balance);
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

void DataMgr::tokenQueryTransferlog(QString conAddr, QString call_name)
{
    QString str("query_transfer_log");
    QString temp("");
    QString act("ACT");
    DataMgr::getInstance()->callContractTesting(conAddr, call_name, str, temp, act);
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

void DataMgr::callContractTesting(QString& contract, QString& call_name,
	QString& function_name, QString& params, QString& asset_symbol) {
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
