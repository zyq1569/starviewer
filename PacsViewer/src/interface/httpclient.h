#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QProgressDialog>
#include <QUrl>
#include <QSslError>
#include <QNetworkAccessManager>


class QNetworkReply;
class QAuthenticator;
class QFile;

//class QNetworkAccessManager;

class ProgressDialog : public QProgressDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(const QUrl &url, QWidget *parent = nullptr);
    ~ProgressDialog();

public slots:
   void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);
};


class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr, QString dir = "");

public:
    void startRequest(const QUrl &requestedUrl);
    void setDwonloadDir(QString dir);
signals:

private slots:
    void downloadFile();
    void cancelDownload();
    void httpFinished();
    void httpReadyRead();
    void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

private:
    bool openFileForWrite(const QString &fileName);
private:
    QUrl m_url;
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QFile *m_file;
    bool m_httpRequestAborted;
    QString m_downDir;


};

#endif // HTTPCLIENT_H
