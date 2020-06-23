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

#include "logging.h"

#include "starviewerapplication.h"

#include <QApplication>

INITIALIZE_EASYLOGGINGPP

namespace udg {

    bool isDirExist(QString fullPath)
    {
        QDir dir(fullPath);
        if(dir.exists())
        {
            return true;
        }
        return false;
    }

    bool CreatDir(QString fullPath)
    {
        QDir dir(fullPath);
        if(dir.exists())
        {
            return true;
        }else{
            dir.setPath("");
            bool ok = dir.mkpath(fullPath);
            return ok;
        }
    }
    void beginLogging()
    {
        QString Dir     = QDir::currentPath();
        QString logDir = Dir+"/log";
        if (!isDirExist(logDir))
        {
            CreatDir(logDir);
        }

        el::Configurations defaultConf;
        defaultConf.setToDefault();
        QString logDirFilename = logDir+"/PacsViewer.log";
        defaultConf.set(el::Level::Info,el::ConfigurationType::Filename, logDirFilename.toStdString());
        el::Loggers::reconfigureLogger("default", defaultConf);
    }

    QString getLogFilePath()
    {
        return QDir::toNativeSeparators(udg::UserLogsFile);
    }

    void debugLog(const QString &msg, const QString &file, int line, const QString &function)
    {
        LOG(DEBUG) << qPrintable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
    }

    void infoLog(const QString &msg, const QString&, int, const QString&)
    {
        LOG(INFO) << qUtf8Printable(msg);
    }

    void warnLog(const QString &msg, const QString &file, int line, const QString &function)
    {
        LOG(WARNING) << qUtf8Printable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
    }

    void errorLog(const QString &msg, const QString &file, int line, const QString &function)
    {
        LOG(ERROR) << qUtf8Printable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
    }

    void fatalLog(const QString &msg, const QString &file, int line, const QString &function)
    {
        LOG(FATAL) << qUtf8Printable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
    }

    void verboseLog(int vLevel, const QString &msg, const QString&, int, const QString&)
    {
        VLOG(vLevel) << qUtf8Printable(msg);
    }

    void traceLog(const QString &msg, const QString&, int, const QString&)
    {
        LOG(TRACE) << qUtf8Printable(msg);
    }

}
