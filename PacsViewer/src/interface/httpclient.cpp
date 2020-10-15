#include "httpclient.h"

#include "ui_authenticationdialog.h"


#include <QNetworkReply>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAuthenticator>

ProgressDialog::ProgressDialog(const QUrl &url, QWidget *parent):QProgressDialog(parent)
{
    setWindowTitle(tr("Download Progress"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setLabelText(tr("Downloading %1.").arg(url.toDisplayString()));
    setMinimum(0);
    setValue(0);
    setMinimumDuration(0);
    setMinimumSize(QSize(400, 75));
}

ProgressDialog::~ProgressDialog()
{

}

void ProgressDialog::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    setMaximum(totalBytes);
    setValue(bytesRead);
}


//-----------------------------------------HttpClient------------------------------------------------------------
HttpClient::HttpClient(QObject *parent, QString dir) : QObject(parent),m_httpRequestAborted(false)
{
    if (dir != "")
    {
        setDwonloadDir(dir);
    }
    else
    {
        m_downDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"\\";
    }
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


    ProgressDialog *progressDialog = new ProgressDialog(m_url, NULL);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(progressDialog, &QProgressDialog::canceled, this, &HttpClient::cancelDownload);
    connect(m_networkreply, &QNetworkReply::downloadProgress, progressDialog, &ProgressDialog::networkReplyProgress);
    connect(m_networkreply, &QNetworkReply::finished, progressDialog, &ProgressDialog::hide);
    progressDialog->show();


}

void HttpClient::downloadFile()
{
    const QString urlSpec = m_url.toString().trimmed();
    if (urlSpec.isEmpty())
    {
        return;
    }

    const QUrl newUrl = QUrl::fromUserInput(urlSpec);
    if (!newUrl.isValid())
    {
        QMessageBox::information(NULL, tr("Error"),
                                 tr("Invalid URL: %1: %2").arg(urlSpec, newUrl.errorString()));
        return;
    }

    QString fileName = newUrl.fileName();
    if (fileName.isEmpty())
    {
        fileName = "temp.tmp";
    }

    QString downloadDirectory = m_downDir;
    bool useDirectory = !downloadDirectory.isEmpty() && QFileInfo(downloadDirectory).isDir();
    if (useDirectory)
        fileName.prepend(downloadDirectory + '/');
    if (QFile::exists(fileName))
    {
        if (QMessageBox::question(NULL, tr("Overwrite Existing File"),
                                  tr("There already exists a file called %1%2."
                                     " Overwrite?")
                                     .arg(fileName,
                                          useDirectory
                                           ? QString()
                                           : QStringLiteral(" in the current directory")),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No)
            == QMessageBox::No)
        {
            return;
        }
        QFile::remove(fileName);
    }
    if (!openFileForWrite(fileName))
    {
        return;
    }

    // schedule the request
    startRequest(newUrl);
}

void HttpClient::cancelDownload()
{
    m_httpRequestAborted = true;
    m_networkreply->abort();
}

void HttpClient::httpFinished()
{
    QFileInfo fileinfo;
    if (m_file)
    {
        fileinfo.setFile(m_file->fileName());
        m_file->close();
        m_file->reset();
    }

    if (m_httpRequestAborted)
    {
        m_networkreply->deleteLater();
        m_networkreply = nullptr;
        return;
    }

    if (m_networkreply->error())
    {
        QFile::remove(fileinfo.absoluteFilePath());
        m_networkreply->deleteLater();
        m_networkreply = nullptr;
        return;
    }

    const QVariant redirectionTarget = m_networkreply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    m_networkreply->deleteLater();
    m_networkreply = nullptr;

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
            return;
        }
        startRequest(redirectedUrl);
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileinfo.absoluteFilePath()));

}

bool HttpClient::openFileForWrite(const QString &fileName)
{
    QFile *file = new QFile(m_downDir+fileName);
    if (!file->open(QIODevice::WriteOnly))
    {
        QMessageBox::information(NULL, tr("Error"),tr("Unable to save the file %1: %2.").arg(
                                     QDir::toNativeSeparators(fileName),file->errorString()));
        return false;
    }
    m_file = file;
    return true;
}

void HttpClient::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (m_file)
    {
        m_file->write(m_networkreply->readAll());
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
