﻿/*************************************************************************************
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

#ifndef UDG_STARVIEWER_APPLICATION
#define UDG_STARVIEWER_APPLICATION

#include <QDir>

namespace udg {

const QString StarviewerVersionString("0.15.0-devel");
const QString StarviewerBuildID("20240801");

#ifdef Q_OS_WIN
const QString StarviewerBuildPlatform("Windows");
#endif
#ifdef Q_OS_OSX
const QString StarviewerBuildPlatform("Mac");
#endif
#ifdef Q_OS_LINUX
const QString StarviewerBuildPlatform("Linux");
#endif

// Iindicates for this version of starviewer which bd revision is required
const int StarviewerDatabaseRevisionRequired(9593);

const QString OrganizationNameString("GILab");
const QString OrganizationDomainString("starviewer.udg.edu");

#ifdef STARVIEWER_LITE
const QString ApplicationNameString("Starviewer Lite");
#else
const QString ApplicationNameString("Starviewer");
#endif

const QString OrganizationEmailString("support@starviewer.udg.edu");
const QString OrganizationWebURL("http://starviewer.udg.edu");

// Default pathways where user configurations and files are saved
/// Path base for user data
const QString UserDataRootPath(QDir::homePath() + "/.starviewer/");
/// People with path
const QString UserLogsPath(UserDataRootPath + "log/");

/// Absolute log file path
const QString UserLogsFile(UserLogsPath + "starviewer.log");

// Note: the following two are methods because they depend on QApplication being initialized.

/// Returns the base directory where the application is installed.
QString installationPath();
/// Returns the root source directory (the one containing src).
QString sourcePath();

//new log path
const QString UserCurrentAppPath(installationPath() + "/log/");
/// new Absolute log file path
const QString UserCurrentAppPathLogsFile(UserCurrentAppPath + "starviewer.log");
}

#endif
