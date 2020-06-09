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

#ifndef _LOGGING_
#define _LOGGING_

#include <QString>
#include <QtGlobal> // Pel qpuntenv()
/*!
    Aquest arxiu conté totes les macros per a fer logs en l'aplicació.
*/

// Include log4cxx header files.
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

/// Macro to initialize loggers
/// We define the environment variable that indicates the location
/// of the log files and then we read the configuration of the logs
#define LOGGER_INIT(file) \
    if (true) \
    { \
        QByteArray logFilePathValue = (QDir::toNativeSeparators(udg::UserLogsFile)).toLatin1(); \
        qputenv("logFilePath", logFilePathValue); \
        log4cxx::PropertyConfigurator::configure(file); \
    } else (void)0

/// Macro for debug messages. \
/// TODO at the moment we use this variable from qmake and it works fine, but it might be more appropriate to find a way to add
/// a variable of its own, such as DEBUG
#ifdef QT_NO_DEBUG
#define DEBUG_LOG(msg) (void)0
#else
#define DEBUG_LOG(msg) \
    if (true) \
    { \
        LOG4CXX_DEBUG(log4cxx::Logger::getLogger("development"), qPrintable(QString(msg))) \
    } else (void)0

#endif

/// Macro for general information messages
#define INFO_LOG(msg) \
    if (true) \
    { \
        LOG4CXX_INFO(log4cxx::Logger::getLogger("info.release"), QString(msg).toUtf8().constData()) \
    } else (void)0

/// Macro for warning messages
#define WARN_LOG(msg) \
    if (true) \
    { \
        LOG4CXX_WARN(log4cxx::Logger::getLogger("info.release"), QString(msg).toUtf8().constData()) \
    } else (void)0

/// Macro for error messages
#define ERROR_LOG(msg) \
    if (true) \
    { \
        LOG4CXX_ERROR(log4cxx::Logger::getLogger("errors.release"), QString(msg).toUtf8().constData()) \
    } else (void)0

/// Macro for fatal / critical error messages
#define FATAL_LOG(msg) \
    if (true) \
    { \
        LOG4CXX_FATAL(log4cxx::Logger::getLogger("errors.release"), QString(msg).toUtf8().constData()) \
    } else (void)0

#endif
