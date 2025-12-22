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

#include "dicomdirimporter.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QThread>

#include "status.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "dicommask.h"
#include "logging.h"
#include "patientfiller.h"
#include "dicomtagreader.h"
#include "localdatabasemanager.h"
#include "localdatabasemanager.h"
#include "directoryutilities.h"
#include "patient.h"

namespace udg {

void DICOMDIRImporter::import(QString dicomdirPath, QString studyUID, QString seriesUID, QString sopInstanceUID)
{
    m_lastError = Ok;
    LocalDatabaseManager localDatabaseManager;
    PatientFiller patientFiller;
    QThread fillersThread;

    /// We check if there is enough free space to import the study
    if (!localDatabaseManager.thereIsAvailableSpaceOnHardDisk())
    {
        if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
        {
            m_lastError = ErrorFreeingSpace;
        }
        else
        {
            m_lastError = NoEnoughSpace;
        }

        return;
    }

    /// We open the dicomdir
    Status state = m_readDicomdir.open(QDir::toNativeSeparators(dicomdirPath));

    if (!state.good())
    {
        m_lastError = ErrorOpeningDicomdir;
        return;
    }

    m_qprogressDialog = new QProgressDialog("", "", 0, 0);
    m_qprogressDialog->setModal(true);
    m_qprogressDialog->setCancelButton(0);
    m_qprogressDialog->setValue(1);
    m_qprogressDialog->setMinimumDuration(0);

    patientFiller.moveToThread(&fillersThread);

    ///We create the necessary connections to import dicomdirs
    createConnections(&patientFiller, &localDatabaseManager, &fillersThread);

    fillersThread.start();

    importStudy(studyUID, seriesUID, sopInstanceUID);

    if (getLastError() == Ok)
    {
        emit importFinished();
    }
    else
    {
        emit importAborted();
    }

    /// We expect the processing and insertion into the database to complete
    fillersThread.wait();

    /// We check that the new study has been successfully imported
    if (getLastError() != Ok)
    {
        /// If there was an error we deleted the imported files from the local cache studio
        deleteFailedImportedStudy(studyUID);
    }
    else
    {
        ///We check that the new study has been successfully inserted in the database
        if (localDatabaseManager.getLastError() == LocalDatabaseManager::Ok)
        {
            INFO_LOG("Estudi " + studyUID + " importat");
            m_lastError = Ok;
        }
        else
        {
            if (localDatabaseManager.getLastError() == LocalDatabaseManager::PatientInconsistent)
            {
                /// No s'ha pogut inserir el patient, perquè patientfiller
                /// no ha pogut emplenar l'informació de patient correctament
                m_lastError = PatientInconsistent;
            }
            else
            {
                m_lastError = DatabaseError;
            }

            ///If there was an error we deleted the imported files from the local cache studio
            deleteFailedImportedStudy(studyUID);
        }
    }

    m_qprogressDialog->close();
}

void DICOMDIRImporter::importStudy(QString studyUID, QString seriesUID, QString sopInstanceUID)
{
    DicomMask mask;
    QList<Patient*> patientStudyListToImport;
    QString studyPath = LocalDatabaseManager::getCachePath() + studyUID + "/";;

    QDir().mkdir(studyPath);

    mask.setStudyInstanceUID(studyUID);

    m_readDicomdir.readStudies(patientStudyListToImport, mask);

    ///We check that studies have been found to import
    if (!patientStudyListToImport.isEmpty())
    {
        QList<Series*> seriesListToImport;

        m_qprogressDialog->setLabelText(getDescriptionForQProgressDialog(studyUID, seriesUID, sopInstanceUID));

        m_readDicomdir.readSeries(studyUID, seriesUID, seriesListToImport);

        if (seriesListToImport.isEmpty())
        {
            ERROR_LOG("No series were found for the study");
            m_lastError = DicomdirInconsistent;
            return;
        }

        foreach (Series *seriesToImport, seriesListToImport)
        {
            importSeries(studyUID, seriesToImport->getInstanceUID(), sopInstanceUID);
            if (getLastError() != Ok)
            {
                break;
            }
        }
    }
    else
    {
        ERROR_LOG("No study found to import");
        m_lastError = DicomdirInconsistent;
    }
}

void DICOMDIRImporter::importSeries(QString studyUID, QString seriesUID, QString sopInstanceUID)
{
    QList<Image*> imageListToImport;
    QString seriesPath = LocalDatabaseManager::getCachePath() + studyUID + "/" + seriesUID;

    QDir().mkdir(seriesPath);

    m_readDicomdir.readImages(seriesUID, sopInstanceUID, imageListToImport);

    if (imageListToImport.isEmpty())
    {
        ERROR_LOG("No images found for the series");
        m_lastError = DicomdirInconsistent;
        return;
    }

    foreach (Image *imageToImport, imageListToImport)
    {
        importImage(imageToImport, seriesPath);
        if (getLastError() != Ok)
        {
            break;
        }
    }
}

void DICOMDIRImporter::importImage(Image *image, QString pathToImportImage)
{
    QString cacheImagePath, dicomdirImagePath = getDicomdirImagePath(image);

    if (dicomdirImagePath.length() == 0)
    {
        m_lastError = DicomdirInconsistent;
        return;
    }

    cacheImagePath = pathToImportImage + "/" + image->getSOPInstanceUID();

    if (!copyDicomdirImageToLocal(dicomdirImagePath, cacheImagePath))
    {
        ///Could not copy check, if the file already exists
        if (QFile::exists(cacheImagePath))
        {
            /// The file already exists and we want to delete it
            if (QFile::remove(cacheImagePath))
            {
                /// We deleted the file that already existed, now we are trying to copy it
                if (!copyDicomdirImageToLocal(dicomdirImagePath, cacheImagePath))
                {
                    ERROR_LOG("The file: <" + dicomdirImagePath + "> could not copy to <" + cacheImagePath +
                              ">,the file already existed on the destination, it was deleted successfully, but at the same time the operation failed to copy it");
                    m_lastError = ErrorCopyingFiles;
                }
            }
            else
            {
                ERROR_LOG("The file: <" + dicomdirImagePath + ">could not copy to<" + cacheImagePath + ">, since the file already exists at the destination, " +
                          "tried to delete local file but failed, you might not have write permissions to the destination directory");
                m_lastError = ErrorCopyingFiles;
            }
        }
        else
        {
            ERROR_LOG("The file: <" + dicomdirImagePath + "> could not copy to <" + cacheImagePath +
                      ">,you may not have permissions on the destination directory");
            m_lastError = ErrorCopyingFiles;
        }
    }
}

bool DICOMDIRImporter::copyDicomdirImageToLocal(QString dicomdirImagePath, QString localImagePath)
{
    if (QFile::copy(dicomdirImagePath, localImagePath))
    {
        // We give permissions in case the file is still read only when coming from a CD
        if (!QFile::setPermissions(localImagePath, QFile::WriteOwner | QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther))
        {
            WARN_LOG("We were unable to change the read / write permissions for the imported file[" + localImagePath + "]");
        }
        // TODO: why do I need to make this DICOMTagReader? Is the dicom tag reader cache still used ????
        DICOMTagReader *dicomTagReader = new DICOMTagReader(localImagePath);
        emit imageImportedToDisk(dicomTagReader);

        m_qprogressDialog->setValue(m_qprogressDialog->value() + 1);

        return true;
    }
    else
    {
        return false;
    }
}

QString DICOMDIRImporter::getDicomdirImagePath(Image *image)
{
    /// We check if the image to be imported exists
    if (QFile::exists(image->getPath()))
    {
        return image->getPath();
    }
    else if (QFile::exists(image->getPath().toLower()))
    {
        /// Default Linux on vfat drives, show filenames that are shortname (8 or less characters) in lower case
        /// because in the dicomdir file the file paths are saved in uppercase, if we make an exist of the file name on
        /// vfat drives fail, so what we do is convert the file name to lowercase
        return image->getPath().toLower();
    }
    else
    {
        ERROR_LOG("DICOMDIR inconsistent: The image [" + image->getPath() + "] no existeix");
        return "";
    }

}

void DICOMDIRImporter::createConnections(PatientFiller *patientFiller, LocalDatabaseManager *localDatabaseManager, QThread *fillersThread)
{
    // Connections between downloading and processing files
    connect(this, &DICOMDIRImporter::imageImportedToDisk, patientFiller, &PatientFiller::processDICOMFile);
    connect(this, SIGNAL(importFinished()), patientFiller, SLOT(finishDICOMFilesProcess()));

    /// Connection between the processing of DICOM files and the insertion in the BD,
    ///  it is important that this signal is a Qt: DirectConnection so that the
    /// it processes the threads of the fillers, this way the download thread that
    /// is waiting in fillersThread.wait () when it leaves
    /// because the filler threads have already ended, the patient has already been inserted into the database.
    connect(patientFiller, SIGNAL(patientProcessed(Patient*)), localDatabaseManager, SLOT(save(Patient*)), Qt::DirectConnection);

    ///Connections to end threads
    connect(patientFiller, SIGNAL(patientProcessed(Patient*)), fillersThread, SLOT(quit()), Qt::DirectConnection);

    /// Abortion connections
    connect(this, SIGNAL(importAborted()), fillersThread, SLOT(quit()), Qt::DirectConnection);
}

void DICOMDIRImporter::deleteFailedImportedStudy(QString studyInstanceUID)
{
    DirectoryUtilities delDirectory;
    LocalDatabaseManager localDatabaseManager;

    INFO_LOG("Images imported from the studio will be deleted from the cache " + studyInstanceUID + " since its import has failed");
    delDirectory.deleteDirectory(localDatabaseManager.getStudyPath(studyInstanceUID), true);
}

DICOMDIRImporter::DICOMDIRImporterError DICOMDIRImporter::getLastError()
{
    return m_lastError;
}

QString DICOMDIRImporter::getDescriptionForQProgressDialog(QString studyInstanceUID, QString seriesInstanceUID, QString SOPInstanceUID)
{
    QList<Patient*> patientToImport;
    QList<Series*> seriesToImport;
    QList<Image*> imageToImport;
    QString description;

    if (!SOPInstanceUID.isEmpty() && !seriesInstanceUID.isEmpty() && studyInstanceUID.isEmpty())
    {
        m_readDicomdir.readImages(seriesInstanceUID, SOPInstanceUID, imageToImport);
    }

    if (!seriesInstanceUID.isEmpty() && !studyInstanceUID.isEmpty())
    {
        m_readDicomdir.readSeries(studyInstanceUID, seriesInstanceUID, seriesToImport);
    }

    if (!studyInstanceUID.isEmpty())
    {
        DicomMask dicomMask;
        dicomMask.setStudyInstanceUID(studyInstanceUID);

        m_readDicomdir.readStudies(patientToImport, dicomMask);
    }

    if (imageToImport.count() == 1 && seriesToImport.count() == 1 && patientToImport.count() == 1)
    {
        description = tr("Importing image %1 of series %2 from study %3, %4").arg(imageToImport.at(0)->getInstanceNumber(), seriesToImport.at(0)->getSeriesNumber(),
                                                                                  patientToImport.at(0)->getStudies().at(0)->getID());
    }
    else if (seriesToImport.count() == 1 && patientToImport.count() == 1)
    {
        description = tr("Importing series %1 of study %2, %3").arg(seriesToImport.at(0)->getSeriesNumber(), patientToImport.at(0)->getStudies().at(0)->getID(),
                                                                    patientToImport.at(0)->getFullName());
    }
    else if (patientToImport.count() == 1)
    {
        description = tr("Importing study %1, %2").arg(patientToImport.at(0)->getStudies().at(0)->getID(), patientToImport.at(0)->getFullName());
    }
    else
    {
        //// It should never happen that I got here
        description = tr("Importing images from DICOMDIR");
    }

    qDeleteAll(imageToImport);
    qDeleteAll(seriesToImport);
    foreach(Patient *patient, patientToImport)
    {
        qDeleteAll(patient->getStudies());
    }

    qDeleteAll(patientToImport);

    return description;
}

}
