#include "thirddatamgr.h"
#include <assert.h>
#include <algorithm>
#include "misc.h"

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

ThirdDataMgr* ThirdDataMgr::_instance = nullptr;

ThirdDataMgr* ThirdDataMgr::getInstance()
{
	if (!_instance)
        _instance = new ThirdDataMgr();
		
	return _instance;
}

void ThirdDataMgr::deleteInstance()
{
	if (_instance)
		delete _instance;
	
	_instance = nullptr;
}

ThirdDataMgr::ThirdDataMgr()
{
	resetQuotationInfo();

	requestQuotationInfo();
	requestExchageRateInfo();

	currencySymbolMap.insert("CNY", QString::fromLocal8Bit("￥"));
	currencySymbolMap.insert("USD", "$");
	currentCurrencyName = "CNY";
}

ThirdDataMgr::~ThirdDataMgr()
{
	clearQuotationInfo();
}

void ThirdDataMgr::clearQuotationInfo()
{
	if (quotationInfoList != nullptr)
	{
		for (int i = quotationInfoList->size() - 1; i >= 0; i--)
		{
			delete quotationInfoList->at(i);
		}

		delete quotationInfoList;
		delete quotationInfoMap;
	}
}

void ThirdDataMgr::resetQuotationInfo()
{
	clearQuotationInfo();

	quotationInfoList = new QVector<QuotationInfo*>();
	quotationInfoMap = new QMap<QString, QuotationInfo*>();
}

// -1: no price data
double ThirdDataMgr::getAssetPrice(const QString& assetName)
{
	auto quotationInfoItr = quotationInfoMap->find(assetName);
	if (quotationInfoItr != quotationInfoMap->end())
	{
		auto quotationInfo = quotationInfoItr.value();
		return quotationInfo->price_usd * getCurrentExchangeRate();
	}

	return -1;
}

const QVector<QuotationInfo*>* ThirdDataMgr::getQuotationInfoList()
{
	return quotationInfoList;
}

void ThirdDataMgr::setCurrentCurrencyName(const QString& currencyName)
{
	currentCurrencyName = currencyName;
}

QString ThirdDataMgr::getCurrentCurrencyName()
{
	return currentCurrencyName;
}

double ThirdDataMgr::getCurrentExchangeRate()
{
	double currentExchangeRate = 1;

	auto exchangeRateItr = exchangeRateMap.find(currentCurrencyName);
	if (exchangeRateItr != exchangeRateMap.end())
	{
		currentExchangeRate = exchangeRateItr.value();
	}

	return currentExchangeRate;
}

QVector<QString> ThirdDataMgr::getDisplayCurrencyList()
{
	return QVector<QString>{"CNY", "USD"};
}

QString ThirdDataMgr::getCurrencySymbol(const QString currencyName)
{
	auto itr = currencySymbolMap.find(currencyName);
	if (itr != currencySymbolMap.end())
	{
		return currencySymbolMap[currencyName];
	}

	return "";
}

QString ThirdDataMgr::getCurrentCurrencySymbol()
{
	return getCurrencySymbol(currentCurrencyName);
}

void ThirdDataMgr::requestQuotationInfo()
{
	QString url = QUOTATION_URL;

	QByteArray post_data;
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setHeader(QNetworkRequest::ContentLengthHeader, post_data.length());

	if (url.startsWith("https")) {
		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}
	request.setUrl(QUrl(url));
	netReply_quotation = netmgr_quotation.post(request, post_data);
	connect(netReply_quotation, &QNetworkReply::finished, this, &ThirdDataMgr::requestQuotationFinished);
	connect(netReply_quotation, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(requestQuotationError(QNetworkReply::NetworkError)));
}

void ThirdDataMgr::requestQuotationFinished()
{
	if (netReply_quotation == nullptr)
		return;

	const QByteArray& byteArray = netReply_quotation->readAll();
	netReply_quotation->deleteLater();
	netReply_quotation = NULL;

	QJsonParseError json_error;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &json_error);
	if (!doc.isEmpty())
	{
		resetQuotationInfo();
		auto quotationArray = doc.array();

		for (int i = 0; i < quotationArray.size(); i++)
		{
			auto quotationObj = quotationArray[i].toObject();

			QuotationInfo* quotationInfo = new QuotationInfo;
			//quotationInfo->id = quotationObj.value("id").toInt();
			//quotationInfo->name = quotationObj.value("name").toString();
			quotationInfo->symbol = quotationObj.value("symbol").toString();
			quotationInfo->rank = quotationObj.value("rank").toInt();
			//quotationInfo->last_updated = quotationObj.value("last_updated").toDouble();

			auto quotesObj = quotationObj.value("quotes").toObject();
			auto usdPriceObj = quotesObj.value("USD").toObject();
			quotationInfo->price_usd = usdPriceObj.value("price").toDouble();
			//quotationInfo->volume_usd_24h = usdPriceObj.value("volume_24h").toDouble();
			//quotationInfo->market_cap_usd = usdPriceObj.value("market_cap").toDouble();
			quotationInfo->percent_change_1h = usdPriceObj.value("percent_change_1h").toDouble();
			quotationInfo->percent_change_24h = usdPriceObj.value("percent_change_24h").toDouble();
			quotationInfo->percent_change_7d = usdPriceObj.value("percent_change_7d").toDouble();

			quotationInfoList->push_back(quotationInfo);
			quotationInfoMap->insert(quotationInfo->symbol, quotationInfo);
		}

		emit onReqQuotationFinished();
	}
	else
	{
		qDebug() << "requestQuotationFinished parse Json : " << json_error.errorString();
	}
}

void ThirdDataMgr::requestQuotationError(QNetworkReply::NetworkError errorCode)
{
	netReply_quotation->deleteLater();
	netReply_quotation = nullptr;
	netmgr_quotation.clearAccessCache();

	QTimer::singleShot(1000, this, SLOT(requestQuotationInfo()));
}

void ThirdDataMgr::requestExchageRateInfo()
{
	QString url = EXCHANGE_RATE_URL;

	QByteArray post_data;
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setHeader(QNetworkRequest::ContentLengthHeader, post_data.length());

	if (url.startsWith("https")) {
		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}
	request.setUrl(QUrl(url));
	netReply_exchange_rate = netmgr_exchange_rate.post(request, post_data);
	connect(netReply_exchange_rate, &QNetworkReply::finished, this, &ThirdDataMgr::requestExchageRateFinished);
	connect(netReply_exchange_rate, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(requestExchageRateError(QNetworkReply::NetworkError)));
}

void ThirdDataMgr::requestExchageRateFinished()
{
	if (netReply_exchange_rate == nullptr)
		return;

	const QByteArray& content = netReply_exchange_rate->readAll();
	netReply_exchange_rate->deleteLater();
	netReply_exchange_rate = NULL;

	QJsonParseError json_error;
	QJsonDocument doc = QJsonDocument::fromJson(content, &json_error);
	if (!doc.isEmpty())
	{
		QJsonObject ratesObj = doc.object();
		exchangeRateMap.clear();

		auto ratesObjItr = ratesObj.begin();
		for (; ratesObjItr != ratesObj.end(); ratesObjItr++)
		{
			const QString& symbol = ratesObjItr.key();
			double exchangeRate = ratesObjItr.value().toDouble();
			exchangeRateMap.insert(symbol, exchangeRate);
		}
	}
	else
	{
		qDebug() << "requestExchageRateFinished parse Json : " << json_error.errorString();
	}
}

void ThirdDataMgr::requestExchageRateError(QNetworkReply::NetworkError errorCode)
{
	netReply_exchange_rate->deleteLater();
	netReply_exchange_rate = nullptr;
	netmgr_exchange_rate.clearAccessCache();

	QTimer::singleShot(1000, this, SLOT(requestExchageRateInfo()));
}
