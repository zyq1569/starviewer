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

#include "dicomdirburningapplication.h"

#include <QProcess>
#include <QFile>
#include <QDir>

#include "logging.h"
#include "inputoutputsettings.h"

namespace udg {

DICOMDIRBurningApplication::DICOMDIRBurningApplication()
{
}

DICOMDIRBurningApplication::DICOMDIRBurningApplication(const QString &isoPath, const CreateDicomdir::recordDeviceDicomDir &currentDevice)
{
    m_isoPath = isoPath;
    m_currentDevice = currentDevice;
}

DICOMDIRBurningApplication::~DICOMDIRBurningApplication()
{
}

void DICOMDIRBurningApplication::setIsoPath(const QString &isoPath)
{
    m_isoPath = isoPath;
}

QString DICOMDIRBurningApplication::getIsoPath() const
{
    return m_isoPath;
}

void DICOMDIRBurningApplication::setCurrentDevice(const CreateDicomdir::recordDeviceDicomDir &currentDevice)
{
    m_currentDevice = currentDevice;
}

CreateDicomdir::recordDeviceDicomDir DICOMDIRBurningApplication::getCurrentDevice() const
{
    return m_currentDevice;
}

DICOMDIRBurningApplication::DICOMDIRBurningApplicationError DICOMDIRBurningApplication::getLastError() const
{
    return m_lastError;
}

QString DICOMDIRBurningApplication::getLastErrorDescription() const
{
    return m_lastErrorDescription;
}

bool DICOMDIRBurningApplication::burnIsoImageFile()
{
    /// Check that the iso file you want to burn to CD or DVD exists
    if (!QFile::exists(m_isoPath))
    {
        m_lastErrorDescription = QObject::tr("The ISO image \"%1\" to burn does not exist.").arg(m_isoPath);
        m_lastError = IsoPathNotFound;
        return false;
    }

    Settings settings;
    QString burningApplicationPath = (settings.getValue(InputOutputSettings::DICOMDIRBurningApplicationPathKey)).toString();

    /// It is verified that the path of the recording application is correct,
    /// although in principle it has been validated in the DICOMDIR configuration
    if (!QFile::exists(burningApplicationPath))
    {
        m_lastErrorDescription = QObject::tr("The burn application path \"%1\" does not exist.").arg(burningApplicationPath);
        m_lastError = BurnApplicationPathNotFound;
        return false;
    }

    QProcess process;
    QStringList processParameters;

    /// If the option to enter different parameters is activated depending on whether you want
    /// to burn a CD or a DVD you will need to add them to the processParameters
    if ((settings.getValue(InputOutputSettings::DICOMDIRBurningApplicationHasDifferentCDDVDParametersKey)).toBool())
    {
        switch (m_currentDevice)
        {
        case CreateDicomdir::CdRom:
            processParameters << (settings.getValue(InputOutputSettings::DICOMDIRBurningApplicationCDParametersKey)).toString()
                                 .arg(QDir::toNativeSeparators(m_isoPath)).split(" ");
            break;
        case CreateDicomdir::DvdRom:
            processParameters << (settings.getValue(InputOutputSettings::DICOMDIRBurningApplicationDVDParametersKey)).toString()
                                 .arg(QDir::toNativeSeparators(m_isoPath)).split(" ");
            break;
        default:
            break;
        }
    }
    else
    {
        processParameters << (settings.getValue(InputOutputSettings::DICOMDIRBurningApplicationParametersKey)).toString()
                             .arg(QDir::toNativeSeparators(m_isoPath)).split(" ");
    }

    process.start(burningApplicationPath, processParameters);
    process.waitForFinished(-1);

    if (process.exitCode() != 0)
    {
        m_lastErrorDescription = QObject::tr("An error occurred during the ISO image file burn process.");
        m_lastError = InternalError;

        ERROR_LOG("Error recording ISO image with order: " + burningApplicationPath + "; With parameters: " + processParameters.join(" ") +
                  "; Exit code qprocess: " + process.exitCode());
        return false;
    }
    return true;
}

}
