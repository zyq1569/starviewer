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

#include "cachetest.h"
#include "inputoutputsettings.h"
#include "systeminformation.h"
#include "starviewerapplication.h"

#include <QFileInfo>
#include <QStringList>

namespace udg {

CacheTest::CacheTest(QObject *parent)
    : DiagnosisTest(parent)
{
}

CacheTest::~CacheTest()
{
}

DiagnosisTestResult CacheTest::run()
{
    DiagnosisTestResult problems;

    Settings settings;
    QString cachePath = settings.getValue(InputOutputSettings::CachePath).toString();
    
    /// Check the free space on the hard disk where the cache is
    unsigned int freeSpace = getFreeSpace(cachePath);
    unsigned int minimumFreeSpace = getMinimumFreeSpace();
    if (freeSpace / 1024.0f < minimumFreeSpace)
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("The free space on the local database location is below the minimum %1 GB required").arg(minimumFreeSpace));
        problem.setSolution(tr("Make some space on disk or change the local database to another location"));
        problems.addError(problem);
    }

    /// Check read and write permissions on the cache folder
    if (!doesCacheDirectoryHaveReadWritePermissions(cachePath))
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("Invalid permissions on the local database directory"));
        problem.setSolution(tr("Fix the directory permissions or change the local database to another location"));
        problems.addError(problem);
    }

    /// For now, in case there is no error, we will look at the warning
    /// Check if the cache is, or not, in the default path
    if (!isCacheOnDefaultPath())
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Warning);
        problem.setDescription(tr("The local database is not on the default path"));
        problem.setSolution("");
        problems.addWarning(problem);
    }
    
    return problems;
}

QString CacheTest::getDescription()
{
    return tr("%1 local database is correctly configured").arg(ApplicationNameString);
}

unsigned int CacheTest::getFreeSpace(const QString &cachePath)
{
    SystemInformation *system = SystemInformation::newInstance();
    unsigned int freeSpace = system->getHardDiskFreeSpace(cachePath.left(2));
    delete system;

    return freeSpace;
}

bool CacheTest::doesCacheDirectoryHaveReadWritePermissions(const QString &cachePath)
{
    QFileInfo file(cachePath);
    return file.isReadable() && file.isWritable();
}

bool CacheTest::isCacheOnDefaultPath()
{
    return true;
}

unsigned int CacheTest::getMinimumFreeSpace()
{
    Settings settings;
    return settings.getValue(InputOutputSettings::MinimumFreeGigaBytesForCache).toUInt();
}

} // end namespace udg
