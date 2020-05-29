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

#include "systeminformation.h"

#include "screenmanager.h"

#ifdef WIN32
#include "windowssysteminformation.h"
#endif

// Qt
#include <QDesktopWidget>
#include <QRect>
#include <QSize>

namespace udg {

SystemInformation::SystemInformation()
{
}

SystemInformation::~SystemInformation()
{
}

SystemInformation* SystemInformation::newInstance()
{
#ifdef WIN32
    return new WindowsSystemInformation();
#else
    return new SystemInformation();
#endif
}

SystemInformation::OperatingSystem SystemInformation::getOperatingSystem()
{
    return SystemInformation::OSLinux;
}

QString SystemInformation::getOperatingSystemAsString()
{
    return "Unknown";
}

QString SystemInformation::getOperatingSystemAsShortString()
{
    return "Unknown";
}

bool SystemInformation::isOperatingSystem64BitArchitecture()
{
    return true;
}

QString SystemInformation::getOperatingSystemVersion()
{
    return "";
}

QString SystemInformation::getOperatingSystemServicePackVersion()
{
    return "";
}

unsigned int SystemInformation::getRAMTotalAmount()
{
    return 0;
}

QList<unsigned int> SystemInformation::getRAMModulesCapacity()
{
    return QList<unsigned int>();
}

QList<unsigned int> SystemInformation::getRAMModulesFrequency()
{
    return QList<unsigned int>();
}

unsigned int SystemInformation::getCPUNumberOfCores()
{
    return 0;
}

QList<unsigned int> SystemInformation::getCPUFrequencies()
{
    return QList<unsigned int>();
}

unsigned int SystemInformation::getCPUL2CacheSize()
{
    return 0;
}

QStringList SystemInformation::getGPUBrand()
{
    return QStringList();
}

QStringList SystemInformation::getGPUModel()
{
    return QStringList();
}

QList<unsigned int> SystemInformation::getGPURAM()
{
    return QList<unsigned int>();
}

QStringList SystemInformation::getGPUOpenGLCompatibilities()
{
    return QStringList();
}

QString SystemInformation::getGPUOpenGLVersion()
{
    return "0.0";
}

QStringList SystemInformation::getGPUDriverVersion()
{
    return QStringList();
}

QList<QSize> SystemInformation::getScreenResolutions()
{
    QList<QSize> screenResolutions;
    ScreenLayout layout = ScreenManager().getScreenLayout();
    for (int i = 0; i < layout.getNumberOfScreens(); i++)
    {
        screenResolutions.append(layout.getScreen(i).getGeometry().size());
    }
    return screenResolutions;
}

QStringList SystemInformation::getScreenVendors()
{
    return QStringList();
}

QStringList SystemInformation::getHardDiskDevices()
{
    return QStringList();
}

unsigned int SystemInformation::getHardDiskCapacity(const QString &device)
{
    Q_UNUSED(device);
    return 0;
}

unsigned int SystemInformation::getHardDiskFreeSpace(const QString &device)
{
    Q_UNUSED(device);
    return 0;
}

bool SystemInformation::doesOpticalDriveHaveWriteCapabilities()
{
    return false;
}

unsigned int SystemInformation::getNetworkAdapterSpeed()
{
    return 0;
}

bool SystemInformation::isDesktopCompositionAvailable()
{
    return false;
}

bool SystemInformation::isDesktopCompositionEnabled()
{
    return false;
}

}
