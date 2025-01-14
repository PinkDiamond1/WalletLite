#ifndef TRACKMGR_H
#define TRACKMGR_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

struct TrackData
{
	QString type;
	QString uid;
	QString platform;
	QString mac;

	QByteArray toJsonByteArray() const
	{
		QJsonObject obj;
		obj.insert("type", type);
		obj.insert("uid", uid);
		obj.insert("platform", platform);
		obj.insert("mac", mac);

		QJsonDocument jsonDoc;
		jsonDoc.setObject(obj);
		const QByteArray& byteArray = jsonDoc.toJson(QJsonDocument::Compact);

		return byteArray;
	}
};

class TrackMgr : public QObject
{
    Q_OBJECT
public:
	static TrackMgr* getInstance();
	static void destroyInstance();

	void login();

private:
	TrackMgr();
	~TrackMgr();

    QString getUid();
    QString genUid();
    QString getHostMacAddress();
	void stratAutoHeartbeat();
	void sendTrackData(TrackData& trackData);

public slots :
	void heartbeat();
	void requestTrackFinished();
	void requestTrackError(QNetworkReply::NetworkError errorCode);
	void sendTrackDataToServer();

private:
	static TrackMgr* _track_mgr;
	QString uid = "";
	QString macAddr = "";
	QTimer* timerHeartbeat = nullptr;
	QVector<TrackData> trackMsgVec;

	QNetworkAccessManager netmgrTrack;
	QNetworkReply* netReplyTrack = nullptr;
	bool isSendingTrackData = false;
};

#endif // TRACKMGR_H
