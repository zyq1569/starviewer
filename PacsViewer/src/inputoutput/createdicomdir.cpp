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

#include "createdicomdir.h"

#include <QString>
#include <QDir>
// Make sure OS specific configuration is included first
#include <osconfig.h>
#include <dctk.h>
// For class DicomDirInterface
#include <dcddirif.h>
// For class OFStandard
#include <ofstd.h>
// For class OFCondition
#include <ofcond.h>

#include "status.h"
#include "logging.h"

#if defined (HAVE_WINDOWS_H) || defined(HAVE_FNMATCH_H)
#define PATTERN_MATCHING_AVAILABLE
#endif

namespace udg {

CreateDicomdir::CreateDicomdir()
{
    // It allows to record to the discdur and tb usb's
    m_optProfile = DicomDirInterface :: AP_GeneralPurpose;
    // The dicom regulations state that the name of dicomdir files must be 8 characters long and must be in uppercase. Linux for
    // vfat file systems such as flash drives and for filenames of 8 characters or less always show the names in lower case, regardless
    // that we save them in capital letters, due to this when we want to create a dicomdir in a vfat device the names of the images are copied in
    // lowercase, when generating the dicomdir gives us problems, why are images in lowercase and DICOM does not allow it. To solve these cases
    // we need to enable dcmtk enableMapFilenamesMode, which if it encounters lowercase files ignores it and creates the dicomdir.

    m_ddir.enableMapFilenamesMode(OFTrue);
}

CreateDicomdir::~CreateDicomdir()
{
}

void CreateDicomdir::setDevice(recordDeviceDicomDir deviceToCreateDicomdir)
{
    // Indiquem que el propòsit d'aquest dicomdir
    switch (deviceToCreateDicomdir)
    {
    case recordDeviceDicomDir(HardDisk):
        m_optProfile = DicomDirInterface::AP_GeneralPurpose;
        break;
    case recordDeviceDicomDir(CdRom):
        m_optProfile = DicomDirInterface::AP_GeneralPurpose;
        break;
    case recordDeviceDicomDir(DvdRom):
        m_optProfile = DicomDirInterface::AP_GeneralPurposeDVDJPEG;
        break;
    case recordDeviceDicomDir(UsbPen):
        m_optProfile = DicomDirInterface::AP_GeneralPurpose;
        break;
    default:
        m_optProfile = DicomDirInterface::AP_GeneralPurpose;
        break;
    }
}

void CreateDicomdir::setStrictMode(bool enabled)
{
    if (enabled)
    {
        /// We reject images that contain type 1 tags with length 0
        m_ddir.enableInventMode(OFFalse);
        ///Reject Images that do not meet the dicom standard in encoding pixel information
        m_ddir.disableEncodingCheck(OFFalse);
        /// Reject images that do not meet the dicom standard in encoding pixel information
        m_ddir.disableResolutionCheck(OFFalse);
        /// We reject images that do not have PatientID
        m_ddir.enableInventPatientIDMode(OFFalse);

        INFO_LOG("DICOMDIR will be created in strict DICOM compliance mode");
    }
    else
    {
        // We don't want strict mode
        /// If an image does not have a level 1 tag, which are the required tags and cannot have length 1,
        /// when creating the dicomdir they are invented
        m_ddir.enableInventMode(OFTrue);
        ///Accept Images that do not meet the dicom standard in encoding pixel information
        m_ddir.disableEncodingCheck(OFTrue);
        /// Accept Images that do not meet the spatial resolution
        m_ddir.disableResolutionCheck(OFTrue);
        /// In case a patient does not have PatientID it is invented
        m_ddir.enableInventPatientIDMode(OFTrue);

        INFO_LOG("The DICOMDIR will be created in permissive mode in the DICOM compliment");
    }
}

void CreateDicomdir::setCheckTransferSyntax(bool checkTransferSyntax)
{
    /// Attention the name of the disableTransferSyntaxCheck method is misleading,
    /// because to disable the check of transfer syntax, it must be invoked by passing it
    /// the variable with the value false, and to indicate that the transfer syntax must
    /// be checked it must be called with the variable to true
    m_ddir.disableTransferSyntaxCheck(checkTransferSyntax);
}

Status CreateDicomdir::create(QString dicomdirPath)
{
    /// File name dicomDir
    QString outputDirectory = dicomdirPath + "/DICOMDIR";
    /// Create list of input files
    OFList<OFString> fileNames;
    const char *opt_pattern = NULL;
    const char *opt_fileset = DEFAULT_FILESETID;
    const char *opt_descriptor = NULL;
    const char *opt_charset = DEFAULT_DESCRIPTOR_CHARSET;
    OFCondition result;
    E_EncodingType opt_enctype = EET_ExplicitLength;
    E_GrpLenEncoding opt_glenc = EGL_withoutGL;

    Status state;

    /// We look for the file in dicomdir. Previously, the files of the selected
    /// studies must have been copied to the ConvertoToDicomdir class.
    /// in the destination dicomdir directory
    OFStandard::searchDirectoryRecursively("", fileNames, opt_pattern, qPrintable(QDir::toNativeSeparators(dicomdirPath)));

    // Comprovem que el directori no estigui buit
    if (fileNames.empty())
    {
        ERROR_LOG(QString("El directori destí del DICOMDIR [%1] està buit o no hi tenim accés per llistar-ne el contingut. No podem crear l'arxiu de DICOMDIR.")
                  .arg(dicomdirPath));
        state.setStatus("DICOMDIR file destination directory is either empty or not readable.", false, 1301);
        return state;
    }

    /// We check that the directory is not empty
    result = m_ddir.createNewDicomDir(m_optProfile, qPrintable(QDir::toNativeSeparators(outputDirectory)), opt_fileset);

    if (!result.good())
    {
        ERROR_LOG("Error al crear el DICOMDIR. ERROR : " + QString(result.text()));
        state.setStatus(result);
        return state;
    }

    // Set fileset descriptor and character set
    result = m_ddir.setFilesetDescriptor(opt_descriptor, opt_charset);
    if (result.good())
    {
        OFListIterator(OFString) iter = fileNames.begin();
        OFListIterator(OFString) last = fileNames.end();

        //// Iterate over the file list and add them to the dicomdir
        while ((iter != last) && result.good())
        {
            /// We add the files to the dicomdir
            result = m_ddir.addDicomFile(qPrintable(QString((*iter).c_str()).toUpper()), qPrintable(QDir::toNativeSeparators(dicomdirPath)));
            if (result.good())
            {
                iter++;
            }
        }

        if (!result.good())
        {
            ERROR_LOG("Error converting file to DICOMDIR : " + dicomdirPath + "/" + (*iter).c_str() + result.text());
            result = EC_IllegalCall;
        }
        else
        {
            /// Write the dicomDir
            result = m_ddir.writeDicomDir(opt_enctype, opt_glenc);
        }
    }

    return state.setStatus(result);
}

}
