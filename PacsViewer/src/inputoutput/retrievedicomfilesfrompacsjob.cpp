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

#include "retrievedicomfilesfrompacsjob.h"

#include <QtGlobal>
#include <QThread>

#include "logging.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "starviewerapplication.h"
#include "retrievedicomfilesfrompacs.h"
#include "dicommask.h"
#include "localdatabasemanager.h"
#include "patientfiller.h"
#include "directoryutilities.h"
#include "harddiskinformation.h"
#include "inputoutputsettings.h"
#include "dicomtagreader.h"
#include "portinuse.h"
#include "dicomsource.h"
#include "usermessage.h"

namespace udg {

RetrieveDICOMFilesFromPACSJob::RetrieveDICOMFilesFromPACSJob(PacsDevice pacsDevice,
                                                             RetrievePriorityJob retrievePriorityJob, Study *studyToRetrieveDICOMFiles,
                                                             const QString &seriesInstanceUIDToRetrieve, const QString &sopInstanceUIDToRetrieve)
    : PACSJob(pacsDevice)
{
    Q_ASSERT(studyToRetrieveDICOMFiles);
    Q_ASSERT(studyToRetrieveDICOMFiles->getParentPatient());

    m_retrieveDICOMFilesFromPACS = new RetrieveDICOMFilesFromPACS(getPacsDevice());
    m_studyToRetrieveDICOMFiles = copyBasicStudyInformation(studyToRetrieveDICOMFiles);
    m_seriesInstanceUIDToRetrieve = seriesInstanceUIDToRetrieve;
    m_SOPInstanceUIDToRetrieve = sopInstanceUIDToRetrieve;
    m_retrievePriorityJob = retrievePriorityJob;
}

RetrieveDICOMFilesFromPACSJob::~RetrieveDICOMFilesFromPACSJob()
{
    delete m_studyToRetrieveDICOMFiles;
    delete m_retrieveDICOMFilesFromPACS;
}

PACSJob::PACSJobType RetrieveDICOMFilesFromPACSJob::getPACSJobType()
{
    return PACSJob::RetrieveDICOMFilesFromPACSJobType;
}

Study* RetrieveDICOMFilesFromPACSJob::getStudyToRetrieveDICOMFiles()
{
    return m_studyToRetrieveDICOMFiles;
}

void RetrieveDICOMFilesFromPACSJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread)
{
    Q_UNUSED(self)
    Q_UNUSED(thread)

    Settings settings;
    // TODO: Is this the place for this post? it might not be better to do it in RetrieveDICOMFilesFromPACS
    INFO_LOG(QString("Iniciant descarrega del PACS %1, IP: %2, Port: %3, AE Title Local: %4 Port local: %5, "
                     "the study UID: %6, series UID: %7, SOP Instance UID:%8")
             .arg(getPacsDevice().getAETitle(), getPacsDevice().getAddress(), QString::number(getPacsDevice().getQueryRetrieveServicePort()))
             .arg(settings.getValue(InputOutputSettings::LocalAETitle).toString(),
                  settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toString())
             .arg(m_studyToRetrieveDICOMFiles->getInstanceUID(), m_seriesInstanceUIDToRetrieve, m_SOPInstanceUIDToRetrieve));

    m_retrievedSeriesInstanceUIDSet.clear();

    m_retrieveRequestStatus = thereIsAvailableSpaceOnHardDisk();

    if (m_retrieveRequestStatus != PACSRequestStatus::RetrieveOk)
    {
        return;
    }

    int localPort = settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toInt();

    if (PortInUse().isPortInUse(localPort))
    {
        m_retrieveRequestStatus = PACSRequestStatus::RetrieveIncomingDICOMConnectionsPortInUse;
        ERROR_LOG("The port " + QString::number(localPort) + " for incoming PACS connections, it is in use, the study cannot be downloaded");
    }
    else
    {
        PatientFiller patientFiller(getDICOMSourceRetrieveFiles());
        QThread fillersThread;
        patientFiller.moveToThread(&fillersThread);
        LocalDatabaseManager localDatabaseManager;

        /// Must be specified as DirectConnection, because otherwise this
        /// signal it to the person who created the Job, which is the interface, therefore
        /// would not be attended to until the interface is free,
        /// causing incorrect behaviors
        connect(m_retrieveDICOMFilesFromPACS, SIGNAL(DICOMFileRetrieved(DICOMTagReader*, int)), this, SLOT(DICOMFileRetrieved(DICOMTagReader*, int)),
                Qt::DirectConnection);
        ///We connect to the patientFiller signals to process the downloaded files
        connect(this, &RetrieveDICOMFilesFromPACSJob::DICOMTagReaderReadyForProcess, &patientFiller, &PatientFiller::processDICOMFile);
        connect(this, SIGNAL(DICOMFilesRetrieveFinished()), &patientFiller, SLOT(finishDICOMFilesProcess()));
        /// Connection between the processing of DICOM files and the insertion in the BD,
        /// it is important that this signal is a Qt: DirectConnection so that the
        /// is processed by the child threads, thus the thread
        /// download that is waiting in fillersThread.wait () when it exits
        /// hence because the fillers are already finished the patient has already been inserted into the database.
        connect(&patientFiller, SIGNAL(patientProcessed(Patient*)), &localDatabaseManager, SLOT(save(Patient*)), Qt::DirectConnection);
        ///Connections to end threads
        connect(&patientFiller, SIGNAL(patientProcessed(Patient*)), &fillersThread, SLOT(quit()), Qt::DirectConnection);

        localDatabaseManager.setStudyBeingRetrieved(m_studyToRetrieveDICOMFiles->getInstanceUID());
        fillersThread.start();

        m_retrieveRequestStatus = m_retrieveDICOMFilesFromPACS->retrieve(m_studyToRetrieveDICOMFiles->getInstanceUID(), m_seriesInstanceUIDToRetrieve,
                                                                         m_SOPInstanceUIDToRetrieve);

        if ((m_retrieveRequestStatus == PACSRequestStatus::RetrieveOk || m_retrieveRequestStatus == PACSRequestStatus::RetrieveSomeDICOMFilesFailed) &&
                !this->isAbortRequested())
        {
            INFO_LOG(QString("PACS %2 study %1 has finished downloading, %3 files have been downloaded")
                     .arg(m_studyToRetrieveDICOMFiles->getInstanceUID(), getPacsDevice().getAETitle())
                     .arg(m_retrieveDICOMFilesFromPACS->getNumberOfDICOMFilesRetrieved()));

            //We indicate that the download process is complete
            emit DICOMFilesRetrieveFinished();

            // We expect the processing and insertion into the database to complete
            fillersThread.wait();

            if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
            {
                if (localDatabaseManager.getLastError() == LocalDatabaseManager::PatientInconsistent)
                {
                    /// Could not insert patient, because patientfiller
                    /// could not fill in the patient information correctly
                    m_retrieveRequestStatus = PACSRequestStatus::RetrievePatientInconsistent;
                }
                else
                {
                    m_retrieveRequestStatus = PACSRequestStatus::RetrieveDatabaseError;
                }
            }
        }
        else
        {
            fillersThread.quit();
            /// We hope that the thread ends, because from what is interpreted from the documentation,
            /// it seems that quitting the thread is not done until it returns
            /// in eventLoop, this causes for example in the cases that we have canceled
            /// lat the download of a study, if we do not wait for the thread to be dead
            /// we can delete images that the fillers are processing on that one
            /// moment while they're still running and ask for the Starviewer, because
            /// the Slot quit of the thread has not been addressed, so we hope this
            /// is dead deleting downloaded images.
            fillersThread.wait();
            deleteRetrievedDICOMFilesIfStudyNotExistInDatabase();
        }

        localDatabaseManager.setNoStudyBeingRetrieved();
    }
}

void RetrieveDICOMFilesFromPACSJob::requestCancelJob()
{
    INFO_LOG(QString("PACS Study UID %1 .AETitle %1 Image Download Job Canceled")
             .arg(getStudyToRetrieveDICOMFiles()->getInstanceUID(),
                  getPacsDevice().getAETitle()));
    m_retrieveDICOMFilesFromPACS->requestCancel();
}

PACSRequestStatus::RetrieveRequestStatus RetrieveDICOMFilesFromPACSJob::getStatus()
{
    return m_retrieveRequestStatus;
}

void RetrieveDICOMFilesFromPACSJob::DICOMFileRetrieved(DICOMTagReader *dicomTagReader, int numberOfImagesRetrieved)
{
    emit DICOMFileRetrieved(m_selfPointer.toStrongRef(), numberOfImagesRetrieved);

    /// We update the number of series processed if it reaches us
    /// a new image that belongs to a series not downloaded so far
    QString seriesInstancedUIDRetrievedImage = dicomTagReader->getValueAttributeAsQString(DICOMSeriesInstanceUID);
    if (!m_retrievedSeriesInstanceUIDSet.contains(seriesInstancedUIDRetrievedImage))
    {
        m_retrievedSeriesInstanceUIDSet.insert(seriesInstancedUIDRetrievedImage);
        emit DICOMSeriesRetrieved(m_selfPointer.toStrongRef(), m_retrievedSeriesInstanceUIDSet.count());
    }

    /// We make an issue indicating that dicomTagReader is ready to
    /// be processed by PatientFiller's processDICOMFile slot, we can't make a connection
    /// directly between the DICOMFileRetrieved signal of RetrieveDICOMFileFromPACS
    /// and processDICOMFile of PatientFiller, because we could find
    /// that when in this method it checks if we have downloaded a new series
    /// that DICOMTagReader no longer has value, so we first capture the signal from
    /// RetrieveDICOMFileFromPACS we check if it is a new series
    /// that is downloaded and then we issue it for PatientFiller to process the DICOMTagReader

    emit DICOMTagReaderReadyForProcess(dicomTagReader);
}

int RetrieveDICOMFilesFromPACSJob::priority() const
{
    return m_retrievePriorityJob;
}

PACSRequestStatus::RetrieveRequestStatus RetrieveDICOMFilesFromPACSJob::thereIsAvailableSpaceOnHardDisk()
{
    LocalDatabaseManager localDatabaseManager;
    // TODO:This signal should not be made from this class but from a CacheManager, but as of now it is not yet implemented
    // temporarily emit the signal from here * /
    connect(&localDatabaseManager, SIGNAL(studyWillBeDeleted(QString)), SIGNAL(studyFromCacheWillBeDeleted(QString)));

    if (!localDatabaseManager.thereIsAvailableSpaceOnHardDisk())
    {
        // If there is not enough space we emit this signal
        if (localDatabaseManager.getLastError() == LocalDatabaseManager::Ok)
        {
            return PACSRequestStatus::RetrieveNoEnoughSpace;
        }
        else
        {
            return PACSRequestStatus::RetrieveErrorFreeingSpace;
        }
    }

    return PACSRequestStatus::RetrieveOk;
}

void RetrieveDICOMFilesFromPACSJob::deleteRetrievedDICOMFilesIfStudyNotExistInDatabase()
{
    /// We check if the study is inserted in the database, if so
    /// means that part or all of the study had previously been downloaded,
    /// as we already have other elements of this study inserted in the
    /// database we do not delete the study directory
    if (!LocalDatabaseManager().studyExists(m_studyToRetrieveDICOMFiles->getInstanceUID()))
    {
        /// If the study does not exist in the database we delete it
        /// the content of the directory, in principle according to the DICO regulations; if we receive a status of
        /// type error by MoveSCP would indicate that no dicom object has been successfully downloaded

        INFO_LOG("The study " + m_studyToRetrieveDICOMFiles->getInstanceUID() + " does not exist in the database, we delete the contents of your directory.");
        DirectoryUtilities().deleteDirectory(LocalDatabaseManager().getStudyPath(m_studyToRetrieveDICOMFiles->getInstanceUID()), true);
    }
    else
    {
        INFO_LOG("The study " + m_studyToRetrieveDICOMFiles->getInstanceUID() + " exists in the database, we do not delete the contents of your directory.");
    }
}

/// TODO:We centralize the construction of error messages because
/// all interfaces can use one, and there is no need to have error handling
/// duplicate or translations, but is the best place to put this code here?
QString RetrieveDICOMFilesFromPACSJob::getStatusDescription()
{
    QString message;
    QString errorDetails = "\n\n" + tr("Details:") + "\n" + m_retrieveDICOMFilesFromPACS->getResponseStatus().toString();
    QString studyID = getStudyToRetrieveDICOMFiles()->getID();
    QString patientName = getStudyToRetrieveDICOMFiles()->getParentPatient()->getFullName();
    QString pacsAETitle = getPacsDevice().getAETitle();
    Settings settings;

    switch (getStatus())
    {
    case PACSRequestStatus::RetrieveOk:
        message = tr("Images from study %1 of patient %2 have been successfully retrieved from PACS %3.").arg(studyID, patientName, pacsAETitle);
        break;
    case PACSRequestStatus::RetrieveCancelled:
        message = tr("Retrieval of the images from study %1 of patient %2 from PACS %3 has been canceled.").arg(studyID, patientName, pacsAETitle);
        break;
    case PACSRequestStatus::RetrieveCanNotConnectToPACS:
        message = tr("Unable to connect to PACS %1 to retrieve images from study %2 of patient %3.")
                .arg(pacsAETitle, studyID, patientName);
        message += "\n\n";
        message += tr("Make sure your computer is connected to the network and the PACS parameters are correct.");
        message += "\n";
        message += UserMessage::getProblemPersistsAdvice();
        break;
    case PACSRequestStatus::RetrieveNoEnoughSpace:
    {
        Settings settings;
        HardDiskInformation hardDiskInformation;
        quint64 freeSpaceInHardDisk = hardDiskInformation.getNumberOfFreeMBytes(LocalDatabaseManager::getCachePath());
        quint64 minimumSpaceRequired = quint64(settings.getValue(InputOutputSettings::MinimumFreeGigaBytesForCache).toULongLong() * 1024);
        message = tr("There is not enough space to retrieve images from study %1 of patient %2, please free space or change your local "
                     "database settings.")
                .arg(studyID, patientName);
        message += "\n\n";
        message += tr("- Available disk space: %1 MB.").arg(freeSpaceInHardDisk);
        message += "\n";
        message += tr("- Minimum disk space required to retrieve studies: %1 MB.").arg(minimumSpaceRequired);
    }
        break;
    case PACSRequestStatus::RetrieveErrorFreeingSpace:
        message = tr("An error occurred while freeing space on hard disk, images from study %1 of patient %2 won't be retrieved.").arg(studyID, patientName);
        message += "\n\n";
        message += UserMessage::getCloseWindowsAndTryAgainAdvice();
        message += "\n";
        message += UserMessage::getProblemPersistsAdvice();
        break;
    case PACSRequestStatus::RetrieveDatabaseError:
        message = tr("Cannot retrieve images from study %1 of patient %2 because a database error occurred.").arg(studyID, patientName);
        message += "\n\n";
        message += UserMessage::getCloseWindowsAndTryAgainAdvice();
        message += "\n";
        message += UserMessage::getProblemPersistsAdvice();
        break;
    case PACSRequestStatus::RetrievePatientInconsistent:
        message = tr("Cannot retrieve images from study %1 of patient %2 from PACS %3. Unable to correctly read data from images.")
                .arg(studyID, patientName, pacsAETitle);
        message += "\n\n";
        message += tr("The study may be corrupted, if it is not corrupted please contact with %1 team.").arg(ApplicationNameString);
        break;
    case PACSRequestStatus::RetrieveDestinationAETileUnknown:
        message = tr("Cannot retrieve images from study %1 of patient %2 because PACS %3 does not recognize your computer's AE Title %4.")
                .arg(studyID, patientName, pacsAETitle, settings.getValue(InputOutputSettings::LocalAETitle).toString());
        message += "\n\n";
        message += tr("Contact with an administrator to register your computer to the PACS.");
        message += errorDetails;
        break;
    case PACSRequestStatus::RetrieveUnknowStatus:
        message = tr("Cannot retrieve images from study %1 of patient %2 due to an unknown error of PACS %3.")
                .arg(studyID, patientName, pacsAETitle);
        message += "\n\n";
        message += tr("The cause of the error may be that the requested images are corrupted. Please contact with a PACS administrator.");
        message += errorDetails;
        break;
    case PACSRequestStatus::RetrieveFailureOrRefused:
        message = tr("Cannot retrieve images from study %1 of patient %2 due to an error of PACS %3.")
                .arg(studyID, patientName, pacsAETitle);
        message += "\n\n";
        message += tr("The cause of the error may be that the requested images are corrupted or the incoming connections port in PACS configuration "
                      "is not correct.");
        message += errorDetails;
        break;
    case PACSRequestStatus::RetrieveIncomingDICOMConnectionsPortInUse:
        message = tr("Cannot retrieve images from study %1 of patient %2 because port %3 for incoming connections from PACS is already in use "
                     "by another application.")
                .arg(studyID, patientName, settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toString());
        break;
    case PACSRequestStatus::RetrieveSomeDICOMFilesFailed:
        message = tr("Unable to retrieve some images from study %1 of patient %2 from PACS %3. Maybe those images are missing or corrupted in PACS.")
                .arg(studyID, patientName, pacsAETitle);
        message += "\n";
        message += errorDetails;
        break;
    default:
        message = tr("Cannot retrieve images from study %1 of patient %2 from PACS %3 due to an unknown error.")
                .arg(ApplicationNameString, studyID, patientName, pacsAETitle);
        message += "\n\n";
        message += UserMessage::getCloseWindowsAndTryAgainAdvice();
        message += "\n";
        message += UserMessage::getProblemPersistsAdvice();
        message += errorDetails;
        break;
    }

    return message;
}

Study* RetrieveDICOMFilesFromPACSJob::copyBasicStudyInformation(Study *studyToCopy)
{
    Study *copiedStudy = new Study();
    Patient *copiedPatient = new Patient();

    copiedPatient->setID(studyToCopy->getParentPatient()->getID());
    copiedPatient->setFullName(studyToCopy->getParentPatient()->getFullName());

    copiedStudy->setParentPatient(copiedPatient);
    copiedStudy->setInstanceUID(studyToCopy->getInstanceUID());
    copiedStudy->setID(studyToCopy->getID());
    copiedStudy->setDateTime(studyToCopy->getDateAsString(), studyToCopy->getTimeAsString());
    copiedStudy->setDescription(studyToCopy->getDescription());
    copiedStudy->setAccessionNumber(studyToCopy->getAccessionNumber());

    foreach(QString modality, studyToCopy->getModalities())
    {
        copiedStudy->addModality(modality);
    }

    copiedStudy->setDICOMSource(studyToCopy->getDICOMSource());

    return copiedStudy;
}

DICOMSource RetrieveDICOMFilesFromPACSJob::getDICOMSourceRetrieveFiles()
{
    DICOMSource filesDICOMSource;
    filesDICOMSource.addRetrievePACS(getPacsDevice());

    return filesDICOMSource;
}

};
