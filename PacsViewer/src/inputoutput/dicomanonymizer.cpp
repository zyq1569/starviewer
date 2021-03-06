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

#include "dicomanonymizer.h"

#include <gdcmGlobal.h>
#include <gdcmUIDGenerator.h>
#include <gdcmReader.h>
#include <gdcmWriter.h>
#include <gdcmDefs.h>
#include <QCoreApplication>
#include <QDir>
#include <dcuid.h>

#include "logging.h"

namespace udg {

DICOMAnonymizer::DICOMAnonymizer()
{
    initializeGDCM();

    m_replacePatientIDInsteadOfRemove = false;
    m_replaceStudyIDInsteadOfRemove = false;
    m_removePritaveTags = true;
    m_patientNameAnonymized = "";
}

DICOMAnonymizer::~DICOMAnonymizer()
{
    delete m_gdcmAnonymizer;
}

void DICOMAnonymizer::setPatientNameAnonymized(const QString &patientNameAnonymized)
{
    ///According to DICOM a PN (PersonName) tag cannot be longer than 64 characters
    m_patientNameAnonymized = patientNameAnonymized.left(64);
}

QString DICOMAnonymizer::getPatientNameAnonymized() const
{
    return m_patientNameAnonymized;
}

void DICOMAnonymizer::setReplacePatientIDInsteadOfRemove(bool replace)
{
    m_replacePatientIDInsteadOfRemove = replace;
}

bool DICOMAnonymizer::getReplacePatientIDInsteadOfRemove()
{
    return m_replacePatientIDInsteadOfRemove;
}

void DICOMAnonymizer::setReplaceStudyIDInsteadOfRemove(bool replace)
{
    m_replaceStudyIDInsteadOfRemove = replace;
}

bool DICOMAnonymizer::getReplaceStudyIDInsteadOfRemove()
{
    return m_replaceStudyIDInsteadOfRemove;
}

void DICOMAnonymizer::setRemovePrivateTags(bool removePritaveTags)
{
    m_removePritaveTags = removePritaveTags;
}

bool DICOMAnonymizer::getRemovePrivateTags()
{
    return m_removePritaveTags;
}

void DICOMAnonymizer::initializeGDCM()
{
    m_gdcmAnonymizer = new gdcm::gdcmAnonymizerStarviewer();
    gdcm::Global *gdcmGlobalInstance = &gdcm::Global::GetInstance();

    // We indicate the directory where you can find the part3.xml file which is a DICOM dictionary.
    // TODO: Where we put the part3.xml file
    gdcmGlobalInstance->Prepend(qPrintable(QCoreApplication::applicationDirPath()));

    // Upload the part3.xml file
    if (!gdcmGlobalInstance->LoadResourcesFiles())
    {
        ERROR_LOG("Part3.xml file not found in" + QCoreApplication::applicationDirPath());
    }

    const gdcm::Defs &defs = gdcmGlobalInstance->GetDefs();
    (void)defs;
    /// TODO:we use the dcmtk UID we should have our own this also happens in VolumeBuilderFromCaptures
    if (!gdcm::UIDGenerator::IsValid(SITE_UID_ROOT))
    {
        ERROR_LOG(QString("DICOM files cannot be anonymized because the root UID to create the new files is invalid %1").arg(SITE_UID_ROOT));
    }

    gdcm::UIDGenerator::SetRoot(SITE_UID_ROOT);
}

bool DICOMAnonymizer::anonymyzeDICOMFilesDirectory(const QString &directoryPath)
{
    QDir directory;
    directory.setPath(directoryPath);

    foreach (const QFileInfo &entryInfo, directory.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files))
    {
        if (entryInfo.isDir())
        {
            if (!anonymyzeDICOMFilesDirectory(entryInfo.absoluteFilePath()))
            {
                return false;
            }
        }
        else
        {
            if (!anonymizeDICOMFile(entryInfo.absoluteFilePath(), entryInfo.absoluteFilePath()))
            {
                return false;
            }
        }
    }

    return true;
}

bool DICOMAnonymizer::anonymizeDICOMFile(const QString &inputPathFile, const QString &outputPathFile)
{
    gdcm::Reader gdcmReader;
    gdcmReader.SetFileName(qPrintable(inputPathFile));

    if (!gdcmReader.Read())
    {
        ERROR_LOG("The file to be anonymized was not found " + inputPathFile);
        return false;
    }

    gdcm::File &gdcmFile = gdcmReader.GetFile();
    gdcm::MediaStorage gdcmMediaStorage;
    gdcmMediaStorage.SetFromFile(gdcmFile);
    if (!gdcm::Defs::GetIODNameFromMediaStorage(gdcmMediaStorage))
    {
        ERROR_LOG(QString("Media storage type of unsupported file: %1").arg(gdcmMediaStorage.GetString()));
        return false;
    }

    QString originalPatientID = readTagValue(&gdcmFile, gdcm::Tag(0x0010, 0x0020));
    QString originalStudyInstanceUID = readTagValue(&gdcmFile, gdcm::Tag(0x0020, 0x000d));

    m_gdcmAnonymizer->SetFile(gdcmFile);
    if (!m_gdcmAnonymizer->BasicApplicationLevelConfidentialityProfile(true))
    {
        ERROR_LOG("Could not anonymize file" + inputPathFile);
        return false;
    }

    // Estableix el mom del pacient anonimitzat
    m_gdcmAnonymizer->Replace(gdcm::Tag(0x0010, 0x0010), qPrintable(m_patientNameAnonymized));

    if (getReplacePatientIDInsteadOfRemove())
    {
        // ID Pacient
        m_gdcmAnonymizer->Replace(gdcm::Tag(0x0010, 0x0020), qPrintable(getAnonimyzedPatientID(originalPatientID)));
    }

    if (getReplaceStudyIDInsteadOfRemove())
    {
        // ID Estudi
        m_gdcmAnonymizer->Replace(gdcm::Tag(0x0020, 0x0010), qPrintable(getAnonymizedStudyID(originalStudyInstanceUID)));
    }

    if (getRemovePrivateTags())
    {
        if (!m_gdcmAnonymizer->RemovePrivateTags())
        {
            ERROR_LOG("Could not remove private tags from file " + inputPathFile);
            return false;
        }
    }

    ///We will regenerate the DICOM header with the new SOP Instance UID
    gdcm::FileMetaInformation gdcmFileMetaInformation = gdcmFile.GetHeader();
    gdcmFileMetaInformation.Clear();

    gdcm::Writer gdcmWriter;
    gdcmWriter.SetFileName(qPrintable(outputPathFile));
    gdcmWriter.SetFile(gdcmFile);
    if (!gdcmWriter.Write())
    {
        ERROR_LOG("Could not generate anonymized file from " + inputPathFile + " a " + outputPathFile);
        return false;
    }

    return true;
}

QString DICOMAnonymizer::getAnonimyzedPatientID(const QString &originalPatientID)
{
    if (!m_hashOriginalPatientIDToAnonimyzedPatientID.contains(originalPatientID))
    {
        m_hashOriginalPatientIDToAnonimyzedPatientID.insert(originalPatientID, QString::number(m_hashOriginalPatientIDToAnonimyzedPatientID.count() + 1));
    }

    return m_hashOriginalPatientIDToAnonimyzedPatientID.value(originalPatientID);
}

QString DICOMAnonymizer::getAnonymizedStudyID(const QString &originalStudyInstanceUID)
{
    if (!m_hashOriginalStudyInstanceUIDToAnonimyzedStudyID.contains(originalStudyInstanceUID))
    {
        m_hashOriginalStudyInstanceUIDToAnonimyzedStudyID.insert(originalStudyInstanceUID,
                                                                 QString::number(m_hashOriginalStudyInstanceUIDToAnonimyzedStudyID.count() + 1));
    }

    return m_hashOriginalStudyInstanceUIDToAnonimyzedStudyID.value(originalStudyInstanceUID);
}

QString DICOMAnonymizer::readTagValue(gdcm::File *gdcmFile, gdcm::Tag tagToRead) const
{
    gdcm::DataElement dataElement = gdcmFile->GetDataSet().GetDataElement(tagToRead);
    QString tagValue = "";

    if (!dataElement.IsEmpty())
    {
        if (const gdcm::ByteValue *byteValueTag = dataElement.GetByteValue())
        {
            tagValue = (std::string(byteValueTag->GetPointer(), byteValueTag->GetLength())).c_str();
        }
    }

    return tagValue;
}

}
