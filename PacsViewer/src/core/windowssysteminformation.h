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

#ifndef UDGWINDOWSSYSTEMINFORMATION_H
#define UDGWINDOWSSYSTEMINFORMATION_H

#include "systeminformation.h"
// Qt
#include <QStringList>
// Windows
#include <Wbemidl.h>

namespace udg {

class WindowsSystemInformation : public SystemInformation {
public:
    WindowsSystemInformation();
    ~WindowsSystemInformation();
    
    OperatingSystem getOperatingSystem();
    bool isOperatingSystem64BitArchitecture();
    QString getOperatingSystemVersion();
    QString getOperatingSystemServicePackVersion();
    QString getOperatingSystemName();
    QString getOperatingSystemAsString();
    QString getOperatingSystemAsShortString();

    /// Returns the total amount of RAM in MegaBytes
    unsigned int getRAMTotalAmount();
    QList<unsigned int> getRAMModulesCapacity();
    QList<unsigned int> getRAMModulesFrequency();

    unsigned int getCPUNumberOfCores();
    
    /// Returns a list of the frequency of each processor
    QList<unsigned int> getCPUFrequencies();
    unsigned int getCPUL2CacheSize();

    QStringList getGPUBrand();
    QStringList getGPUModel();
    QList<unsigned int> getGPURAM();
    QStringList getGPUOpenGLCompatibilities();
    QString getGPUOpenGLVersion();
    QStringList getGPUDriverVersion();

    // Screen, Display, Monitor, Desktop, ...
    QStringList getScreenVendors();

    QStringList getHardDiskDevices();
    unsigned int getHardDiskCapacity(const QString &device); // From the disc dur I counted the Starviewer cache folder
    unsigned int getHardDiskFreeSpace(const QString &device);
    bool doesOpticalDriveHaveWriteCapabilities();

    unsigned int getNetworkAdapterSpeed();

    bool isDesktopCompositionAvailable();
    bool isDesktopCompositionEnabled();

protected:
    /// Alternative method in case we can't get the number of cores via WMI
    virtual unsigned int getCPUNumberOfCoresFromEnvironmentVar();

    /// Gets the major version of the service pack
    QString getOperatingSystemServicePackMajorVersion();

    /// Gets the minor version of the service pack
    QString getOperatingSystemServicePackMinorVersion();

protected:
    virtual IWbemClassObject* getNextObject(IEnumWbemClassObject *enumerator);
    virtual IEnumWbemClassObject* executeQuery(QString query);
    virtual bool getProperty(IWbemClassObject *object, QString propertyName, VARIANT *propertyVariant);
    IWbemServices* initializeAPI();
    virtual void uninitializeAPI(IWbemServices *services);
    virtual QString createOpenGLContextAndGetExtensions();
    virtual QString createOpenGLContextAndGetVersion();

protected:
    IWbemServices* m_api;

private:
    /// Returns a pointer to Dwmapi.dll if available, null otherwise
    HMODULE getDesktopWindowManagerDLL();

private:
    /// Constant string for Dwmapi.dll
    static const LPWSTR DesktopWindowManagerDLLName;

    /// Pointer to DwmIsCompositionEnabled function address (Dwmapi.dll)
    typedef HRESULT (WINAPI* DwmIsCompositionEnabledType)(BOOL*);
};

}

#endif
