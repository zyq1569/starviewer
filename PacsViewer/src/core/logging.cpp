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
#include "easylogging++.h"
#include "starviewerapplication.h"

#include <QApplication>

namespace udg
{

const char* getPID(const el::LogMessage*)
{
#ifdef Q_OS_WIN
    static std::string stdpid = QString::number(qApp->applicationPid()).toStdString();
#else
    static std::string stdpid = QString::number(getpid()).toStdString();
#endif
    return  stdpid.c_str();
}

void setLogDefault()
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    el::Helpers::installCustomFormatSpecifier(el::CustomFormatSpecifier("%pid", getPID));
    // Values are always std::string
    //defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime{%Y-%M-%d %H:%m:%s} %level %msg");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Format, "%datetime{%Y-%M-%d %H:%m:%s:%g} %pid %thread %levshort %msg");
    // default logger uses default configurations
    //el::Loggers::reconfigureLogger("default", defaultConf);
    //LOG(INFO) << "Log using default file";
    // To set GLOBAL configurations you may use
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime{%Y-%M-%d %H:%m:%s:%g} %pid %thread %levshort %msg");
	defaultConf.setGlobally(el::ConfigurationType::Filename, qPrintable(getLogFilePath()));
    el::Loggers::reconfigureLogger("default", defaultConf);
	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
	el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
	el::Loggers::setLoggingLevel(el::Level::Error);

    INFO_LOG("--------setLogDefault()-----------");
}

void beginLogging()
{
    //First we check that the directory ~ / .starviewer / log / exists where we will look for the logs
    //QDir logDir = udg::UserLogsPath;
    //QDir logDir = udg::UserCurrentAppPath;//use current work path
    //if (!logDir.exists())
    //{
    //    // Creem el directori
    //    logDir.mkpath(udg::UserLogsPath);
    //    //logDir.mkpath(udg::UserCurrentAppPath);
    //}
    QDir logConf = getLogConfFilePath();
    if (logConf.exists())
    {
        el::Configurations logConfig(getLogConfFilePath().toStdString());
        //logConfig.setGlobally(el::ConfigurationType::Filename, getLogFilePath().toStdString());
		logConfig.setGlobally(el::ConfigurationType::Filename, qPrintable(getLogFilePath()));
        //Disable logging to the standard output when compiled on release
#ifdef QT_NO_DEBUG
        logConfig.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
#endif
        el::Loggers::reconfigureAllLoggers(logConfig);
    }
    else
    {
        setLogDefault();
    }
}

QString getLogFilePath()
{
    //return QDir::toNativeSeparators(udg::UserLogsFile);
    //return QDir::toNativeSeparators(udg::UserCurrentAppPathLogsFile);
    return QDir::toNativeSeparators(installationPath()+"/log/starviewer.log");
}

QString getLogConfFilePath()
{
    // TODO we assume that the file is so called and is in the location we indicate.
    // It should be made a little more flexible or generic;

    // it's like that because at the moment we want to go to work
    // and not entertain ourselves but it has to be done well.

    QString configurationFile;

    if (qApp->applicationFilePath().contains("autotests"))
    {
        configurationFile = sourcePath() + "/tests/auto/log.conf";
    }
    else
    {
        configurationFile = "/etc/starviewer/log.conf";

        if (!QFile::exists(configurationFile))
        {
            configurationFile = installationPath() + "/log.conf";
        }
        if (!QFile::exists(configurationFile))
        {
            configurationFile = sourcePath() + "/bin/log.conf";
        }
    }

    return configurationFile;
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
