#include "trackmgr.h"
#include "macro.h"

#include <QNetworkInterface>
#include <QList>
#include <QUuid>
#include <QSettings>
#include <QTimeZone>

#define TRACK_TYPE_LOGIN "login"
#define TRACK_TYPE_HEARTBREAT "heartbeat"


TrackMgr* TrackMgr::_track_mgr = nullptr;

TrackMgr* TrackMgr::getInstance()
{
	if (_track_mgr == nullptr) {
		_track_mgr = new TrackMgr();
	}
	return _track_mgr;
}

void TrackMgr::destroyInstance()
{
	if (_track_mgr != nullptr) {
		delete _track_mgr;
		_track_mgr = nullptr;
	}
}

TrackMgr::TrackMgr()
{
	uuid = getUuid();
	macAddr = getHostMacAddress();
	trackMsgVec.clear();
}

TrackMgr::~TrackMgr()
{
	if (timerHeartbeat != nullptr)
	{
		delete timerHeartbeat;
		timerHeartbeat = nullptr;
	}
}

QString TrackMgr::getUuid()
{
#ifdef WIN32
	QSettings settings("HKEY_CURRENT_USER\\Software\\AchainWalletLite", QSettings::NativeFormat);
	QString _uuid = settings.value("uuid", "").toString();
	if (_uuid.isEmpty())
	{
		_uuid = genUuid();
		settings.setValue("uuid", _uuid);
	}
#else

#endif

	return _uuid;
}

QString TrackMgr::getHostMacAddress()
{
	QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
	int nCnt = nets.count();
	QString strMacAddr = "";
	for (int i = 0; i < nCnt; i++)
	{
		// 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
		if (nets[i].flags().testFlag(QNetworkInterface::IsUp)
			&& nets[i].flags().testFlag(QNetworkInterface::IsRunning) 
			&& !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
		{
			strMacAddr = nets[i].hardwareAddress();
			break;
		}
	}
	return strMacAddr;
}

QString TrackMgr::genUuid()
{
	return QUuid::createUuid().toString().remove("{").remove("}").remove("-");
}

void TrackMgr::stratAutoHeartbeat()
{
	if (timerHeartbeat == nullptr)
	{
		timerHeartbeat = new QTimer(this);
		connect(timerHeartbeat, SIGNAL(timeout()), this, SLOT(heartbeat()));
		timerHeartbeat->start(40 * 60 * 1000);
	}
}

void TrackMgr::sendTrackData(TrackData& tData)
{
	tData.uuid = uuid;
	tData.mac = macAddr;
#ifdef WIN32
	tData.platform = "win";
#else
	tata.platform = "mac";
#endif

	trackMsgVec.push_back(tData);
	sendTrackDataToServer();
}

void TrackMgr::sendTrackDataToServer()
{
	if (isSendingTrackData || trackMsgVec.size() == 0)
		return;

	const TrackData& tData = trackMsgVec.at(0);
	QByteArray postData = tData.toJsonByteArray();
	postData = QString("data=").toUtf8() + postData;
	trackMsgVec.pop_front();

	QString url = TRACK_URL;
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setHeader(QNetworkRequest::ContentLengthHeader, postData.length());

	if (url.startsWith("https")) {
		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0);
		request.setSslConfiguration(config);
	}
	request.setUrl(QUrl(url));
	netReplyTrack = netmgrTrack.post(request, postData);
	connect(netReplyTrack, &QNetworkReply::finished, this, &TrackMgr::requestTrackFinished);
	connect(netReplyTrack, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(requestTrackError(QNetworkReply::NetworkError)));

	isSendingTrackData = true;
}

void TrackMgr::requestTrackFinished()
{
	if (netReplyTrack == nullptr)
		return;

	netReplyTrack->deleteLater();
	netReplyTrack = nullptr;

	isSendingTrackData = false;
	QTimer::singleShot(1000, this, SLOT(sendTrackDataToServer()));
}

void TrackMgr::requestTrackError(QNetworkReply::NetworkError errorCode)
{
	netReplyTrack->deleteLater();
	netReplyTrack = nullptr;
	netmgrTrack.clearAccessCache();
	
	isSendingTrackData = false;
	QTimer::singleShot(1000, this, SLOT(sendTrackDataToServer()));
}

void TrackMgr::login()
{
	TrackData tData;
	tData.type = TRACK_TYPE_LOGIN;
	sendTrackData(tData);

	stratAutoHeartbeat();
}

void TrackMgr::heartbeat()
{
	TrackData tData;
	tData.type = TRACK_TYPE_HEARTBREAT;
	sendTrackData(tData);
}


