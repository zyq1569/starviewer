#include "httpclient.h"

#include "ui_authenticationdialog.h"

#include "hthreadobject.h"
#include "hmanagethread.h"

#include <QNetworkReply>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAuthenticator>

///-------Json-------------
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <QProcess>
QProcess *process;
///// -------------------------------------------------------------------------
/// eg..
/// dcm/
///http://127.0.0.1:8080/WADO?
///studyuid=1.2.826.1.1.3680043.2.461.20090916105245.168977.200909160196
///&seriesuid=1.2.840.113619.2.55.3.604688119.969.1252951290.810.4
///&sopinstanceuid=1.2.840.113619.2.55.3.604688119.969.1252951290.968.37
/// -------------------------------------------------------------------------
/// json file
///http://127.0.0.1:8080/WADO?
///studyuid=1.2.826.1.1.3680043.2.461.20090916105245.168977.200909160196&type=json
///-------------------------------------------------------------------------

//-----------------------------------------HttpClient------------------------------------------------------------
bool HttpClient::CreatDir(QString fullPath)
{
    QDir dir(fullPath); // 注意
    if(dir.exists())
    {
        return true;
    }
    else
    {
        dir.setPath("");
        bool ok = dir.mkpath(fullPath);
        return ok;
    }
}

QStringList* HttpClient::getListStudyUID()
{
    return &m_listStudyuid;
}

QString HttpClient::getHttpServerHost()
{
    return m_host;
}

void HttpClient::setHttpServerHost(QString host)
{
    m_host = host;
}

const HManageThread * HttpClient::getManageThread()
{
    return  m_managethread;
}

HttpClient::HttpClient(QObject *parent, QString dir) : QObject(parent),m_httpRequestAborted(false)
{
    m_parent = parent;
    m_file = nullptr;
    if (dir != "")
    {
        setDwonloadDir(dir);
    }
    else
    {
        m_downDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"\\";
    }
    m_currentfiletype = DownFileType::other;
    m_managethread = nullptr;
}

void HttpClient::setDwonloadDir(QString dir)
{
    m_downDir = dir;
    if (m_downDir.isEmpty() || !QFileInfo(m_downDir).isDir())
    {
        m_downDir = QDir::currentPath();
    }
}
void HttpClient::startRequest(const QUrl &requestedUrl)
{
    m_url = requestedUrl;
    m_httpRequestAborted = false;

    m_networkreply = m_networkmanager.get(QNetworkRequest(m_url));
    connect(m_networkreply, &QIODevice::readyRead, this, &HttpClient::httpReadyRead);
    connect(m_networkreply, &QNetworkReply::finished, this, &HttpClient::httpFinished);

}


void HttpClient::ParseDwonData()
{
    m_patientstudydb.count = 0;
    m_patientstudydb.rowinfo.clear();
    if (m_currentfiletype == DownFileType::studyini && m_currentDownData.size() > 1)
    {
        TRACE_LOG("---step 1/3---: start parse jsonfile : "+m_url.query());
        HStudy study;
        QJsonParseError jsonError;
        QJsonDocument paserDoc = QJsonDocument::fromJson(m_currentDownData, &jsonError);
        if (jsonError.error != QJsonParseError::NoError)
        {
            INFO_LOG("---second Parse Json--- studyuid: " + m_currentJsonfile);
            QFile file(m_currentJsonfile);
            file.open(QIODevice::ReadOnly);
            QByteArray data = file.readAll();
            file.close();
            paserDoc = QJsonDocument::fromJson(data, &jsonError);
        }
        if (jsonError.error == QJsonParseError::NoError)
        {
            QJsonObject paserObj = paserDoc.object();
            study.StudyUID = paserObj.take("studyuid").toString();
            study.imageCount = paserObj.take("numImages").toInt();
            QJsonArray array = paserObj.take("seriesList").toArray();
            CreatDir(m_downDir+"/"+study.StudyUID);
            TRACE_LOG("------step 2/3---:  parse dcm studyuid:: " + study.StudyUID);
            m_listStudyuid.clear();
            QList<HttpInfo> httpinfo;
            int size = array.size();
            for (int i=0; i<size; i++)
            {
                HSeries series;
                QJsonObject Obj = array.at(i).toObject();
                series.SeriesUID = Obj.take("seriesUid").toString();
                QJsonArray iarray = Obj.take("instanceList").toArray();
                CreatDir(m_downDir+"/"+study.StudyUID+"/"+series.SeriesUID);
                int isize = iarray.size();
                for (int j=0; j<isize; j++)
                {
                    QString imageuid = iarray.at(j).toObject().take("imageId").toString();
                    series.ImageSOPUI.push_back(imageuid);
                    QString newurl = m_host+"/WADO?studyuid="+study.StudyUID+"&seriesuid="+series.SeriesUID+"&sopinstanceuid="+imageuid;
                    HttpInfo info;
                    info.url = QUrl(newurl);
                    info.fullpathfilename = m_downDir + "/"+study.StudyUID+"/"+series.SeriesUID+"/"+imageuid+".dcm";
                    httpinfo.push_back(info);
                    m_listStudyuid.push_back(info.fullpathfilename);
                }
                study.Serieslist.push_back(series);
            }
            TRACE_LOG(tr("---step 3/3---:  start to down all dcm files : series size =  %1   | image filse : count = %2 ").arg(size).arg(httpinfo.size()));
            if (!m_managethread)
            {
                m_managethread = new HManageThread();
                connect(m_managethread, SIGNAL(allFinished()), this,  SLOT(allFilesThreadFinished()));
                //allFilesThreadFinished
            }
            m_managethread->start(httpinfo);
        }
        else
        {
            ERROR_LOG(tr("---error---->parse json fail: %1 %2").arg(m_url.query()).arg(jsonError.errorString()));
            QMessageBox::question(NULL, tr("Down error"),tr("parse json fail: %1 %2?").arg(m_url.query(),jsonError.errorString()), QMessageBox::Ok);
        }
    }
}

void HttpClient::downFileFromWeb(QUrl httpUrl, QString savefilename, QString downDir)
{
    QString fileName = savefilename;
    QString downloadDirectory = downDir;
    bool useDirectory = !downloadDirectory.isEmpty() && QFileInfo(downloadDirectory).isDir();
    if (useDirectory)
    {
        fileName.prepend(downloadDirectory + '/');
    }
    if (QFile::exists(fileName))
    {
//        if (QMessageBox::question(NULL, tr("Overwrite Existing File"),
//                                  tr("There already exists a file called %1%2."
//                                     " Overwrite?").arg(fileName,
//                                                        useDirectory ? QString() : QStringLiteral(" in the current directory")),
//                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
//        {
//            return;
//        }
        QFile::remove(fileName);
    }
    if (m_currentfiletype != DownFileType::dbinfo)
    {
        if (m_currentfiletype == DownFileType::studyini)
        {
            m_currentJsonfile = fileName;
        }
        if (!openFileForWrite(fileName))
        {
            return;
        }
    }

    // schedule the request
    startRequest(httpUrl);
}

void HttpClient::getStudyImageFile(QUrl url,QString studyuid,QString seruid, QString imguid)
{
    if ( studyuid == "")
    {
        return;
    }
    if (url.toString() == "")
    {
        url = QUrl(getHttpServerHost());
    }
    m_currentfiletype = DownFileType::other;

    if (seruid != "" && imguid != "")
    {
        QString strURL = url.toString()+"/WADO?studyuid="+studyuid+"&seriesuid="+seruid+"&sopinstanceuid="+imguid;
        url = strURL;
        m_currentfiletype = DownFileType::dcm;
    }
    else
    {
        QString strURL = url.toString()+"/WADO?studyuid="+studyuid+"&type=json";
        url = QUrl(strURL);
        m_currentfiletype = DownFileType::studyini;
    }

    m_url = url;
    const QString urlSpec = m_url.toString().trimmed();
    if (urlSpec.isEmpty())
    {
        return;
    }

    const QUrl newUrl = QUrl::fromUserInput(urlSpec);
    if (!newUrl.isValid())
    {
        QMessageBox::information(NULL, tr("Error"),tr("Invalid URL: %1: %2").arg(urlSpec, newUrl.errorString()));
        return;
    }

    QString fileName = newUrl.fileName();
    switch (m_currentfiletype)
    {
    case DownFileType::dcm :
        fileName = imguid+".dcm";
        break;
    case DownFileType::studyini:
        fileName = studyuid+".json";
        break;
    default:
        break;
    }
    if (fileName.isEmpty())
    {
        fileName = "temp.tmp";
    }

    downFileFromWeb(newUrl,fileName,m_downDir);
}


void HttpClient::allFilesThreadFinished()
{
    emit allFilesFinished();
}

void HttpClient::cancelDownload()
{
    m_httpRequestAborted = true;
    m_networkreply->abort();
}

void HttpClient::httpFinished()
{
    QFileInfo fileinfo;
    if (m_file )
    {
        if (m_file->isOpen())
        {
            fileinfo.setFile(m_file->fileName());
            m_file->close();
            delete  m_file;
            m_file = NULL;
        }
    }

    if (m_httpRequestAborted)
    {
        m_networkreply->deleteLater();
        m_networkreply = NULL;
        return;
    }

    if (m_networkreply->error())
    {
        QFile::remove(fileinfo.absoluteFilePath());
        m_networkreply->deleteLater();
        m_networkreply = NULL;
        return;
    }

    const QVariant redirectionTarget = m_networkreply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    m_networkreply->deleteLater();
    m_networkreply = NULL;

    if (!redirectionTarget.isNull())
    {
        const QUrl redirectedUrl = m_url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(NULL, tr("Redirect"),tr("Redirect to %1 ?").arg(redirectedUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            QFile::remove(fileinfo.absoluteFilePath());
            return;
        }
        if (!openFileForWrite(fileinfo.absoluteFilePath()))
        {
            QFile::remove(fileinfo.absoluteFilePath());
            return;
        }
        startRequest(redirectedUrl);
        return;
    }
    ParseDwonData();
}

bool HttpClient::openFileForWrite(const QString &fileName)
{
    if (m_file)
    {
        delete m_file;
        m_file = NULL;
    }
    m_file = new QFile(fileName);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::information(NULL, tr("Error"),tr("Unable to save the file %1: %2.").arg(
                                     QDir::toNativeSeparators(fileName),m_file->errorString()));
        m_file->close();
        delete m_file;
        m_file = NULL;
        return false;
    }
    return true;
}

void HttpClient::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    m_currentDownData.clear();
    if (m_currentfiletype == DownFileType::dbinfo)
    {
        m_currentDownData = m_networkreply->readAll();
    }
    else
    {
        if (m_currentfiletype == DownFileType::studyini)
        {
            m_currentDownData = m_networkreply->readAll();
            if (m_file)
            {
                m_file->write(m_currentDownData);
            }
        }
        else if (m_file)
        {
            m_file->write(m_networkreply->readAll());
        }
    }
}

void HttpClient::slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator)
{
    QDialog authenticationDialog;
    Ui::Dialog ui;
    ui.setupUi(&authenticationDialog);
    authenticationDialog.adjustSize();
    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm(), m_url.host()));

    // Did the URL have information? Fill the UI
    // This is only relevant if the URL-supplied credentials were wrong
    ui.userEdit->setText(m_url.userName());
    ui.passwordEdit->setText(m_url.password());

    if (authenticationDialog.exec() == QDialog::Accepted)
    {
        authenticator->setUser(ui.userEdit->text());
        authenticator->setPassword(ui.passwordEdit->text());
    }
}

#ifndef QT_NO_SSL
void HttpClient::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors)
    {
        if (!errorString.isEmpty())
        {
            errorString += '\n';
        }
        errorString += error.errorString();
    }

    if (QMessageBox::warning(NULL, tr("SSL Errors"),
                             tr("One or more SSL errors has occurred:\n%1").arg(errorString),
                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore)
    {
        m_networkreply->ignoreSslErrors();
    }
}
#endif
