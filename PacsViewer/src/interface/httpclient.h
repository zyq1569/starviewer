﻿#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H


#include "logging.h"

#include <QObject>
#include <QUrl>
//#include <QSslError>
#include <QNetworkAccessManager>
#include <QList>
#include <QDir>

class QNetworkReply;
class QAuthenticator;
class QFile;
class HManageThread;


///-----------------------------------------------------------------------------------------------------------------------
enum DownFileType
{
    dcm,
    dbinfo,
    studyini,
    other
};

typedef  QString  OFString;
struct DicomFileInfo
{
    OFString patientName,patientSex, patientAge,patientBirthDate, patientBirthTime,  patientId;
    OFString modality, manufacturer, institutionName;
    OFString studyId, studyUID,studyDescription,studyDate, studyTime;
    OFString seriesUID, seriesDescription, seriesNumber;
    OFString imageSOPInstanceUID, instanceNumber;
};

struct ImageInfo
{
    //(0008,0018) UI 1.2.840.113619.2.55.3.604688119.699.1256270047.398.2   # 1, 52 SOP Instance UID
    OFString imageSOPInstanceUID;
    // (0020, 0013) IS 2                                                 # 1, 2 Instance Number;
    int instanceNumber;
};

struct SeriesInfo
{
    QList<ImageInfo> imagesInfoList;
    OFString seriesUID;
    OFString seriesDescription;
    //Series Number
    int seriesNumber;
};

struct StudyInfo
{
    OFString patientName, patientId, modality, studyId, studyUID, studyDate, studyDescription;
    QList<SeriesInfo> seriesInfoList;
};

struct HSeries
{
    QString SeriesUID;
    QList <QString> ImageSOPUI;
};

struct HStudy
{
    QString StudyUID,StudyDate;
    int imageCount;
    QList <HSeries> Serieslist;
};

///----------------------------------------------------------------------------------------
struct StudyRowInfo
{
    QString patientIdentity,patientName,patientId,patientSex,patientBirthday,patientTelNumber;
    QString patientAddr,patientCarID,patientType,patientEmail,studyOrderIdentity,studyId,studyuid;
    QString scheduledDateTime,ScheduledDate,orderDateTime,studyDescription,studyModality,aETitle;
    QString studyType,studyCode,studyState,studyCost,studyDate,studyDepart,sStudyModality,costType;
};

struct PatientStudyDB
{
    int code;
    QString msg,ver;
    int count;
    QList<StudyRowInfo> rowinfo;
};

///----------------------------------------------------------------------------------------
class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = NULL, QString dir = "");

public:
    void setDwonloadDir(QString dir);

    void getStudyImageFile(QUrl url,QString studyuid="",QString seruid = "", QString imguid = "");

    bool CreatDir(QString fullPath);

    void setHttpServerHost(QString host);

    QString getHttpServerHost();

    QStringList* getListStudyUID();

    const HManageThread* getManageThread();

signals:
    void parseDataFinished();
    void allFilesFinished();

public slots:
    void allFilesThreadFinished();

private slots:
    void cancelDownload();
    void httpFinished();
    void httpReadyRead();
    void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

private:
    bool openFileForWrite(const QString &fileName);
    void downFileFromWeb(QUrl httpUrl,QString savefilename,QString downDir);
    void startRequest(const QUrl &requestedUrl);
    void ParseDwonData();

private:
    QUrl m_url;
    QString m_host;
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QFile *m_file;
    bool m_httpRequestAborted;
    QString m_downDir;
    DownFileType m_currentfiletype;
    QByteArray m_currentDownData;
    QString m_currentJsonfile;
    PatientStudyDB m_patientstudydb;
    HManageThread *m_managethread;
    QObject *m_parent;
    QStringList m_listStudyuid;
};

#endif // HTTPCLIENT_H
