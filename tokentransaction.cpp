#include "tokentransaction.h"
#include "macro.h"
#include "misc.h"
#include "datamgr.h"

TokenTransaction::TokenTransaction() {
    token_req = false;
}

TokenTransaction::~TokenTransaction() {
}

void TokenTransaction::trxFromJson(QJsonObject& result_obj)
{
    QJsonObject obj = result_obj.value("result").toObject();
    obj.value("currentPage");
    QJsonArray trxarr = obj.value("dataList").toArray();
    //qDebug() << trxarr.size();
    trxvector.totaRecords = obj.value("totalRecords").toInt();
    trxvector.currentPage = obj.value("currentPage").toInt();
    trxvector.totalPage = obj.value("totalPage").toInt();
    trxvector.pageSize = obj.value("pageSize").toInt();
    trxvector.trx.clear();

	const QString& currencyName = DataMgr::getInstance()->getCurrCurrencyName();
    
    for(QJsonValue trxrecords: trxarr) {
        QJsonObject trxobj = trxrecords.toObject();
        
        //fail transfer
		const QString& coinType = trxobj.value("coinType").toString();
		//const QString& eventType = trxobj.value("eventType").toString();

		//if (coinType != currencyName || eventType == "transfer_to_fail") {
		if (currencyName != COMMONASSET && coinType != currencyName)
		{
            trxvector.totaRecords--;
            if (trxvector.totaRecords <= 0)
			{
                trxvector.totaRecords = 0;
            }
            continue;
        }
        
        Transaction trx;
		trx.cointype = coinType;
        trx.from_addr = trxobj.value("from_addr").toString();
        trx.from_acct = trxobj.value("from_acct").toString();
        trx.to_addr = trxobj.value("to_addr").toString();
        trx.to_acct = trxobj.value("to_acct").toString();
		trx.trx_time = trxobj.value("trx_time").toString();
        trx.memo = trxobj.value("memo").toString();
		trx.amount = trxobj.value("amount").toString();
		trx.fee = trxobj.value("fee").toString();

		/*
        QString eventParam = trxobj.value("eventParam").toString();
        if (eventParam.isEmpty()) {
            return;
        }
        QString frombalance = eventParam.split(',').at(0);
        frombalance = frombalance.split(':').at(1);
        QString tobalance = eventParam.split(',').at(1);
        tobalance = tobalance.split(':').at(1);
        trx.frombalance = QString::number(frombalance.toDouble() / 100000, 'f', 5);
        trx.tobalance = QString::number(tobalance.toDouble() / 100000, 'f', 5);
		*/
        trxvector.trx.append(trx);
    }
}

void urlAddParam(QString& url, QString& param) {
    url += param + "&";
}

void TokenTransaction::balanceRequestSet(QString& url, QString account_addr, QString contract_id,
        int page_n, int per_page_n, int block_n) {
	QString request = TRANSACTION_REQ;
    QString per_page = "per_page=" + QString::number(per_page_n);
    QString acct_addr = "acct_address=" + account_addr;
    QString page = "page=" + QString::number(page_n);
    QString asset_type = "contract_id=" + contract_id;
    url = request;
    urlAddParam(url, acct_addr);
    urlAddParam(url, asset_type);
    urlAddParam(url, page);
    urlAddParam(url, per_page);
}

//int request = false;
void TokenTransaction::showTokenTraction(TrxVector& trxvector) {
    int i = 0;
    
    for(Transaction trx: trxvector.trx) {
        i++;
    }
}

void TokenTransaction::httpFinished()
{
	const QByteArray& byteArray = pReply->readAll();
	QJsonDocument doc = QJsonDocument::fromJson(byteArray);

	if (doc.isObject()) {
		QJsonObject result = doc.object();
		trxFromJson(result);
	}

    pReply->deleteLater();
    pReply = NULL;
    emit tokenTrxRequestEnd();
    token_req = false;
}

void TokenTransaction::httpError(QNetworkReply::NetworkError errorCode)
{
	qDebug() << "TokenTransaction::httpError == " << errorCode << pReply->errorString();
}

void TokenTransaction::connectToBlockBrower(QString account_addr, QString contract_id,
        int page_n, int per_page_n, int block_n) {
    if (token_req) {
        return;
    }
    
    QNetworkRequest request;
    QString url;
    balanceRequestSet(url, account_addr, contract_id, page_n, per_page_n, block_n);
    trxvector.addr = account_addr;
    trxvector.page = page_n;
    trxvector.contract_id = contract_id;
    //  balanceRequestSet(url, account_addr, asset, page_n, per_page_n);
    
    if (url.startsWith("https")) {
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_0);
        request.setSslConfiguration(config);
    }

    //qDebug() << "TokenTransaction::connectToBlockBrower" << url;
    
    request.setUrl(QUrl(url));
    pReply = netmgr.get(request);
    token_req = true;
	connect(pReply, &QNetworkReply::finished, this, &TokenTransaction::httpFinished);
	connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(httpError(QNetworkReply::NetworkError)));
}

void TokenTransaction::clearTrxVector()
{
	trxvector.totaRecords = 0;
	trxvector.currentPage = 0;
	trxvector.totalPage = 0;
	trxvector.pageSize = 0;
	trxvector.trx.clear();
}
