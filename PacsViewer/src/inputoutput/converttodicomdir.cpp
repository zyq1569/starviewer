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

#include "converttodicomdir.h"

#include <QProgressDialog>
#include <QTextStream>
#include <QFile>

#include "logging.h"
#include "status.h"
#include "dicommask.h"
#include "convertdicomtolittleendian.h"
#include "directoryutilities.h"
#include "starviewerapplication.h"
#include "localdatabasemanager.h"
#include "patient.h"
#include "study.h"
#include "image.h"
#include "inputoutputsettings.h"
#include "dicomanonymizer.h"

namespace udg {

ConvertToDicomdir::ConvertToDicomdir()
{
    m_study = 0;
    m_series = 0;
    m_currentItemNumber = 0;
    m_patient = 0;

    m_convertDicomdirImagesToLittleEndian = false;
    m_anonymizeDICOMDIR = false;
    m_progress = NULL;
    m_DICOMAnonymizer = NULL;
}

void ConvertToDicomdir::setAnonymizeDICOMDIR(bool anonymizeDICOMDIR, QString patientNameAnonymized)
{
    m_anonymizeDICOMDIR = anonymizeDICOMDIR;
    m_patientNameAnonymized = patientNameAnonymized;
}

void ConvertToDicomdir::addStudy(const QString &studyUID)
{
    // Studies should be grouped by patient, what we do is add them to list of studies to convert to
    // dicomdir already sorted by patient
    StudyToConvert studyToConvert;
    int index = 0;
    bool stop = false;

    LocalDatabaseManager localDatabaseManager;
    DicomMask studyMask;
    studyMask.setStudyInstanceUID(studyUID);
    QList<Patient*> patientList = localDatabaseManager.queryPatientsAndStudies(studyMask);
    if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
    {
        ERROR_LOG(QString("Error adding a study to generate a DICOMDIR; Error:% 1; StudyUID: %2")
                  .arg(localDatabaseManager.getLastError())
                  .arg(studyUID));
        return;
    }

    // \ TODO This must be done because queryPatientStudy returns Patient list
    // We really only want one.
    Patient *patient = patientList.first();

    studyToConvert.studyUID = studyUID;
    studyToConvert.patientId = patient->getID();

    delete patient;

    // We look for the position where the study should be inserted in the list of studies to convert to dicomdir, sorting by patient id
    while (index < m_studiesToConvert.count() && !stop)
    {
        // We compare with the other studies in the list, until we find their corresponding place
        if (studyToConvert.patientId < m_studiesToConvert.at(index).patientId)
        {
            stop = true;
        }
        else
        {
            index++;
        }
    }

    // Once we have found the position where we need to insert it
    if (stop)
    {
        m_studiesToConvert.insert(index, studyToConvert);
    }
    else
    {
        // In this case it is valid in the end
        m_studiesToConvert.push_back(studyToConvert);
    }
}

/// TODO: If the creation of the DICOMDIR Fails this method deletes the contents of
/// the directory where the DICOMDIR was to be created, by doing this
/// the class before creating the DICOMDIR should check that the directory is empty,
/// because otherwise we could delete user content.
/// Now this check is done in QCreateDicomdir and should be done here
Status ConvertToDicomdir::convert(const QString &dicomdirPath, CreateDicomdir::recordDeviceDicomDir selectedDevice, bool copyFolderContent)
{
    /// We first copy the studies to the destination directory,
    /// and then turn the directory into a dicomdir
    Status state;
    int totalNumberOfItems = 0;

    m_dicomDirPath = dicomdirPath;

    QString pathFolderContentToCopyToDICOMDIR = Settings().getValue(InputOutputSettings::DICOMDIRFolderPathToCopy).toString();
    if (copyFolderContent && !AreValidRequirementsOfFolderContentToCopyToDICOMDIR(pathFolderContentToCopyToDICOMDIR))
    {
        ERROR_LOG(QString("Unable to create DICOMDIR because path% 1 contains an item named DICOM or DICOMDIR")
                  .arg(pathFolderContentToCopyToDICOMDIR));
        state.setStatus("", false, 4003);
        return state;
    }

    DicomMask studyMask;
    QList<Study*> studyList;

    LocalDatabaseManager localDatabaseManager;

    foreach (StudyToConvert studyToConvert, m_studiesToConvert)
    {
        studyMask.setStudyInstanceUID(studyToConvert.studyUID);
        Patient *patient = localDatabaseManager.retrieve(studyMask);
        if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
        {
            ERROR_LOG(QString("Studio% 1 data not found in database ").arg(studyToConvert.studyUID));
            QString error = QString("Error performing a retrieve study to generate a DICOMDIR; Error: %1; StudyUID: %2")
                    .arg(localDatabaseManager.getLastError())
                    .arg(studyToConvert.studyUID);
            state.setStatus(error, false, -1);
            break;
        }
        else
        {
            state.setStatus("", true, -1);
        }

        /// \ TODO This patient-> getStudies (). First must be done because queryPatientStudy returns Patient list
        /// We really only want one study.
        Study *study = patient->getStudies().first();
        studyList.append(study);
        foreach (Series *series, study->getSeries())
        {
            totalNumberOfItems += series->getNumberOfItems();
        }
    }

    if (!state.good())
    {
        DirectoryUtilities().deleteDirectory(m_dicomDirPath, false);
        return state;
    }

    /// We add another image to prevent the progress bar from reaching 100%, and so wait for the dicomdir to be created, which is done when invoked
    /// createDicomdir.Create ()
    m_progress = new QProgressDialog(tr("Creating DICOMDIR..."), "", 0, totalNumberOfItems + 1);
    m_progress->setMinimumDuration(0);
    m_progress->setCancelButton(0);
    m_progress->setModal(true);

    if (m_anonymizeDICOMDIR)
    {
        INFO_LOG("The DICOMDIR is to create anonymity");
        m_DICOMAnonymizer = new DICOMAnonymizer();
        m_DICOMAnonymizer->setPatientNameAnonymized(m_patientNameAnonymized);
        m_DICOMAnonymizer->setReplacePatientIDInsteadOfRemove(true);
        m_DICOMAnonymizer->setReplaceStudyIDInsteadOfRemove(true);
        m_DICOMAnonymizer->setRemovePrivateTags(true);
    }
    else
    {
        m_DICOMAnonymizer = NULL;
    }

    //We copy the images of the selected studies to the destination directory
    state = copyStudiesToDicomdirPath(studyList);

    if (!state.good())
    {
        m_progress->close();
        DirectoryUtilities().deleteDirectory(m_dicomDirPath, false);
        return state;
    }
    // Once the images have been copied, we create them
    state = createDicomdir(m_dicomDirPath, selectedDevice);

    if (!state.good() && state.code() != 4001)
    {
        // Error 4001 is that the images do not meet the 100 standard, but dicomdir can be used
        DirectoryUtilities().deleteDirectory(m_dicomDirPath, false);
    }
    else
    {
        if (copyFolderContent)
        {
            /// Copy the contents of the directory must be done once the DICOMDIR has been created,
            ///  because if dcmtk detects in the directory where it should be created
            /// the DICOMDIR that there are non-DICOM files will fail.

            if (!copyFolderContentToDICOMDIR())
            {
                m_progress->close();
                DirectoryUtilities().deleteDirectory(m_dicomDirPath, false);
                state.setStatus("", false, 4002);
                return state;
            }
        }

        createReadmeTxt();
    }

    m_progress->close();

    if (m_DICOMAnonymizer)
    {
        delete m_DICOMAnonymizer;
        m_DICOMAnonymizer = NULL;
    }

    if (m_progress)
    {
        delete m_progress;
        m_progress = NULL;
    }

    return state;
}

Status ConvertToDicomdir::createDicomdir(const QString &dicomdirPath, CreateDicomdir::recordDeviceDicomDir selectedDevice)
{
    CreateDicomdir createDicomdir;
    Status state, stateNotDicomConformance;

    /// The DICOM indicates for each profile the images must be of certain transfer syntax,
    /// for example for STD-GEN-CD they must be Explicit
    /// Little Endian for STD-GEN-DVD-JPEG can be Explicit Little Endian and also support some
    /// lossy and lossless JPEG syntax transfer. What they have
    /// all common profiles are that they support Explicit Little Endian, so if we don't
    /// convert the images to Little Endian before generating the DICOMDIR
    /// we can find that it contains some image with some transfer syntax, that according
    /// to the norm dicom is not accepted by this profile, for that reason
    /// indicate that the transfer syntax is not checked.
    /// TENSION if we disable syntax transfer checking we can have DICOMDIR that are not DICOM conformance
    createDicomdir.setCheckTransferSyntax(getConvertDicomdirImagesToLittleEndian());

    createDicomdir.setDevice(selectedDevice);
    /// Invoquem el mètode per convertir el directori destí Dicomdir on ja s'han copiat les imatges en un dicomdir
    state = createDicomdir.create(dicomdirPath);
    ///  Ha fallat crear el dicomdir, ara intentem crear-lo en mode no estricte
    if (!state.good())
    {
        WARN_LOG("Algunes de les imatges no compleixen l'estandard DICOM al 100% es provara de crear el DICOMDIR sense el mode estricte");
        createDicomdir.setStrictMode(false);
        state = createDicomdir.create(dicomdirPath);
        if (state.good())
        {
            return stateNotDicomConformance.setStatus("Alguna de les imatges no complia l'estàndard DICOM", false, 4001);
        }
    }

    /// We have assigned as value to progressbar Number Images +1, +1 is the step of converting the files to dicomdir
    m_progress->setValue(m_progress->value() + 1);

    return state;
}

void ConvertToDicomdir::setConvertDicomdirImagesToLittleEndian(bool convertDicomdirImagesToLittleEndian)
{
    m_convertDicomdirImagesToLittleEndian = convertDicomdirImagesToLittleEndian;

    if (convertDicomdirImagesToLittleEndian)
    {
        INFO_LOG("Les imatges del dicomdir es transformaran a LittleEndian");
    }
    else
    {
        INFO_LOG("Les imatges del dicomdir mantindran la seva transfer syntax");
    }
}

bool ConvertToDicomdir::getConvertDicomdirImagesToLittleEndian()
{
    return m_convertDicomdirImagesToLittleEndian;
}

bool ConvertToDicomdir::AreValidRequirementsOfFolderContentToCopyToDICOMDIR(QString path)
{
    QDir dir(path);
    QStringList directoryContent = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

    return !directoryContent.contains("DICOMDIR", Qt::CaseInsensitive) &&
            !directoryContent.contains("DICOM", Qt::CaseInsensitive);
}

Status ConvertToDicomdir::copyStudiesToDicomdirPath(QList<Study*> studyList)
{
    StudyToConvert studyToConvert;
    QString m_OldPatientId;
    QString patientNameDir;
    QDir patientDir;
    Status state;
    QChar fillChar = '0';
    Study *study;

    m_patient = 0;

    /// We group studies1 by patient, as we have the list sorted by patientId
    while (!m_studiesToConvert.isEmpty())
    {
        studyToConvert = m_studiesToConvert.takeFirst();

        /// \ TODO: It gets sloppy to go fast. Actually the method would have to be redone for it to work with a Study list and not
        /// keep two lists, one of uids and the other of studio's. Now that we create studyList from m_studiesToConvert, we take advantage
        /// that we have the same order.
        study = studyList.takeFirst();

        if (study->getInstanceUID() != studyToConvert.studyUID)
        {
            state.setStatus("CopyStudiesToDicomdirPath bug does not work, there is a different order between studyList and m_studiesToConvert",
                            false, -1);
            break;
        }

        //If the patient is different we create a new PAtient directory
        if (m_OldPatientId != studyToConvert.patientId)
        {
            patientNameDir = QString("/PAT%1").arg(m_patient, 5, 10, fillChar);
            m_dicomdirPatientPath = m_dicomDirPath + "/DICOM/" + patientNameDir;
            patientDir.mkpath(m_dicomdirPatientPath);
            m_patient++;
            m_study = 0;
            /// We create a list with the directories created, in case there is an error deleting them
            m_patientDirectories.push_back(m_dicomdirPatientPath);
        }

        state = copyStudyToDicomdirPath(study);

        delete study;
        if (!state.good())
        {
            break;
        }
        m_OldPatientId = studyToConvert.patientId;
    }

    return state;
}

Status ConvertToDicomdir::copyStudyToDicomdirPath(Study *study)
{
    ///We create the studio directory where a selected studio will be moved to convert to dicomdir
    QDir studyDir;
    QChar fillChar = '0';
    QString studyName = QString("/STU%1").arg(m_study, 5, 10, fillChar);
    Status state;

    m_study++;
    m_series = 0;

    /// We create the directory where the study will be saved in DicomDir format
    m_dicomDirStudyPath = m_dicomdirPatientPath + studyName;
    studyDir.mkdir(m_dicomDirStudyPath);

    /// For each series of the study, we create the directory of the series
    foreach (Series *series, study->getSeries())
    {
        if (series->getNumberOfItems() > 0)
        {
            state = copySeriesToDicomdirPath(series);

            if (!state.good())
            {
                break;
            }
        }
    }

    return state;
}

Status ConvertToDicomdir::copySeriesToDicomdirPath(Series *series)
{
    QDir seriesDir;
    QChar fillChar = '0';
    /// We create the name of the series directory, the format is SERXXXXX, where XXXXX is the serial number within the studio
    QString seriesName = QString("/SER%1").arg(m_series, 5, 10, fillChar);
    Status state;

    m_series++;
    ///We create the directory where the series will be saved in DicomDir format
    m_dicomDirSeriesPath = m_dicomDirStudyPath + seriesName;
    seriesDir.mkdir(m_dicomDirSeriesPath);

    state = copyImages(series->getImages());

    return state;
}

Status ConvertToDicomdir::copyImages(QList<Image*> images)
{
    Status state;
    m_currentItemNumber = 0;
    // HACK to avoid cases where they are images from a multiframe
    // that we copy a file more than once
    QString lastPath;
    foreach (Image *imageToCopy, images)
    {
        if (lastPath != imageToCopy->getPath())
        {
            lastPath = imageToCopy->getPath();
            
            m_currentItemNumber++;
            state = copyImageToDicomdirPath(imageToCopy);
            
            ///The progress bar moves forward
            m_progress->setValue(m_progress->value() + 1);
            m_progress->repaint();
            
            if (!state.good())
            {
                break;
            }
        }
    }

    return state;
}

Status ConvertToDicomdir::copyImageToDicomdirPath(Image *image)
{
    /// We create the name of the image file, the format is IMGXXXXX, where XXXXX is the image number within the series
    QString imageOutputPath = getCurrentItemOutputPath();
    Status state;

    if (getConvertDicomdirImagesToLittleEndian())
    {
        /// We convert the image to littleEndian, required by DICOM regulations and save it in the destination directory
        state = ConvertDicomToLittleEndian().convert(image->getPath(), imageOutputPath);

        if (m_anonymizeDICOMDIR && state.good())
        {
            anonymizeFile(imageOutputPath, imageOutputPath, state, true);
        }
    }
    else
    {
        /// If we have to anonymize the file what we do is that instead of copying it and then anonymizing it,
        /// it is to indicate to the method of anonymizing
        /// save the file where it should have been copied to create the DICOMDIR, thus creating DICOMDIR for images
        /// not having to convert to LittleEndian is faster.
        if (m_anonymizeDICOMDIR)
        {
            anonymizeFile(image->getPath(), imageOutputPath, state, false);
        }
        else
        {
            copyFileToDICOMDIRDestination(image->getPath(), imageOutputPath, state);
        }
    }

    return state;
}

QString ConvertToDicomdir::getDICOMDIROutputFilenamePrefix() const
{
    return "IMG";
}

QString ConvertToDicomdir::getCurrentItemOutputPath()
{
    return m_dicomDirSeriesPath + QString("/%1%2").arg(getDICOMDIROutputFilenamePrefix()).arg(m_currentItemNumber, 5, 10, QChar('0'));
}

void ConvertToDicomdir::copyFileToDICOMDIRDestination(const QString &sourceFile, const QString &destinationFile, Status &status)
{
    if (QFile::copy(sourceFile, destinationFile))
    {
        status.setStatus("", true, 0);
    }
    else
    {
        QString errorString = QString("Unable to copy file from %1 to %2").arg(sourceFile, destinationFile);
        ERROR_LOG(qPrintable(errorString));
        status.setStatus(errorString, false, 3001);
    }
}

void ConvertToDicomdir::anonymizeFile(const QString &sourceFile, const QString &destinationFile, Status &status, bool isLittleEndian)
{
    if (m_DICOMAnonymizer->anonymizeDICOMFile(sourceFile, destinationFile))
    {
        status.setStatus("", true, 0);
    }
    else
    {
        QString statusMessage;
        if (isLittleEndian)
        {
            statusMessage = QString("Unable to anonymize Little Endian Image %1").arg(sourceFile);
        }
        else
        {
            statusMessage = QString("Unable to anonymize file %1 to %2").arg(sourceFile, destinationFile);
        }
        
        status.setStatus(statusMessage, false, 3003);
    }
}

void ConvertToDicomdir::createReadmeTxt()
{
    Settings settings;
    QString readmeFilePath = m_dicomDirPath + "/README.TXT";
    QFile file(readmeFilePath);

    if (file.exists())
    {
        INFO_LOG("A readme.txt file has already been found in DICOMDIR from the contents of the folder to be copied");
        /// If the file already exists it means that we have copied it from the content to the folder that was copied when creating a DICOMDIR
        /// in this case we keep the existing Readme.txt and we do not generate ours
        return;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream out(&file);
    out << "The DICOMDIR has been generated with " << ApplicationNameString << " Version " << StarviewerVersionString << "\n";
    out << "E-mail contact : " << OrganizationEmailString << "\n";
    out << ApplicationNameString << " is not responsible for DICOMDIR content." << "\n\n";

    if (!settings.getValue(InputOutputSettings::InstitutionName).toString().isEmpty())
    {
        out << "The DICOMDIR has been created by: " << settings.getValue(InputOutputSettings::InstitutionName).toString() << "\n";

        if (!settings.getValue(InputOutputSettings::InstitutionAddress).toString().isEmpty())
        {
            out << settings.getValue(InputOutputSettings::InstitutionAddress).toString() << "\n";
        }
        if (!settings.getValue(InputOutputSettings::InstitutionZipCode).toString().isEmpty())
        {
            out << settings.getValue(InputOutputSettings::InstitutionZipCode).toString() << " - ";
        }
        if (!settings.getValue(InputOutputSettings::InstitutionTown).toString().isEmpty())
        {
            out << settings.getValue(InputOutputSettings::InstitutionTown).toString() << "\n";
        }
        if (!settings.getValue(InputOutputSettings::InstitutionCountry).toString().isEmpty())
        {
            out << settings.getValue(InputOutputSettings::InstitutionCountry).toString() << "\n";
        }
        if (!settings.getValue(InputOutputSettings::InstitutionPhoneNumber).toString().isEmpty())
        {
            out << settings.getValue(InputOutputSettings::InstitutionPhoneNumber).toString() << "\n";
        }
        if (!settings.getValue(InputOutputSettings::InstitutionEmail).toString().isEmpty())
        {
            out << settings.getValue(InputOutputSettings::InstitutionEmail).toString() << "\n";
        }
    }

    file.close();
}

bool ConvertToDicomdir::copyFolderContentToDICOMDIR()
{
    // TODO:This code snippet should be copied to a DICOMDIR Manager should not be here in the UI
    QString folderToCopyPath = Settings().getValue(InputOutputSettings::DICOMDIRFolderPathToCopy).toString();
    bool ok = true;

    INFO_LOG("The contents of the folder will be copied to DICOMDIR" + folderToCopyPath);

    if (!DirectoryUtilities::copyDirectory(folderToCopyPath, m_dicomDirPath))
    {
        ERROR_LOG(QString("Could not copy DICOM viewer% 1 to DICOMDIR% 2").arg(folderToCopyPath, m_dicomDirPath));
        ok = false;
    }

    return ok;
}

}
