#ifndef __THIRD_DATA_MGR_H__
#define __THIRD_DATA_MGR_H__

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>

struct QuotationInfo
{
	int id;
	QString name;
	QString symbol;
	int rank;
	
	double price_usd;
	double volume_usd_24h;
	double market_cap_usd;
	double percent_change_1h;
	double percent_change_24h;
	double percent_change_7d;

	long long last_updated;
};

class ThirdDataMgr : public QObject
{
	Q_OBJECT
public:
	static ThirdDataMgr* getInstance();
	static void deleteInstance();

	double getAssetPrice(const QString& assetName);
	const QVector<QuotationInfo*>* getQuotationInfoList();

	void setCurrentCurrencyName(const QString& currencyName);
	QString getCurrentCurrencyName();
	double getCurrentExchangeRate();

	QVector<QString> getDisplayCurrencyList();
	QString getCurrencySymbol(const QString currencyName);
	QString getCurrentCurrencySymbol();

private:
	ThirdDataMgr();
	~ThirdDataMgr();

	void clearQuotationInfo();
	void resetQuotationInfo();

signals:
	void onReqQuotationFinished(bool);

public slots:
	void requestQuotationInfo();
	void requestExchageRateInfo();

	void requestQuotationFinished();
	void requestQuotationError(QNetworkReply::NetworkError errorCode);

	void requestExchageRateFinished();
	void requestExchageRateError(QNetworkReply::NetworkError errorCode);

private:
	static ThirdDataMgr* _instance;

	QNetworkAccessManager netmgr_quotation;
	QNetworkReply* netReply_quotation;

	QNetworkAccessManager netmgr_exchange_rate;
	QNetworkReply* netReply_exchange_rate;
	
	bool firstReqQuotation = true;
	QVector<QuotationInfo*>* quotationInfoList = nullptr;
	QMap<QString, QuotationInfo*>* quotationInfoMap = nullptr;

	QMap<QString, double> exchangeRateMap;

	QString currentCurrencyName;
	QMap<QString, QString> currencySymbolMap;

};
#endif // __THIRD_DATA_MGR_H__
