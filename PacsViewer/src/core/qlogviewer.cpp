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

#include "qlogviewer.h"

#include "coresettings.h"
#include "logging.h"
#include "starviewerapplication.h"

// Qt
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QTextCodec>
#include <QFileDialog>

namespace udg {

QLogViewer::QLogViewer(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    readSettings();
    createConnections();
}

QLogViewer::~QLogViewer()
{
    writeSettings();
}

void QLogViewer::updateData()
{
    // \TODO this directory should be saved in some sort of settings or similar
    //QFile logFile(udg::UserLogsFile);
    QString logfullpath = installationPath()+"/log/starviewer.log";
    QFile logFile(logfullpath);// read current work dir logsfile
    if (!logFile.open(QFile::ReadOnly | QFile::Text))
    {
        ERROR_LOG("Could not open log file");
        m_logBrowser->setPlainText(tr("ERROR: No Log file found at this path: %1\nEnvironment variable (logFilePath): %2").arg(
                                       logfullpath).arg(
                                       QString::fromLocal8Bit(qgetenv("logFilePath"))));
    }
    else
    {
        INFO_LOG("The log file was successfully opened [" + logfullpath + "]");
        m_logBrowser->setPlainText(logFile.readAll());
        m_logBrowser->moveCursor(QTextCursor::End);
        //        INFO_LOG("The log file was successfully opened [" + udg::getLogFilePath() + "]");
        //        m_logBrowser->setPlainText(logFile.readAll());
        //        m_logBrowser->moveCursor(QTextCursor::End);
    }
}

void QLogViewer::createConnections()
{
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveLogFileAs()));
}

void QLogViewer::saveLogFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As..."), QString(), tr("Log Files (*.log)"));

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
    {
        return;
    }

    QTextStream logStream(&file);
    logStream << m_logBrowser->document()->toPlainText();
}

void QLogViewer::writeSettings()
{
    Settings settings;
    settings.saveGeometry(CoreSettings::LogViewerGeometry, this);
}

void QLogViewer::readSettings()
{
    Settings settings;
    settings.restoreGeometry(CoreSettings::LogViewerGeometry, this);
}

}
