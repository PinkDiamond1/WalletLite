#include <QApplication>

#ifdef WIN32
#include "Windows.h"
#include "DbgHelp.h"
#include "tchar.h"
#include "ShlObj.h"
#endif

#include "datamgr.h"
#include "database.h"
#include "thirddatamgr.h"
#include "macro.h"
#include "goopal.h"
#include "frame.h"
#include "outputmessage.h"
#include "misc.h"

#include <QNetworkInterface>
#include <QDebug>
#include <qapplication.h>
#include <QTranslator>
#include <QThread>
#include <QTextCodec>
#include <QDir>

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bool checkOnly()
{
    const char filename[]  = "/tmp/AchainWalletLite";
    int fd = open (filename, O_WRONLY | O_CREAT , 0644);
    int flock = lockf(fd, F_TLOCK, 0 );
    if (fd == -1) {
            perror("open lockfile/n");
            return false;
    }
    //给文件加锁
    if (flock == -1) {
            perror("lock file error/n");
            return false;
    }
    //程序退出后，文件自动解锁
    return true;
}
#else
bool checkOnly()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    HANDLE m_hMutex = CreateMutex(NULL, FALSE, L"AchainWalletLite");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(m_hMutex);
        m_hMutex = NULL;

        return  false;
    }
    else
        return true;
}

LONG WINAPI TopLevelExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)  //zxlwin
{
    qDebug() << "Enter TopLevelExceptionFilter Function" ;
    HANDLE hFile = CreateFile(L"project.dmp",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    MINIDUMP_EXCEPTION_INFORMATION stExceptionParam;
    stExceptionParam.ThreadId    = GetCurrentThreadId();
    stExceptionParam.ExceptionPointers = pExceptionInfo;
    stExceptionParam.ClientPointers    = FALSE;
    MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile,MiniDumpWithFullMemory,&stExceptionParam,NULL,NULL);
    CloseHandle(hFile);

    qDebug() << "End TopLevelExceptionFilter Function" ;
    return EXCEPTION_EXECUTE_HANDLER;
}

void refreshIcon()  //zxlwin
{
    // 解决windows下自动更新后图标不改变的bug
    SHChangeNotify(0x8000000, 0x1000, 0, 0);
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, NULL, SMTO_ABORTIFHUNG, 100, 0);
}
#endif // WIN32

inline QString getMAC()
{
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    QString MAC = "";
    if(list.size() != 0) {
        MAC = list[0].hardwareAddress();
    }
    return MAC;
}

inline QString getUrlData(QString url)
{
    QString allData = "mac=" + getMAC() + "&version=" + ACHAIN_WALLET_VERSION;
#ifdef _DEBUG
    std::string urlEncode = allData.toUtf8().toPercentEncoding().toStdString();
#endif // _DEBUG
    return url + "?" + allData.toUtf8().toPercentEncoding().toBase64();
}
inline bool uploadData(QString url)
{
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
    QByteArray responseData;
    QEventLoop eventLoop;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    responseData = reply->readAll();
    qDebug() << "reply->error()" << reply->error();
    if (QNetworkReply::NoError != reply->error())
    {
        return false;
    }
    return true;
}

inline QByteArray getPostData()
{
    QJsonObject jsonObj;
    jsonObj.insert("MAC", getMAC());
	jsonObj.insert("version", "Lite " ACHAIN_WALLET_VERSION_STR);
    QJsonDocument doc(jsonObj);
    QByteArray postData = doc.toJson();
    return postData;
}

inline bool uploadPostData(QString url, QByteArray data)
{
    QByteArray post_data;
    post_data.append("data=" + data);

    QNetworkRequest network_request;
    network_request.setUrl(url);
    network_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    network_request.setHeader(QNetworkRequest::ContentLengthHeader, post_data.length());

    if (url.startsWith("https")) {
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_0);
        network_request.setSslConfiguration(config);
    }

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(network_request, post_data);

    QByteArray responseData;
    QEventLoop eventLoop;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    responseData = reply->readAll();
    //qDebug() << "reply->error()" << reply->error();
    if (QNetworkReply::NoError != reply->error())
    {
        return false;
    }
    return true;
}

void writeLogMain(QString log)
{
    QFile file("./Achain.log");
    //方式：Append为追加，WriteOnly，ReadOnly
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << log << endl;
    out.flush();
    file.close();
}

int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);

#ifdef WIN32
    refreshIcon();  //zxlwin
    SetUnhandledExceptionFilter(TopLevelExceptionFilter);  //zxlwin
#endif

    if(checkOnly()==false)  return 0;    // 防止程序多次启动  //zxlwin

    DataBase::getInstance();
	ThirdDataMgr::getInstance();
    DataMgr::getDataMgr();
	DataMgr::getDataMgr()->initCurrencyListFromDB();
	DataMgr::getDataMgr()->walletListAccounts();

#ifdef WIN32
	QSettings config(QCoreApplication::applicationDirPath() + "/" + VERSION_CONFIG, QSettings::IniFormat);
	if (config.value("download").toBool())
	{
		QString app_path = QCoreApplication::applicationDirPath() + "/" + UPDATE_TOOL_NAME;
		QStringList args = QStringList() << "install" << DataMgr::getInstance()->getConfigPath();
		bool result = QProcess::startDetached(app_path, args);
		writeLogMain(result ? "successed" : "failed");

		DataBase::destroyInstance();
		ThirdDataMgr::deleteInstance();
		DataMgr::deleteDataMgr();
		a.quit();

		return 0;
	}
#endif

	Frame*  frame = new Frame();
    Goopal::getInstance()->mainFrame = frame;   // 一个全局的指向主窗口的指针

    frame->show();
    a.installEventFilter(frame);

    QByteArray postData = getPostData();
    uploadPostData(REPORT_REQ, postData);

    a.setStyle("Windows");
    int result = a.exec();

	frame->close();
	delete frame;

    DataMgr::deleteDataMgr();
	ThirdDataMgr::deleteInstance();
    DataBase::destroyInstance();

    DLOG_QT_WALLET_FUNCTION_END;

    return result;
}
