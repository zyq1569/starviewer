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

#include "senddicomfilestopacs.h"

#include <diutil.h>
#include <dcfilefo.h>
#include <assoc.h>
#include <dctagkey.h>
#include <dcdeftag.h>

#include <QDir>
#include <QSet>

#include "logging.h"
#include "image.h"
#include "pacsconnection.h"
#include "pacsrequeststatus.h"
#include "inputoutputsettings.h"

namespace udg {

SendDICOMFilesToPACS::SendDICOMFilesToPACS(PacsDevice pacsDevice)
    : DIMSECService()
{
    m_pacs = pacsDevice;
    m_abortIsRequested = false;

    this->setUpAsCStore();
}

PacsDevice SendDICOMFilesToPACS::getPacs()
{
    return m_pacs;
}

PACSRequestStatus::SendRequestStatus SendDICOMFilesToPACS::send(QList<Image*> imageListToSend)
{
    QScopedPointer<PACSConnection> pacsConnection(createPACSConnection(m_pacs));
    // TODO: S'hauria de comprovar que es tracti d'un PACS amb el servei d'store configurat
    if (!pacsConnection->connectToPACS(PACSConnection::SendDICOMFiles))
    {
        ERROR_LOG(" S'ha produit un error al intentar connectar al PACS per fer un send. AE Title: " + m_pacs.getAETitle());
        return PACSRequestStatus::SendCanNotConnectToPACS;
    }

    removeDuplicateFiles(imageListToSend);
    initialitzeDICOMFilesCounters(imageListToSend.count());

    foreach (Image *imageToStore, imageListToSend)
    {
        if (m_abortIsRequested)
        {
            break;
        }

        INFO_LOG(QString("S'enviara al PACS %1 el fitxer %2").arg(m_pacs.getAETitle(), imageToStore->getPath()));
        if (storeSCU(pacsConnection->getConnection(), qPrintable(imageToStore->getPath())))
        {
            emit DICOMFileSent(imageToStore, getNumberOfDICOMFilesSentSuccesfully() + this->getNumberOfDICOMFilesSentWarning());
        }
        else if (m_lastOFCondition == DIMSE_SENDFAILED)
        {
            // Si se'ns retorna un OFCondition == DIMSE_SENDFAILED, indica que s'ha perdut la connexió amb el PACS
            break;
        }
    }

    pacsConnection->disconnect();

    return getStatusStoreSCU();
}

void SendDICOMFilesToPACS::requestCancel()
{
    m_abortIsRequested = true;
    INFO_LOG("Ens han demanat cancel·lar l'enviament dels fitxers al PACS");
}

PACSConnection* SendDICOMFilesToPACS::createPACSConnection(const PacsDevice &pacsDevice) const
{
    return new PACSConnection(pacsDevice);
}

void SendDICOMFilesToPACS::removeDuplicateFiles(QList<Image*> &imageList) const
{
    QSet<QString> paths;
    QMutableListIterator<Image*> it(imageList);

    while (it.hasNext())
    {
        Image *image = it.next();

        if (paths.contains(image->getPath()))
        {
            it.remove();
        }
        else
        {
            paths.insert(image->getPath());
        }
    }
}

void SendDICOMFilesToPACS::initialitzeDICOMFilesCounters(int numberOfDICOMFilesToSend)
{
    // Inicialitzem els comptadors
    m_numberOfDICOMFilesSentSuccessfully = 0;
    m_numberOfDICOMFilesSentWithWarning = 0;
    m_numberOfDICOMFilesToSend = numberOfDICOMFilesToSend;
}

// This function will read all the information from the given file,
// figure out a corresponding presentation context which will be used
// to transmit the information over the network to the SCP, and it
// will finally initiate the transmission of all data to the SCP.
//
// Parameters:
//   association - [in] The associationiation (network connection to another DICOM application).
//   filepathToStore - [in] Name of the file which shall be processed.
bool SendDICOMFilesToPACS::storeSCU(T_ASC_Association *association, QString filepathToStore)
{
    DIC_US msgId = association->nextMsgID++;
    T_ASC_PresentationContextID presentationContextID;
    T_DIMSE_C_StoreRQ request;
    T_DIMSE_C_StoreRSP response;
    DIC_UI sopClass;
    DIC_UI sopInstance;
    DcmDataset *statusDetail = NULL;
    DcmFileFormat dcmff;

    m_lastOFCondition = dcmff.loadFile(qPrintable(QDir::toNativeSeparators(filepathToStore)));

    // Figure out if an error occured while the file was read
    if (m_lastOFCondition.bad())
    {
        ERROR_LOG("Could not open file " + filepathToStore);
        return false;
    }
    // Figure out which SOP class and SOP instance is encapsulated in the file
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER  < 365
    if (!DU_findSOPClassAndInstanceInDataSet(dcmff.getDataset(), sopClass, sopInstance, OFFalse))
#else if  PACKAGE_VERSION_NUMBER == 365
    if (!DU_findSOPClassAndInstanceInDataSet(dcmff.getDataset(), sopClass,sizeof(sopClass), sopInstance, sizeof(sopInstance), OFFalse))
#endif
#endif
    {
        ERROR_LOG("Could not get SOPClass and SOPInstance from file " + filepathToStore);
        return false;
    }

    // Figure out which of the accepted presentation contexts should be used
    DcmXfer filexfer(dcmff.getDataset()->getOriginalXfer());

    // Busquem dels presentationContextID que hem establert al connectar quin és el que hem d'utilitzar per transferir aquesta imatge
    if (filexfer.getXfer() != EXS_Unknown)
    {
        presentationContextID = ASC_findAcceptedPresentationContextID(association, sopClass, filexfer.getXferID());
    }
    else
    {
        presentationContextID = ASC_findAcceptedPresentationContextID(association, sopClass);
    }

    if (presentationContextID == 0)
    {
        //We did not find any valid presentation contexts that we configured in the pacsserver.cpp connection
        const char *modalityName = dcmSOPClassUIDToModality(sopClass);

        if (!modalityName)
        {
            modalityName = dcmFindNameOfUID(sopClass);
        }

        if (!modalityName)
        {
            modalityName = "unknown SOP class";
        }

        ERROR_LOG("No valid presentation context was found on the connection for the mode : " + QString(modalityName)
                  + " amb la SOPClass " + QString(sopClass) + " pel fitxer " + filepathToStore);

        return false;
    }
    else
    {
        // Prepare the transmission of data
        bzero((char*)&request, sizeof(request));
        request.MessageID = msgId;
        strcpy(request.AffectedSOPClassUID, sopClass);
        strcpy(request.AffectedSOPInstanceUID, sopInstance);
        request.DataSetType = DIMSE_DATASET_PRESENT;
        request.Priority = DIMSE_PRIORITY_LOW;

        m_lastOFCondition = DIMSE_storeUser(association, presentationContextID, &request, NULL /*imageFileName*/, dcmff.getDataset(),
                                            NULL /*progressCallback*/, NULL /*callbackData */, DIMSE_NONBLOCKING,
                                            Settings().getValue(InputOutputSettings::PACSConnectionTimeout).toInt(), &response, &statusDetail,
                                            NULL /*check for cancel parameters*/, OFStandard::getFileSize(qPrintable(filepathToStore)));

        if (m_lastOFCondition.bad())
        {
            ERROR_LOG("There was an error storing the image" + filepathToStore + ", error description" + QString(m_lastOFCondition.text()));
        }

        processResponseFromStoreSCP(response.DimseStatus, filepathToStore);
        processServiceClassProviderResponseStatus(response.DimseStatus, statusDetail);

        if (statusDetail != NULL)
        {
            delete statusDetail;
        }

        return m_lastOFCondition.good() && response.DimseStatus == STATUS_Success;
    }
}

void SendDICOMFilesToPACS::processResponseFromStoreSCP(unsigned int dimseStatusCode, QString filePathDicomObjectStoredFailed)
{
    QString messageErrorLog = "No s'ha pogut enviar el fitxer " + filePathDicomObjectStoredFailed + ", descripció error rebuda";

    // In section B.2.3, table B.2-1 we can find a description of the errors.
    // For details on "related fields" see PS 3.7, Annex C - Status Type Enconding

    // Also note that the meaning of the Status is different from that of MoveScu.
    // - Failure the image or it could be uploaded
    // - Warning the image has been uploaded, but not SOPClass, some data has been coerced ...

    if (dimseStatusCode == STATUS_Success)
    {
        // Image sent successfully
        m_numberOfDICOMFilesSentSuccessfully++;
        return;
    }

    switch (dimseStatusCode)
    {
    case STATUS_STORE_Refused_OutOfResources:
        // 0xA7XX
    case STATUS_STORE_Refused_SOPClassNotSupported:
        // 0x0122
    case STATUS_STORE_Error_DataSetDoesNotMatchSOPClass:
        // 0xA9XX
    case STATUS_STORE_Error_CannotUnderstand:
        // 0xCXXX
        ERROR_LOG(messageErrorLog + QString(DU_cstoreStatusString(dimseStatusCode)));
        break;

        /// Cohesion between types, one type has been converted to another type and data may have been lost, for example from decimal to integer, however
        /// files have been sent and saved
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER == 361
    case STATUS_STORE_Warning_CoersionOfDataElements:
#else if  PACKAGE_VERSION_NUMBER == 365
    case STATUS_STORE_Warning_CoercionOfDataElements:
#endif
#endif
        // 0xB000
    case STATUS_STORE_Warning_DataSetDoesNotMatchSOPClass:
        // 0xB007
    case STATUS_STORE_Warning_ElementsDiscarded:
        // 0xB006
        ERROR_LOG(messageErrorLog + QString(DU_cstoreStatusString(dimseStatusCode)));
        m_numberOfDICOMFilesSentWithWarning++;
        break;
        
    default:
        // An unintended error has occurred. In principle this branch should never be reached
        ERROR_LOG(messageErrorLog + QString(DU_cstoreStatusString(dimseStatusCode)));
        break;
    }
}

PACSRequestStatus::SendRequestStatus SendDICOMFilesToPACS::getStatusStoreSCU()
{
    // The error handling of StoreSCU is different from moveSCU, in moveSCU we receive a final status indicating how the operation went, while
    // in storeSCU for each image that is sent it receives a status, as we can have when sending a study, status failure, warning, ..., to the user
    // we will only send one error and show the most critical, for example if we have 5 Warning errors and one Failure, we will send error indicates that sending
    // of some images failed.

    if (m_abortIsRequested)
    {
        INFO_LOG("Sending images to PACS aborted");
        return PACSRequestStatus::SendCancelled;
    }
    else if (m_lastOFCondition == DIMSE_SENDFAILED)
    {
        ERROR_LOG("Lost connection to PACS while sending files");
        return PACSRequestStatus::SendPACSConnectionBroken;
    }
    else if (getNumberOfDICOMFilesSentSuccesfully() == 0)
    {
        // No hem guardat cap imatge (Failure Status)
        ERROR_LOG("Failed to send all files to PACS");
        return PACSRequestStatus::SendAllDICOMFilesFailed;
    }
    else if (getNumberOfDICOMFilesSentFailed() > 0)
    {
        // No s'han pogut guardar els fitxers
        ERROR_LOG(QString("Uploading% 1 of% 2 files to PACS failed")
                  .arg(QString().setNum(getNumberOfDICOMFilesSentFailed()), QString().setNum(m_numberOfDICOMFilesToSend)));
        return PACSRequestStatus::SendSomeDICOMFilesFailed;
    }
    else if (getNumberOfDICOMFilesSentWarning() > 0)
    {
        /// Some images have been saved with the Warning Status (Normally
        /// means that the PACS has modified the data of the sent DICOM file)
        WARN_LOG(QString("A warning was received while% 1 of% 2 files were sent")
                 .arg(QString().setNum(getNumberOfDICOMFilesSentWarning()), QString().setNum(m_numberOfDICOMFilesToSend)));
        return PACSRequestStatus::SendWarningForSomeImages;
    }

    INFO_LOG("All files have been sent to PACS successfully");

    return PACSRequestStatus::SendOk;
}

int SendDICOMFilesToPACS::getNumberOfDICOMFilesSentSuccesfully()
{
    return m_numberOfDICOMFilesSentSuccessfully;
}

int SendDICOMFilesToPACS::getNumberOfDICOMFilesSentFailed()
{
    return m_numberOfDICOMFilesToSend - m_numberOfDICOMFilesSentSuccessfully - m_numberOfDICOMFilesSentWithWarning;
}

int SendDICOMFilesToPACS::getNumberOfDICOMFilesSentWarning()
{
    return m_numberOfDICOMFilesSentWithWarning;
}

}
