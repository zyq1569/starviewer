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

// Starviewer
#include "systemrequerimentstest.h"
#include "systemrequeriments.h"
#include "starviewerapplication.h"

// Qt
#include <QString>
#include <QList>
#include <QStringList>
#include <QSize>

namespace udg {

SystemRequerimentsTest::SystemRequerimentsTest(QObject *parent)
 : DiagnosisTest(parent)
{
}

SystemRequerimentsTest::~SystemRequerimentsTest()
{
}

DiagnosisTestResult SystemRequerimentsTest::run()
{
    /// Requeriments mínims
    SystemRequeriments *requeriments = getSystemRequeriments();
    const unsigned int MinimumNumberOfCores = requeriments->getMinimumCPUNumberOfCores();
    const unsigned int MinimumCoreSpeed = requeriments->getMinimumCPUFrequency();
    const unsigned int MinimumGPURAM = requeriments->getMinimumGPURAM();
    const QStringList MinimumGPUOpenGLExtensions = requeriments->getMinimumGPUOpenGLCompatibilities();
    const QString MinimumGPUOpenGLVersion = requeriments->getMinimumGPUOpenGLVersion();
    const QString MinimumOSVersion = requeriments->getMinimumOperatingSystemVersion();
    const unsigned int MinimumRAM = requeriments->getMinimumRAMTotalAmount();
    const unsigned int MinimumScreenWidth = requeriments->getMinimumScreenWidth();
    const unsigned int MinimumDiskSpace = requeriments->getMinimumHardDiskFreeSpace();

    // TODO Temporal, s'ha de treure i veure com obtenir la unitat on està starviewer
    const QString whichHardDisk = "C:";

    SystemInformation *system = SystemInformation::newInstance();
    DiagnosisTestResult result;
    
    /// Requeriments mínims del sistema:
    // Per exemple: Dual core 1.5Ghz
    unsigned int numberOfCores = getCPUNumberOfCores(system);
    if (numberOfCores < MinimumNumberOfCores)
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("The computer has %1 cores, and the minimum required is %2").arg(numberOfCores).arg(MinimumNumberOfCores));
        problem.setSolution(tr("Update computer's hardware"));
        result.addError(problem);
    }
    else
    {
        // Una màquina pot tenir més d'una CPU.
        // Buscar si alguna té una velocitat superior al mínim
        QList<unsigned int> cpuFrequencies = getCPUFrequencies(system);
        unsigned int maximumCPUFrequency = 0;
        for (int i = 0; i < cpuFrequencies.count(); i++)
        {
            if (cpuFrequencies.at(i) > maximumCPUFrequency)
            {
                maximumCPUFrequency = cpuFrequencies.at(i);
            }
        }
        
        if (maximumCPUFrequency < MinimumCoreSpeed)
        {
            DiagnosisTestProblem problem;
            problem.setState(DiagnosisTestProblem::Error);
            problem.setDescription(tr("The fastest CPU runs at %1 MHz, and the minimum required is %2 MHz").arg(maximumCPUFrequency).arg(MinimumCoreSpeed));
            problem.setSolution(tr("Update computer's hardware"));
            result.addError(problem);
        }
    }

    // Comprovar si la versió d'openGL del sistema és suficient
    QString openGLVersion = getGPUOpenGLVersion(system);
    if (compareVersions(openGLVersion, MinimumGPUOpenGLVersion) == SystemRequerimentsTest::Older)
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("Current OpenGL version is %1, and the minimum required is %2").arg(openGLVersion).arg(MinimumGPUOpenGLVersion));
        // Normalment la versió d'openGL s'actualitza amb els drivers de la gràfica
        problem.setSolution(tr("Update your graphics card driver"));
        result.addError(problem);
    }

    // Tenir en una llista les compatibilitats openGL que starviewer utilitza i anar-les buscant una a una al retorn del mètode
    QStringList openGLExtensions = getGPUOpenGLCompatibilities(system);
    QStringList unsupportedOpenGLExtensions;
    for (int i = 0; i < MinimumGPUOpenGLExtensions.count(); i++)
    {
        if (!openGLExtensions.contains(MinimumGPUOpenGLExtensions.at(i)))
        {
            unsupportedOpenGLExtensions << MinimumGPUOpenGLExtensions.at(i);
        }
    }
    if (!unsupportedOpenGLExtensions.empty())
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("Current OpenGL version does not support these extensions: %1").arg(unsupportedOpenGLExtensions.join(", ")));
        problem.setSolution(tr("Update your graphics card driver"));
        result.addError(problem);
    }

    // Memòria RAM de la GPU
    QList<unsigned int> gpuRAM = getGPURAM(system);
    QStringList gpuModel = getGPUModel(system);
    for (int i = 0; i < gpuRAM.count(); i++)
    {
        if (gpuRAM.at(i) < MinimumGPURAM)
        {
            DiagnosisTestProblem problem;
            problem.setState(DiagnosisTestProblem::Error);
            problem.setDescription(tr("The graphics card %1 has %2 MB of RAM, and the minimum required is %3 MB").arg(gpuModel.at(i)).arg(gpuRAM.at(i)).arg(MinimumGPURAM));
            problem.setSolution(tr("Change the graphics card"));
            result.addError(problem);
        }
    }
    
    // TODO Disc dur. S'ha de fer també del que conté el directori de la cache????????
    if (getHardDiskFreeSpace(system, whichHardDisk) < MinimumDiskSpace)
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("There is not enough disk space to run %1 properly").arg(ApplicationNameString));
        problem.setSolution(tr("Free some space in the hard disk"));
        result.addError(problem);
    }

    // Arquitectura de la màquina (32 o 64 bits)
    if (requeriments->doesOperatingSystemNeedToBe64BitArchitecutre() && !isOperatingSystem64BitArchitecture(system))
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("Operating system is not 64-bit"));
        problem.setSolution(tr("Update operating system to a 64-bit version"));
        result.addError(problem);
    }

    // Versió del sistema operatiu
    QString version;
    QString servicePack;
    switch (getOperatingSystem(system))
    {
        case SystemInformation::OSWindows:
            // Si el SO és windows, quina ha de ser la mínima versió??
            version = getOperatingSystemVersion(system);
            if (compareVersions(version, MinimumOSVersion) == SystemRequerimentsTest::Older)
            {
                DiagnosisTestProblem problem;
                problem.setState(DiagnosisTestProblem::Error);
                problem.setDescription(tr("Current operating system version is %1, and the minimum required is %2").arg(version).arg(MinimumOSVersion));
                problem.setSolution(tr("Update operating system to a newer version"));
                result.addError(problem);
            }
            // Si és windows XP (versió 5.xx), s'ha de comprovar el service pack
            if (version.split(".").at(0).toInt() == 5)
            {
                servicePack = getOperatingSystemServicePackVersion(system);
                unsigned int minimumServicePackVersion;
                if (isOperatingSystem64BitArchitecture(system))
                {
                    minimumServicePackVersion = requeriments->getMinimum64bitOperatingSystemServicePackVersion();
                }
                else
                {
                    minimumServicePackVersion = requeriments->getMinimum32bitOperatingSystemServicePackVersion();
                }
                
                if (servicePack.right(servicePack.count() - 13).toUInt() < minimumServicePackVersion)
                {
                    DiagnosisTestProblem problem;
                    problem.setState(DiagnosisTestProblem::Error);
                    problem.setDescription(tr("Current Service Pack version is %1, and the minimum required is Service Pack %2").arg(servicePack).arg(minimumServicePackVersion));
                    problem.setSolution(tr("Install a newer Service Pack"));
                    result.addError(problem);
                }
            }
            break;
        case SystemInformation::OSLinux:
            version = getOperatingSystemVersion(system);
            break;
        case SystemInformation::OSMac:
            version = getOperatingSystemVersion(system);
            break;
        default:
            break;
    }

    // Memòria RAM
    unsigned int RAMTotalAmount = getRAMTotalAmount(system);
    if (RAMTotalAmount < MinimumRAM)
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setDescription(tr("The total amount of RAM memory is %1 MB, and the minimum required is %2 MB").arg(RAMTotalAmount).arg(MinimumRAM));
        problem.setSolution(tr("Upgrade computer's RAM memory"));
        result.addError(problem);
    }

    // Si alguna de les pantalles és menor de 1185 pixels d'amplada, poder retornar un warning, ja que starviewer no hi cap.
    QList<QSize> resolutions = getScreenResolutions(system);
    QStringList screensInWhichStarviewerWontFit;
    bool starviewerWillFitInOneScreen = false;
    for (int i = 0; i < resolutions.count(); i++)
    {
        if (resolutions.at(i).width() < (int)MinimumScreenWidth)
        {
            // i + 1, ja que les pantalles a la interfície es mostren de 1 a n
            screensInWhichStarviewerWontFit << QString::number(i + 1);
        }
        else
        {
            starviewerWillFitInOneScreen = true;
        }
    }

    if (starviewerWillFitInOneScreen)
    {
        if (screensInWhichStarviewerWontFit.count() > 0)
        {
            // Warning
            DiagnosisTestProblem problem;
            problem.setState(DiagnosisTestProblem::Warning);
            problem.setDescription(tr("One of the screens is too small. Keep in mind that %1 won't fit in that screen").arg(ApplicationNameString));
            problem.setSolution(tr("Avoid placing %1 on the screen or screens %2, or increase their resolution if possible").arg(ApplicationNameString)
                .arg(screensInWhichStarviewerWontFit.join(", ")));
            result.addWarning(problem);
        }
        // else OK
    }
    else
    {
        // ERROR
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Error);
        problem.setSolution(tr("Increase screen resolution"));
        if (resolutions.count() == 1)
        {
            problem.setDescription(tr("The screen is too small to fit %1 application").arg(ApplicationNameString));
        }
        else
        {
            problem.setDescription(tr("The screens are too small to fit %1 application").arg(ApplicationNameString));
        }
        result.addError(problem);
    }

    // Que la unitat de CD/DVD no pugui grabar
    if (requeriments->doesOpticalDriveNeedWriteCapabilities() && !doesOpticalDriveHaveWriteCapabilities(system))
    {
        DiagnosisTestProblem problem;
        problem.setState(DiagnosisTestProblem::Warning);
        problem.setDescription(tr("The optical drive does not have burning capabilities"));
        problem.setSolution(tr("Change the optical drive to a CD-RW/DVD-RW"));
        result.addWarning(problem);
    }

    delete system;
    return result;
}

QString SystemRequerimentsTest::getDescription()
{
    return tr("Hardware meets %1's minimum system requeriments").arg(ApplicationNameString);
}

SystemRequerimentsTest::VersionComparison SystemRequerimentsTest::compareVersions(QString version1, QString version2)
{
    // TODO de moment només estar pensat per windows. S'ha d'estendre a més S.O.
    
    //5.1.2600 XXX.XXX
    QStringList version1List = (version1.split(" ")[0]).split(".");
    QStringList version2List = (version2.split(" ")[0]).split(".");

    int index = 0;
    // Mentre les parts siguin iguals, anem comparant
    while (index < version1List.count() && index < version2List.count())
    {
        if (version1List.at(index).toInt() < version2List.at(index).toInt())
        {
            return SystemRequerimentsTest::Older;
        }
        else if (version1List.at(index).toInt() > version2List.at(index).toInt())
        {
            return SystemRequerimentsTest::Newer;
        }
        index++;
    }

    // Si totes les parts són iguals, la versió que en tingui més serà la major, ja que suposarem que la altra és .0
    if (version1List.count() < version2List.count())
    {
        return SystemRequerimentsTest::Older;
    }
    else if (version1List.count() > version2List.count())
    {
        return SystemRequerimentsTest::Newer;
    }

    return SystemRequerimentsTest::Same;
}

unsigned int SystemRequerimentsTest::getCPUNumberOfCores(SystemInformation *system)
{
    return system->getCPUNumberOfCores();
}

QList<unsigned int> SystemRequerimentsTest::getCPUFrequencies(SystemInformation *system)
{
    return system->getCPUFrequencies();
}

QStringList SystemRequerimentsTest::getGPUOpenGLCompatibilities(SystemInformation *system)
{
    return system->getGPUOpenGLCompatibilities();
}

QString SystemRequerimentsTest::getGPUOpenGLVersion(SystemInformation *system)
{
    return system->getGPUOpenGLVersion();
}

QList<unsigned int> SystemRequerimentsTest::getGPURAM(SystemInformation *system)
{
    return system->getGPURAM();
}

QStringList SystemRequerimentsTest::getGPUModel(SystemInformation *system)
{
    return system->getGPUModel();
}

unsigned int SystemRequerimentsTest::getHardDiskFreeSpace(SystemInformation *system, const QString &device)
{
    return system->getHardDiskFreeSpace(device);
}

SystemInformation::OperatingSystem SystemRequerimentsTest::getOperatingSystem(SystemInformation *system)
{
    return system->getOperatingSystem();
}

QString SystemRequerimentsTest::getOperatingSystemVersion(SystemInformation *system)
{
    return system->getOperatingSystemVersion();
}

QString SystemRequerimentsTest::getOperatingSystemServicePackVersion(SystemInformation *system)
{
    return system->getOperatingSystemServicePackVersion();
}

bool SystemRequerimentsTest::isOperatingSystem64BitArchitecture(SystemInformation *system)
{
    return system->isOperatingSystem64BitArchitecture();
}

unsigned int SystemRequerimentsTest::getRAMTotalAmount(SystemInformation *system)
{
    return system->getRAMTotalAmount();
}

QList<QSize> SystemRequerimentsTest::getScreenResolutions(SystemInformation *system)
{
    return system->getScreenResolutions();
}

bool SystemRequerimentsTest::doesOpticalDriveHaveWriteCapabilities(SystemInformation *system)
{
    return system->doesOpticalDriveHaveWriteCapabilities();
}

SystemRequeriments* SystemRequerimentsTest::getSystemRequeriments()
{
    return new SystemRequeriments();
}

}
