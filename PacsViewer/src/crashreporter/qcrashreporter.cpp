/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#include <QDateTime>

#include "qcrashreporter.h"
#include "crashreportersender.h"
#include "../core/starviewerapplication.h"
#include "logging.h"
#include "executablesnames.h"
#include <QMovie>
#include <QNetworkInterface>
#include <QProcess>

namespace udg {

QCrashReporter::QCrashReporter(const QStringList &args, QWidget *parent)
    : QWidget(parent)
{
    setWindowIcon(QIcon(":/images/starviewer.png"));

    setupUi(this);

    setWindowTitle(tr("%1").arg(ApplicationNameString));
    m_quitPushButton->setText(tr("Quit %1").arg(ApplicationNameString));
    m_restartPushButton->setText(tr("Restart %1").arg(ApplicationNameString));
    m_sendReportCheckBox->setText(tr("Tell %1 about this crash").arg(ApplicationNameString));
    m_informationLabel->setText(tr("We're sorry %1 had a problem and crashed. Please take a moment to send us a crash report to help us diagnose and fix "
                                   "the problem. Your personal information is not sent with this report.  ").arg(ApplicationNameString));

    m_minidumpPath = args[1] + "/" + args[2] + ".dmp";

    QMovie *sendReportAnimation = new QMovie(this);
    sendReportAnimation->setFileName(":/images/loader.gif");
    m_sendReportAnimation->setMovie(sendReportAnimation);
    sendReportAnimation->start();

    m_sendReportLabel->hide();
    m_sendReportAnimation->hide();

    // Busquem les adreces IP del host.
    QString ipAddresses("");

    Q_FOREACH (QNetworkInterface inter, QNetworkInterface::allInterfaces())
    {
        if (inter.flags().testFlag(QNetworkInterface::IsRunning))
        {
            Q_FOREACH (QNetworkAddressEntry entry, inter.addressEntries())
            {
                if (inter.hardwareAddress() != "00:00:00:00:00:00" && entry.ip().toString().contains(".") && entry.ip().toString() != "127.0.0.1")
                {
                    if (!ipAddresses.isEmpty())
                    {
                        ipAddresses += ", ";
                    }

                    ipAddresses += entry.ip().toString();
                }
            }
        }
    }

    QString hostInformation;
    hostInformation = QString("%1: %2").arg(tr("IP")).arg(ipAddresses);

    // En cas que estem a windows, afegim com a info del host el domini de l'usuari.
#ifdef WIN32
    QString userDomain = QProcessEnvironment::systemEnvironment().value(QString("USERDOMAIN"), QString(""));
    if (!userDomain.isEmpty())
    {
        hostInformation += QString("\n%1: %2").arg(tr("User Domain")).arg(userDomain);
    }
#endif

    m_hostInformationTextEdit->setPlainText(hostInformation);

    connect(m_quitPushButton, SIGNAL(clicked()), this, SLOT(quitButtonClickedSlot()));
    connect(m_restartPushButton, SIGNAL(clicked()), this, SLOT(restartButtonClickedSlot()));
}

void QCrashReporter::quitButtonClickedSlot()
{
    maybeSendReport();
    close();
}

void QCrashReporter::restartButtonClickedSlot()
{
    maybeSendReport();

    QString starviewerPath = QCoreApplication::applicationDirPath() + "/" + STARVIEWER_EXE;
#ifdef WIN32
    // En windows per poder executar l'starviewer hem de tenir en compte que si està en algun directori que conte espais
    // com el directori C:\Program Files\Starviewer\starviewer.exe, hem de posar el path entre cometes
    // per a que no ho interpreti com a paràmetres, per exemple "C:\Program Files\Starviewer\starviewer.exe"

    // Afegim les cometes per si algun dels directori conté espai
     starviewerPath = "\"" + starviewerPath + "\"";
#endif

    restart(starviewerPath);

    close();
}

void QCrashReporter::maybeSendReport()
{
    if (m_sendReportCheckBox->isChecked())
    {
        m_sendReportAnimation->show();
        m_sendReportLabel->show();
        qApp->processEvents();
        sendReport();
    }
}

void QCrashReporter::sendReport()
{
    QHash<QString, QString> options;
    options.insert("BuildID", StarviewerBuildID);
    options.insert("ProductName", ApplicationNameString);
    options.insert("Version", StarviewerVersionString);
    options.insert("Email", m_emailLineEdit->text());

    QString comments(m_descriptionTextEdit->toPlainText());
    if (m_hostInformationCheckBox->isChecked())
    {
        comments += QString("\n// %1:\n").arg(tr("Host information"));
        comments += m_hostInformationTextEdit->toPlainText();
    }

    options.insert("Comments", comments);
    options.insert("CrashTime", QByteArray::number(QDateTime::currentDateTime().toTime_t()));
    // El valor 1 significa que es tindran en compte les condicions d'acceptacio programades al fitxer
    // de configuracio del "collector".
    // El valor 0 significa que s'acceptaran tots els reports. Les condicions d'acceptacio no es tenen en compte.
    options.insert("Throttleable", "1");

    // Enviem el report només en cas de release.
#ifdef QT_NO_DEBUG
    bool success = CrashReporterSender::sendReport("http://starviewer.udg.edu/crashreporter/submit", m_minidumpPath, options);

    if (success)
    {
        INFO_LOG(QString("Crash report enviat: %1").arg(m_minidumpPath));
    }
    else
    {
        ERROR_LOG(QString("Error al enviar el crash report: %1").arg(m_minidumpPath));
    }
#endif

    m_sendReportAnimation->hide();
    m_sendReportLabel->hide();
}

bool QCrashReporter::restart(const QString &path)
{
    QProcess process;
    process.startDetached(path);

    return true;
}

};
