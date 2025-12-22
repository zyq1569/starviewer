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

#include "dicomdirreader.h"

// Llegeix el dicom dir
#include <dcdicdir.h>
#include <ofstring.h>
// Make sure OS specific configuration is included first
#include <osconfig.h>
// Provide the information for the tags
#include <dcdeftag.h>
#include <QStringList>
#include <QDir>
#include <QFile>

#include "status.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "dicommask.h"
#include "logging.h"
#include "patientfiller.h"

#include <QTextCodec>

namespace udg {

namespace {

///Returns a text codec suitable for the given directory record.
QTextCodec* getTextCodec(DcmDirectoryRecord *directoryRecord)
{
    QTextCodec *codec = 0;

    if (directoryRecord->tagExists(DCM_SpecificCharacterSet))
    {
        OFString value;
        OFCondition status = directoryRecord->findAndGetOFStringArray(DCM_SpecificCharacterSet, value);

        if (status.good())
        {
            codec = QTextCodec::codecForName(value.c_str());
        }
    }

    if (!codec)
    {
        // Default to Latin-1 if Specific Character Set is not present
        codec = QTextCodec::codecForName("ISO 8859-1");
    }

    return codec;
}

}

DICOMDIRReader::DICOMDIRReader()
{
    m_dicomdir = NULL;
}

DICOMDIRReader::~DICOMDIRReader()
{
}

Status DICOMDIRReader::open(const QString &dicomdirFilePath)
{
    Status state;

    /// There is no command to close a dicomdir, when we want to open a new one,
    ///  the only way to open a new dicomdir, is through the constructor of
    /// DcmDicomDir, passing the path by parameter, so if an open Dicomdir already existed,
    ///  we do a delete, to close it
    if (m_dicomdir != NULL)
    {
        delete m_dicomdir;
    }

    // // Save the directory where the dicomdir is located
    QFileInfo dicomdirFileInfo(dicomdirFilePath);
    m_dicomdirAbsolutePath = dicomdirFileInfo.absolutePath();

    /// The dicom standard indicates that the dicomdir structure must be saved in a file named "DICOMDIR". In linux by
    /// default in vfat drives, shows filenames that are shortname (8 or less characters) in lower case, so
    /// when the dicomdir is saved in vfat units and we want to open it we will find that the file waves saves the information of the
    /// dicomdir will be called "dicomdir" in lower case, for this reason we look for the dicomdir file in both uppercase and lowercase

    /// We look for the name of the file that contains the dicomdir data
    m_dicomdirFileName = dicomdirFileInfo.fileName();

    /// We check if the file system works with lowercase or uppercase name
    if (m_dicomdirFileName == m_dicomdirFileName.toUpper())
    {
        m_dicomFilesInLowerCase = false;
    }
    else
    {
        /// It is mounted on a vfat
        /// indicate that the files are lowercase
        m_dicomFilesInLowerCase = true;
    }

    m_dicomdir = new DcmDicomDir(qPrintable(QDir::toNativeSeparators(dicomdirFilePath)));

    return state.setStatus(m_dicomdir->error());
}

// The dicomdir follows an open structure where we have n patients, who have n studies, which contains n series, and which contains n images, to read the information
// we have to access through this tree structure, first we read the first patient, with the first patient, we can access the second level of the tree, the
// patient studies, and keep doing so until you get to the bottom of everything, the images,
Status DICOMDIRReader::readStudies(QList<Patient*> &outResultsStudyList, DicomMask studyMask)
{
    Status state;

    if (m_dicomdir == NULL)
    {
        // MAKE STATUS RETURN WITH ERROR
        return state.setStatus("Error: Not open dicomfile", false, 1302);
    }

    //We access the tree structure of the dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    //We access the first patient
    DcmDirectoryRecord *patientRecord = root->getSub(0);

    /// In this first while we access the patient Record at the patient data level
    while (patientRecord != NULL)
    {
        Patient *patient = fillPatient(patientRecord);

        /// If he does not comply at the patient level, we will no longer have access to his studies
        if (matchPatientToDicomMask(patient, &studyMask))
        {
            /// We indicate that we want the first study of the patient
            DcmDirectoryRecord *studyRecord = patientRecord->getSub(0);

            ///In this while we access the data of the study
            while (studyRecord != NULL)
            {
                Study *study = fillStudy(studyRecord);

                /// We check if the study meets the search mask they have passed us
                if (matchStudyToDicomMask(study, &studyMask))
                {
                    patient->addStudy(study);
                }
                else
                {
                    delete study;
                }

                /// We access the following patient study
                studyRecord = patientRecord->nextSub(studyRecord);
            }

            /// If no study has completed the search mask we no longer add the patient
            if (patient->getNumberOfStudies() > 0)
            {
                outResultsStudyList.append(patient);
            }
            else
            {
                delete patient;
            }
        }
        else
        {
            delete patient;
        }

        /// We access the next patient of the dicomdir
        patientRecord = root->nextSub(patientRecord);
    }

    return state.setStatus(m_dicomdir->error());
}

/// To find the series of a study we will have to go through all the studies of the patients,
/// that are in the dicomdir, until we obtain the study with the UID
/// requested once found, we will be able to access its information of the series
Status DICOMDIRReader::readSeries(const QString &studyUID, const QString &seriesUID, QList<Series*> &outResultsSeriesList)
{
    Status state;

    if (m_dicomdir == NULL)
    {
        // DO
        return state.setStatus("Error: Not open dicomfile", false, 1302);
    }

    // We access the open structure of the dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    //We access the first patient
    DcmDirectoryRecord *patientRecord = root->getSub(0);
    DcmDirectoryRecord *studyRecord;
    OFString text;
    QString studyUIDRecord, seriesPath;
    bool found = false;

    // We access at the patient level
    while (patientRecord != NULL && !found)
    {
        // We indicate that we want the first study of the patient
        studyRecord = patientRecord->getSub(0);

        while (studyRecord != NULL && !found)
        {
            text.clear();
            studyUIDRecord.clear();
            // We obtain the UID of the study in which we are positioned
            studyRecord->findAndGetOFStringArray(DCM_StudyInstanceUID, text);
            studyUIDRecord = text.c_str();
            // We look for the study that contains the same UID
            if (studyUIDRecord == studyUID)
            {
                found = true;
            }
            else
            {
                /// If we do not find it we access the following study of the patient
                studyRecord = patientRecord->nextSub(studyRecord);
            }
        }

        if (!found)
        {
            /// We access the next patient
            patientRecord = root->nextSub(patientRecord);
        }
    }

    /// If we found the study with the UID we were looking for
    if (found)
    {
        /// Select the study series that contains the studyUID we were looking for
        DcmDirectoryRecord *seriesRecord = studyRecord->getSub(0);

        while (seriesRecord != NULL)
        {
            Series *series = fillSeries(seriesRecord);

            if (seriesUID.length() == 0 || series->getInstanceUID() == seriesUID)
            {
                outResultsSeriesList.append(series);
            }
            else
            {
                delete series;
            }

            /// We access the next series of the study
            seriesRecord = studyRecord->nextSub(seriesRecord);
        }
    }

    return state.setStatus(m_dicomdir->error());
}

Status DICOMDIRReader::readImages(const QString &seriesUID, const QString &sopInstanceUID, QList<Image*> &outResultsImageList)
{
    Status state;

    if (m_dicomdir == NULL)
    {
        // do
        return state.setStatus("Error: Not open dicomfile", false, 1302);
    }

    ///We access the open structure of the dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    /// We access the first patient
    DcmDirectoryRecord *patientRecord = root->getSub(0);
    DcmDirectoryRecord *studyRecord, *seriesRecord;
    OFString text;
    QString studyUIDRecord, seriesUIDRecord, imagePath;
    bool found = false;

    /// We access at the patient level
    while (patientRecord != NULL && !found)
    {
        /// We indicate that we want the first study of the patient
        studyRecord = patientRecord->getSub(0);

        /// We access the study level
        while (studyRecord != NULL && !found)
        {
            /// We selected the study series that contains the studyUID we were looking for
            seriesRecord = studyRecord->getSub(0);
            /// We access level
            while (seriesRecord != NULL && !found)
            {
                /// UID Series
                text.clear();
                seriesUIDRecord.clear();
                seriesRecord->findAndGetOFStringArray(DCM_SeriesInstanceUID, text);
                seriesUIDRecord.insert(0, text.c_str());
                /// We look for the series with the images
                if (seriesUIDRecord == seriesUID)
                {
                    found = true;
                }
                else
                {
                    /// We access the next series of the study
                    seriesRecord = studyRecord->nextSub(seriesRecord);
                }
            }
            ///If we do not find it we access the following study of the patient
            studyRecord = patientRecord->nextSub(studyRecord);
        }

        if (!found)
        {
            ///We access the next patient
            patientRecord = root->nextSub(patientRecord);
        }
    }

    ///If we found the series with the UID we were looking for
    if (found)
    {
        /// We selected the study series that contains the studyUID we were looking for
        DcmDirectoryRecord *imageRecord = seriesRecord->getSub(0);

        while (imageRecord != NULL)
        {
            /// SopUid Image
            Image *image = fillImage(imageRecord);

            if (sopInstanceUID.length() == 0 || sopInstanceUID == image->getSOPInstanceUID())
            {
                /// We insert the image in the list
                outResultsImageList.append(image);
            }
            else
            {
                delete image;
            }

            /// We access the following image in the series
            imageRecord = seriesRecord->nextSub(imageRecord);
        }
    }

    return state.setStatus(m_dicomdir->error());
}

QString DICOMDIRReader::getDicomdirFilePath()
{
    return m_dicomdirAbsolutePath + "/" + m_dicomdirFileName;
}

/// TODO you will need to see if you can use the readimage, readseries and readstudy functions,
/// because this method is the above three methods in a
QStringList DICOMDIRReader::getFiles(const QString &studyUID)
{
    QStringList files;
    Status state;

    if (m_dicomdir == NULL)
    {
        DEBUG_LOG("Error: Not open dicomfile");
        return files;
    }

    /// We access the open structure of the dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    /// We access the first patient
    DcmDirectoryRecord *patientRecord = root->getSub(0);
    DcmDirectoryRecord *studyRecord, *seriesRecord, *imageRecord;

    QString studyUIDRecord, seriesUIDRecord, imagePath;
    bool found = false;

    /// We first find the study we want
    while (patientRecord != NULL && !found)
    {
        /// We indicate that we want the first study of the patient
        studyRecord = patientRecord->getSub(0);
        /// We access the study level
        while (studyRecord != NULL && !found)
        {
            OFString text;
            studyUIDRecord.clear();
            studyRecord->findAndGetOFStringArray(DCM_StudyInstanceUID, text);
            studyUIDRecord = text.c_str();
            ///Is this the studio we want?
            if (studyUIDRecord == studyUID)
            {
                found = true;
            }
            else
            {
                ///If we do not find it we access the following study of the patient
                studyRecord = patientRecord->nextSub(studyRecord);
            }
        }
        if (!found)
        {
            ///We access the next patient
            patientRecord = root->nextSub(patientRecord);
        }
    }

    //// If we have found the requested uid we can continue with the search for the files
    if (found)
    {
        found = false;
        //// Select the study series that contains the studyUID we were looking for
        seriesRecord = studyRecord->getSub(0);
        //// We read all his series
        while (seriesRecord != NULL && !found)
        {
            /// We select each image in the series
            imageRecord = seriesRecord->getSub(0);
            while (imageRecord != NULL)
            {
                OFString text;
                /// Path of the image returns to us the relative path with respect to the dicomdir StudioDirectory / SeriesDirectory / ImageName. Attention returns the directories
                /// separated by '\' (windows format)
                /// We get the relative path of the image
                imageRecord->findAndGetOFStringArray(DCM_ReferencedFileID, text);

                files << m_dicomdirAbsolutePath + "/" + buildImageRelativePath(text.c_str());
                /// We access the following image in the series
                imageRecord = seriesRecord->nextSub(imageRecord);
            }
            //// We access the next series of the study
            seriesRecord = studyRecord->nextSub(seriesRecord);
        }
    }
    else
    {
        DEBUG_LOG("No study was found with this uid: " + studyUID + " when dicomdir");
    }
    return files;
}

Patient* DICOMDIRReader::retrieve(DicomMask maskToRetrieve)
{
    QStringList files = this->getFiles(maskToRetrieve.getStudyInstanceUID());

    PatientFiller patientFiller;

    QList<Patient*> patientsList = patientFiller.processFiles(files);
    if (patientsList.isEmpty())
    {
        return NULL;
    }
    else
    {
        return patientsList.first();
    }
}

// To make the match we will follow the PACS criteria
bool DICOMDIRReader::matchPatientToDicomMask(Patient *patient, DicomMask *mask)
{
    if (!matchDicomMaskToPatientId(mask, patient))
    {
        return false;
    }

    if (!matchDicomMaskToPatientName(mask, patient))
    {
        return false;
    }

    return true;
}

// To make the match we will follow the PACS criteria
bool DICOMDIRReader::matchStudyToDicomMask(Study *study, DicomMask *mask)
{
    if (!matchDicomMaskToStudyDate(mask, study))
    {
        return false;
    }

    if (!matchDicomMaskToStudyUID(mask, study))
    {
        return false;
    }

    return true;
}

bool DICOMDIRReader::matchDicomMaskToStudyUID(DicomMask *mask, Study *study)
{
    return mask->getStudyInstanceUID().length() == 0 || mask->getStudyInstanceUID() == study->getInstanceUID();
}

bool DICOMDIRReader::matchDicomMaskToPatientId(DicomMask *mask, Patient *patient)
{
    ///If the mask is empty we receive '', if it has value we receive * Patient_ID *
    if (mask->getPatientID().length() > 1)
    {
        QString clearedMaskPatientID = mask->getPatientID().remove("*");

        return patient->getID().contains(clearedMaskPatientID, Qt::CaseInsensitive);
    }
    else
    {
        return true;
    }
}

bool DICOMDIRReader::matchDicomMaskToStudyDate(DicomMask *mask, Study *study)
{
    if (mask->getStudyDateMinimum().isValid() && mask->getStudyDateMaximum().isValid())
    {
        return mask->getStudyDateMinimum() <= study->getDate() && mask->getStudyDateMaximum() >= study->getDate();
    }
    else if (mask->getStudyDateMinimum().isValid())
    {
        return mask->getStudyDateMinimum() <= study->getDate();
    }
    else if (mask->getStudyDateMaximum().isValid())
    {
        return mask->getStudyDateMaximum() >= study->getDate();
    }

    return true;
}

bool DICOMDIRReader::matchDicomMaskToPatientName(DicomMask *mask, Patient *patient)
{
    ///If the mask is empty we receive '', if it has value it is received * NAME_FIND SEARCH *
    if (mask->getPatientName().length() > 1)
    {
        QString clearedPatientNameMask = mask->getPatientName().remove("*");

        return patient->getFullName().contains(clearedPatientNameMask, Qt::CaseInsensitive);
    }
    else
    {
        return true;
    }
}

Patient* DICOMDIRReader::fillPatient(DcmDirectoryRecord *dcmDirectoryRecordPatient)
{
    QTextCodec *codec = getTextCodec(dcmDirectoryRecordPatient);
    OFString tagValue;
    Patient *patient = new Patient();

    // Patient name
    dcmDirectoryRecordPatient->findAndGetOFStringArray(DCM_PatientName, tagValue);
    patient->setFullName(codec->toUnicode(tagValue.c_str()));
    // Id patient
    dcmDirectoryRecordPatient->findAndGetOFStringArray(DCM_PatientID, tagValue);
    patient->setID(codec->toUnicode(tagValue.c_str()));

    return patient;
}

Study* DICOMDIRReader::fillStudy(DcmDirectoryRecord *dcmDirectoryRecordStudy)
{
    QTextCodec *codec = getTextCodec(dcmDirectoryRecordStudy);
    OFString tagValue;

    Study *study = new Study();
    // Id study
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyID, tagValue);
    study->setID(codec->toUnicode(tagValue.c_str()));

    // Study time
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyTime, tagValue);
    study->setTime(tagValue.c_str());

    // Study date
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyDate, tagValue);
    study->setDate(tagValue.c_str());

    // Study description
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyDescription, tagValue);
    study->setDescription(codec->toUnicode(tagValue.c_str()));

    // Accession number
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_AccessionNumber, tagValue);
    study->setAccessionNumber(codec->toUnicode(tagValue.c_str()));

    // We get the UID of the study
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyInstanceUID, tagValue);
    study->setInstanceUID(tagValue.c_str());

    return study;
}

Series* DICOMDIRReader::fillSeries(DcmDirectoryRecord *dcmDirectoryRecordSeries)
{
    QTextCodec *codec = getTextCodec(dcmDirectoryRecordSeries);
    OFString tagValue;
    Series *series = new Series;

    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_SeriesInstanceUID, tagValue);
    series->setInstanceUID(tagValue.c_str());

    //// Serial Number
    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_SeriesNumber, tagValue);
    series->setSeriesNumber(tagValue.c_str());

    //// Series mode
    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_Modality, tagValue);
    series->setModality(tagValue.c_str());

    /// Protocol Name
    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_ProtocolName, tagValue);
    series->setProtocolName(codec->toUnicode(tagValue.c_str()));

    return series;
}

Image* DICOMDIRReader::fillImage(DcmDirectoryRecord *dcmDirectoryRecordImage)
{
    OFString tagValue;
    Image *image = new Image();

    // SopUid Image
    dcmDirectoryRecordImage->findAndGetOFStringArray(DCM_ReferencedSOPInstanceUIDInFile, tagValue);
    image->setSOPInstanceUID(tagValue.c_str());

    /// Instance Number
    dcmDirectoryRecordImage->findAndGetOFStringArray(DCM_InstanceNumber, tagValue);
    image->setInstanceNumber(tagValue.c_str());

    // Path of the image returns to us the relative path with respect to the dicomdir
    // StudioDirectory / SeriesDirectory / ImageName. Attention returns the separate directories
    // for '/', for linux should be transformed to '\'
    // We get the relative path of the image
    dcmDirectoryRecordImage->findAndGetOFStringArray(DCM_ReferencedFileID, tagValue);
    image->setPath(m_dicomdirAbsolutePath + "/" + buildImageRelativePath(tagValue.c_str()));

    return image;
}

QString DICOMDIRReader::backSlashToSlash(const QString &original)
{
    QString ret;

    ret = original;

    while (ret.indexOf("\\") != -1)
    {
        ret.replace(ret.indexOf("\\"), 1, "/");
    }

    return ret;
}

QString DICOMDIRReader::buildImageRelativePath(const QString &imageRelativePath)
{
    // Default Linux on vfat drives, shows filenames that are shortname (8 or fewer characters) in
    // lowercase because in the dicomdir file the file paths are saved in uppercase, m_dicomFilesInLowerCase
    // is true if tel dicomdir file is found in lowercase, if dicomdir is consistent with the names of the images and
    // routes will also be lowercase

    if (m_dicomFilesInLowerCase)
    {
        return backSlashToSlash(imageRelativePath).toLower();
    }
    else
    {
        return backSlashToSlash(imageRelativePath);
    }
}

}
