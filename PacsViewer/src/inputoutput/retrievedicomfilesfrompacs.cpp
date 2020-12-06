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

#include "retrievedicomfilesfrompacs.h"

// Make sure OS specific configuration is included first
#include <osconfig.h>
#include <diutil.h>
#include <dcfilefo.h>
//Pels tags DcmTagKey DCM_xxxx
#include <dctagkey.h>
#include <dcdeftag.h>

#include <QDir>
#include <QString>

#include "localdatabasemanager.h"
#include "dicommask.h"
#include "logging.h"
#include "dicomtagreader.h"
#include "pacsconnection.h"
#include "pacsdevice.h"

namespace udg {

// Constant that will contain which Abanstract Syntax of Move we use among the various that we use
static const char *MoveAbstractSyntax = UID_MOVEStudyRootQueryRetrieveInformationModel;

RetrieveDICOMFilesFromPACS::RetrieveDICOMFilesFromPACS(PacsDevice pacs)
    : DIMSECService()
{
    m_pacs = pacs;
    m_abortIsRequested = false;

    this->setUpAsCMove();
}

OFCondition RetrieveDICOMFilesFromPACS::acceptSubAssociation(T_ASC_Network *associationNetwork, T_ASC_Association **association)
{
    const char *knownAbstractSyntaxes[] = { UID_VerificationSOPClass };
    const char *transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                       NULL, NULL, NULL , NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    int numTransferSyntaxes;

    OFCondition condition = ASC_receiveAssociation(associationNetwork, association, ASC_DEFAULTMAXPDU);

    if (condition.good())
    {
#ifndef DISABLE_COMPRESSION_EXTENSION
        // If we have compression we ask for it, and we can speed up the download time considerably
        // For now we ask for the lossless compression that every PACS that supports compression has
        // to provide: JPEGLossless: Non-Hierarchical-1stOrderPrediction
        transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        transferSyntaxes[4] = UID_JPEGLSLosslessTransferSyntax;
        transferSyntaxes[5] = UID_JPEG2000LosslessOnlyTransferSyntax;
        numTransferSyntaxes = 6;

        //transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
        //transferSyntaxes[1] = UID_JPEG2000LosslessOnlyTransferSyntax;
        //transferSyntaxes[2] = UID_JPEGProcess2_4TransferSyntax;
        //transferSyntaxes[3] = UID_JPEGProcess1TransferSyntax;
        //transferSyntaxes[4] = UID_JPEGProcess14SV1TransferSyntax;//
        //transferSyntaxes[5] = UID_JPEGLSLossyTransferSyntax;
        //transferSyntaxes[6] = UID_JPEGLSLosslessTransferSyntax;
        //transferSyntaxes[7] = UID_RLELosslessTransferSyntax;
        //transferSyntaxes[8] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
        //transferSyntaxes[9] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
        //transferSyntaxes[10] = UID_MPEG4HighProfileLevel4_1TransferSyntax;
        //transferSyntaxes[11] = UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax;
        //transferSyntaxes[12] = UID_MPEG4HighProfileLevel4_2_For2DVideoTransferSyntax;
        //transferSyntaxes[13] = UID_MPEG4HighProfileLevel4_2_For3DVideoTransferSyntax;
        //transferSyntaxes[14] = UID_MPEG4StereoHighProfileLevel4_2TransferSyntax;
        //transferSyntaxes[15] = UID_HEVCMainProfileLevel5_1TransferSyntax;
        //transferSyntaxes[16] = UID_HEVCMain10ProfileLevel5_1TransferSyntax;
        //transferSyntaxes[17] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
        //if (gLocalByteOrder == EBO_LittleEndian)
        //{
        //    transferSyntaxes[18] = UID_LittleEndianExplicitTransferSyntax;
        //    transferSyntaxes[19] = UID_BigEndianExplicitTransferSyntax;
        //}
        //else
        //{
        //    transferSyntaxes[18] = UID_BigEndianExplicitTransferSyntax;
        //    transferSyntaxes[19] = UID_LittleEndianExplicitTransferSyntax;
        //}
        //transferSyntaxes[20] = UID_LittleEndianImplicitTransferSyntax;
        //transferSyntaxes[21] = UID_JPEGProcess14TransferSyntax;
        //numTransferSyntaxes = 22;

#else
        // Defined in dcxfer.h
        if (gLocalByteOrder == EBO_LittleEndian)
        {
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        }
        else
        {
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        }
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
#endif

        // Accept the Verification SOP Class if presented
        condition = ASC_acceptContextsWithPreferredTransferSyntaxes(
                    (*association)->params, knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes),
                    transferSyntaxes, numTransferSyntaxes);

        if (condition.good())
        {
#ifdef  PACKAGE_VERSION_NUMBER

#if PACKAGE_VERSION_NUMBER == 361
            // The array of Storage SOP Class UIDs comes from dcuid.h
            condition = ASC_acceptContextsWithPreferredTransferSyntaxes(
                        (*association)->params, dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
                        transferSyntaxes, numTransferSyntaxes);
#else //if  PACKAGE_VERSION_NUMBER  >= 363

            condition = ASC_acceptContextsWithPreferredTransferSyntaxes(
                        (*association)->params, dcmAllStorageSOPClassUIDs, numberOfDcmAllStorageSOPClassUIDs,
                        transferSyntaxes, numTransferSyntaxes);

#endif

#endif
        }
    }

    if (condition.good())
    {
        condition = ASC_acknowledgeAssociation(*association);
    }
    else
    {
        ASC_dropAssociation(*association);
        ASC_destroyAssociation(association);
    }
    return condition;
}

void RetrieveDICOMFilesFromPACS::moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *request, int responseCount, T_DIMSE_C_MoveRSP *response)
{
    Q_UNUSED(responseCount);
    Q_UNUSED(response);
    Q_UNUSED(request);
    Q_UNUSED(callbackData);

    /// This in theory is the code to cancel a download but
    /// the PACS of the UDIAT does not support the requestCancel, therefore the only way
    /// to do so is as done in the subOperationSCP method which aborts the connection to the PACS.

    MoveSCPCallbackData *moveSCPCallbackData = (MoveSCPCallbackData*) callbackData;

    if (moveSCPCallbackData->retrieveDICOMFilesFromPACS->m_abortIsRequested)
    {
        OFCondition condition = DIMSE_sendCancelRequest(
                    moveSCPCallbackData->association, moveSCPCallbackData->presentationContextId, request->MessageID);

        if (condition.good())
        {
            INFO_LOG("Download canceled");
        }
        else
        {
            ERROR_LOG("Error trying to cancel download. Description error: " + QString(condition.text()));
        }
    }
}

OFCondition RetrieveDICOMFilesFromPACS::echoSCP(T_ASC_Association *association, T_DIMSE_Message *dimseMessage,
                                                T_ASC_PresentationContextID presentationContextID)
{
    // The echo succeeded
    OFCondition condition = DIMSE_sendEchoResponse(association,
                                                   presentationContextID, &dimseMessage->msg.CEchoRQ, STATUS_Success, NULL);
    if (condition.bad())
    {
        ERROR_LOG("The PACS requested an echo during the download but the response to this failed");
    }

    return condition;
}

void RetrieveDICOMFilesFromPACS::storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *storeRequest, char *imageFileName,
                                                  DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *storeResponse, DcmDataset **statusDetail)
{
    // Input parameters: callbackData, progress, storeRequest, imageFileName, imageDataSet
    // Output parameters: storeResponse, statusDetail
    Q_UNUSED(imageFileName);

    // If the package is at the end of an image we must save it
    if (progress->state == DIMSE_StoreEnd)
    {
        // No status detail
        *statusDetail = NULL;

        if ((imageDataSet) && (*imageDataSet))
        {
            DIC_UI sopClass, sopInstance;
            OFBool correctUIDPadding = OFFalse;
            StoreSCPCallbackData *storeSCPCallbackData = (StoreSCPCallbackData*)callbackData;
            RetrieveDICOMFilesFromPACS *retrieveDICOMFilesFromPACS = storeSCPCallbackData->retrieveDICOMFilesFromPACS;
            QString dicomFileAbsolutePath = retrieveDICOMFilesFromPACS->getAbsoluteFilePathCompositeInstance(*imageDataSet, storeSCPCallbackData->fileName);

            //Let's save the image
            OFCondition stateSaveImage = retrieveDICOMFilesFromPACS->save(storeSCPCallbackData->dcmFileFormat, dicomFileAbsolutePath);

            if (stateSaveImage.bad())
            {
                storeResponse->DimseStatus = STATUS_STORE_Refused_OutOfResources;
                DEBUG_LOG("The downloaded image could not be saved [" + dicomFileAbsolutePath + "], error: " + stateSaveImage.text());
                ERROR_LOG("The downloaded image could not be saved [" + dicomFileAbsolutePath + "], error: " + stateSaveImage.text());
                if (!QFile::remove(dicomFileAbsolutePath))
                {
                    DEBUG_LOG ("Failed to delete file" + dicomFileAbsolutePath + "previously failed to save.");
                    ERROR_LOG ("Failed to delete file" + dicomFileAbsolutePath + "previously failed to save.");
                }
            }
            else
            {
                // Should really check the image to make sure it is consistent, that its
                // sopClass and sopInstance correspond with those in the request.
                if (storeResponse->DimseStatus == STATUS_Success)
                {
                    // Which SOP class and SOP instance?
                    //if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, correctUIDPadding))
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER < 365
                    if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, correctUIDPadding))
#else if  PACKAGE_VERSION_NUMBER == 365
                    if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass,sizeof(sopClass), sopInstance, sizeof(correctUIDPadding)))
#endif
#endif
                    {
                        storeResponse->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
                        ERROR_LOG(QString("Sop class and sop instance not found for image %1").arg(storeSCPCallbackData->fileName));
                    }
                    else if (strcmp(sopClass, storeRequest->AffectedSOPClassUID) != 0)
                    {
                        storeResponse->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
                        ERROR_LOG(QString("The sop class received does not match the one requested by the image%1").arg(storeSCPCallbackData->fileName));
                    }
                    else if (strcmp(sopInstance, storeRequest->AffectedSOPInstanceUID) != 0)
                    {
                        storeResponse->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
                        ERROR_LOG(QString("It does not match the received instance with the one requested by the image %1").arg(storeSCPCallbackData->fileName));
                    }
                }

                // TODO:You have to process the file if any of the above checks failed ?
                retrieveDICOMFilesFromPACS->m_numberOfImagesRetrieved++;
                DICOMTagReader *dicomTagReader = new DICOMTagReader(dicomFileAbsolutePath, storeSCPCallbackData->dcmFileFormat->getAndRemoveDataset());
                emit retrieveDICOMFilesFromPACS->DICOMFileRetrieved(dicomTagReader, retrieveDICOMFilesFromPACS->m_numberOfImagesRetrieved);
            }
        }
    }
}

OFCondition RetrieveDICOMFilesFromPACS::save(DcmFileFormat *fileRetrieved, QString dicomFileAbsolutePath)
{
    //We indicate that we do not use meta-header
    E_FileWriteMode writeMode = EWM_fileformat;
    E_EncodingType sequenceType = EET_ExplicitLength;
    E_GrpLenEncoding groupLength = EGL_recalcGL;
    E_PaddingEncoding paddingType = EPD_withoutPadding;
    Uint32 filePadding = 0, itemPadding = 0;
    E_TransferSyntax transferSyntaxFile = fileRetrieved->getDataset()->getOriginalXfer();

    return fileRetrieved->saveFile(qPrintable(QDir::toNativeSeparators(dicomFileAbsolutePath)), transferSyntaxFile, sequenceType, groupLength, paddingType,
                                   filePadding, itemPadding, writeMode);
}

OFCondition RetrieveDICOMFilesFromPACS::storeSCP(T_ASC_Association *association, T_DIMSE_Message *msg, T_ASC_PresentationContextID presentationContextID)
{
    T_DIMSE_C_StoreRQ *storeRequest = &msg->msg.CStoreRQ;
    OFBool useMetaheader = OFTrue;
    StoreSCPCallbackData storeSCPCallbackData;
    DcmFileFormat retrievedFile;
    DcmDataset *retrievedDataset = retrievedFile.getDataset();

    storeSCPCallbackData.dcmFileFormat = &retrievedFile;
    storeSCPCallbackData.retrieveDICOMFilesFromPACS = this;
    storeSCPCallbackData.fileName = storeRequest->AffectedSOPInstanceUID;

    OFCondition condition = DIMSE_storeProvider(association, presentationContextID, storeRequest, NULL, useMetaheader, &retrievedDataset, storeSCPCallback,
                                                (void*) &storeSCPCallbackData, DIMSE_BLOCKING, 0);

    if (condition.bad())
    {
        // Remove file
        ERROR_LOG("Occurred while processing a download request from a file, description error " + QString(condition.text()));
        unlink(qPrintable(storeSCPCallbackData.fileName));
    }

    return condition;
}

OFCondition RetrieveDICOMFilesFromPACS::subOperationSCP(T_ASC_Association **subAssociation)
{
    /// We become like a service. The PACS makes us requests that we must respond,
    /// you can ask us to download an image or do an echo
    T_DIMSE_Message dimseMessage;
    T_ASC_PresentationContextID presentationContextID;

    if (!ASC_dataWaiting(*subAssociation, 0))
    {
        return DIMSE_NODATAAVAILABLE;
    }

    OFCondition condition = DIMSE_receiveCommand(*subAssociation, DIMSE_BLOCKING, 0, &presentationContextID, &dimseMessage, NULL);

    if (condition == EC_Normal)
    {
        switch (dimseMessage.CommandField)
        {
        case DIMSE_C_STORE_RQ:
            condition = storeSCP(*subAssociation, &dimseMessage, presentationContextID);
            break;

        case DIMSE_C_ECHO_RQ:
            condition = echoSCP(*subAssociation, &dimseMessage, presentationContextID);
            break;

        default:
            ERROR_LOG("The PACS has requested an invalid type of operation");
            condition = DIMSE_BADCOMMANDTYPE;
            break;
        }
    }
    // Clean up on association termination
    if (condition == DUL_PEERREQUESTEDRELEASE)
    {
        INFO_LOG("The PACS requests to close the connection through which it sent us the files");
        condition = ASC_acknowledgeRelease(*subAssociation);
        ASC_dropSCPAssociation(*subAssociation);
        ASC_destroyAssociation(subAssociation);
        return condition;
    }
    else if (condition == DUL_PEERABORTEDASSOCIATION)
    {
        INFO_LOG("PACS aborted connection");
    }
    else if (condition != EC_Normal)
    {
        ERROR_LOG("An error occurred while receiving a suboperation request, error description: " + QString(condition.text()));
        condition = ASC_abortAssociation(*subAssociation);
    }
    else if (m_abortIsRequested)
    {
        INFO_LOG("We will abort the connections with the PACS, because they have requested to cancel the download");
        condition = ASC_abortAssociation(*subAssociation);
        if (!condition.good())
        {
            ERROR_LOG("Error aborting the connection for which we received the images" + QString(condition.text()));
        }

        /// We close the connection with the PACS because according to the DICOM documentation in PS 3.4
        ///(Baseline Behavior of SCP) C.4.2.3.1 if we abort
        /// the connection through which we receive the images, the behavior of the PACS is unknown,
        /// for example DCM4CHEE closes the connection with the PACS, but
        /// RAIM_Server does not close it and keeps it from ever leaving this class.
        ///  Because it is not possible to know in this situation how they will act
        /// the PACS closes the connection to the PACS here.
        condition = ASC_abortAssociation(m_pacsConnection->getConnection());
        if (!condition.good())
        {
            ERROR_LOG("Error aborting connection to PACS" + QString(condition.text()));
        }
        else
        {
            INFO_LOG("Aborted connection to PACS");
        }
    }

    if (condition != EC_Normal)
    {
        ASC_dropAssociation(*subAssociation);
        ASC_destroyAssociation(subAssociation);
    }
    return condition;
}

void RetrieveDICOMFilesFromPACS::subOperationCallback(void *subOperationCallbackData, T_ASC_Network *associationNetwork, T_ASC_Association **subAssociation)
{
    RetrieveDICOMFilesFromPACS *retrieveDICOMFilesFromPACS = (RetrieveDICOMFilesFromPACS*)subOperationCallbackData;
    if (associationNetwork == NULL)
    {
        // Help no net !
        return;
    }

    if (*subAssociation == NULL)
    {
        OFCondition condition = retrieveDICOMFilesFromPACS->acceptSubAssociation(associationNetwork, subAssociation);
        if (!condition.good())
        {
            ERROR_LOG("An error occurred while negotiating the association of the incoming DICOM connection, description error: " + QString(condition.text()));
        }
        else
        {
            INFO_LOG("Connection request received by the incoming DICOM connection port of the PACS.");
        }
    }
    else
    {
        retrieveDICOMFilesFromPACS->subOperationSCP(subAssociation);
    }
}

PACSRequestStatus::RetrieveRequestStatus RetrieveDICOMFilesFromPACS::
retrieve(const QString &studyInstanceUID, const QString &seriesInstanceUID, const QString &sopInstanceUID)
{
    T_ASC_PresentationContextID presentationContextID;
    T_DIMSE_C_MoveRSP moveResponse;
    DcmDataset *statusDetail = NULL;
    m_pacsConnection = new PACSConnection(m_pacs);
    PACSRequestStatus::RetrieveRequestStatus retrieveRequestStatus;
    MoveSCPCallbackData moveSCPCallbackData;
    DcmDataset *dcmDatasetToRetrieve = getDcmDatasetOfImagesToRetrieve(studyInstanceUID, seriesInstanceUID, sopInstanceUID);
    m_numberOfImagesRetrieved = 0;

    // TODO It should be checked that it is a PACS with the retrieve service configured
    if (!m_pacsConnection->connectToPACS(PACSConnection::RetrieveDICOMFiles))
    {
        ERROR_LOG("An error occurred while trying to connect to the PACS for a retrieve. AE Title: " + m_pacs.getAETitle());
        return PACSRequestStatus::RetrieveCanNotConnectToPACS;
    }

    // Which presentation context should be used, It's important that the connection has MoveStudyRoot level
    T_ASC_Association *association = m_pacsConnection->getConnection();
    presentationContextID = ASC_findAcceptedPresentationContextID(association, MoveAbstractSyntax);
    if (presentationContextID == 0)
    {
        ERROR_LOG("No valid presentation context found");
        return PACSRequestStatus::RetrieveFailureOrRefused;
    }

    moveSCPCallbackData.association = association;
    moveSCPCallbackData.presentationContextId = presentationContextID;
    moveSCPCallbackData.retrieveDICOMFilesFromPACS = this;

    // Set the destination of the images to us
    T_DIMSE_C_MoveRQ moveRequest = getConfiguredMoveRequest(association);
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER < 365
    ASC_getAPTitles(association->params, moveRequest.MoveDestination, NULL, NULL);
#else if  PACKAGE_VERSION_NUMBER == 365
    ASC_getAPTitles(association->params, moveRequest.MoveDestination, sizeof(moveRequest.MoveDestination), NULL, 0, NULL, 0);
#endif
#endif
    OFCondition condition = DIMSE_moveUser(association, presentationContextID, &moveRequest, dcmDatasetToRetrieve, moveCallback, &moveSCPCallbackData,
                                           DIMSE_BLOCKING, 0, m_pacsConnection->getNetwork(), subOperationCallback, this, &moveResponse, &statusDetail,
                                           NULL /*responseIdentifiers*/);

    if (condition.bad())
    {
        ERROR_LOG(QString("The download method did not complete successfully. Error code: %1, error description: %2")
                  .arg(condition.code())
                  .arg(condition.text()));
    }

    m_pacsConnection->disconnect();

    retrieveRequestStatus = getDIMSEStatusCodeAsRetrieveRequestStatus(moveResponse.DimseStatus);
    processServiceClassProviderResponseStatus(moveResponse.DimseStatus, statusDetail);
    
    // Dump status detail information if there is some
    if (statusDetail != NULL)
    {
        delete statusDetail;
    }

    delete dcmDatasetToRetrieve;

    return retrieveRequestStatus;
}

void RetrieveDICOMFilesFromPACS::requestCancel()
{
    m_abortIsRequested = true;
    INFO_LOG("You have been asked to cancel the download");
}

int RetrieveDICOMFilesFromPACS::getNumberOfDICOMFilesRetrieved()
{
    return m_numberOfImagesRetrieved;
}

T_DIMSE_C_MoveRQ RetrieveDICOMFilesFromPACS::getConfiguredMoveRequest(T_ASC_Association *association)
{
    T_DIMSE_C_MoveRQ moveRequest;
    DIC_US messageId = association->nextMsgID++;

    moveRequest.MessageID = messageId;
    strcpy(moveRequest.AffectedSOPClassUID, MoveAbstractSyntax);
    moveRequest.Priority = DIMSE_PRIORITY_MEDIUM;
    moveRequest.DataSetType = DIMSE_DATASET_PRESENT;

    return moveRequest;
}

DcmDataset* RetrieveDICOMFilesFromPACS::
getDcmDatasetOfImagesToRetrieve(const QString &studyInstanceUID, const QString &seriesInstanceUID, const QString &sopInstanceUID)
{
    DcmDataset *dcmDatasetToRetrieve = new DcmDataset();
    QString retrieveLevel = "STUDY";
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER == 361
    DcmElement *elemSpecificCharacterSet = newDicomElement(DCM_SpecificCharacterSet);
#else //if  PACKAGE_VERSION_NUMBER  >= 363
    DcmElement *elemSpecificCharacterSet = DcmItem::newDicomElement(DCM_SpecificCharacterSet);
#endif
#endif

    // ISO_IR 100 and Latin1
    elemSpecificCharacterSet->putString("ISO_IR 100");
    dcmDatasetToRetrieve->insert(elemSpecificCharacterSet, OFTrue);
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER == 361
    DcmElement *elem = newDicomElement(DCM_StudyInstanceUID);
#else //if  PACKAGE_VERSION_NUMBER  >= 363
    DcmElement *elem = DcmItem::newDicomElement(DCM_StudyInstanceUID);
#endif
#endif

    elem->putString(qPrintable(studyInstanceUID));
    dcmDatasetToRetrieve->insert(elem, OFTrue);

    if (!seriesInstanceUID.isEmpty())
    {
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER == 361
        DcmElement *elem = newDicomElement(DCM_SeriesInstanceUID);
#else //if  PACKAGE_VERSION_NUMBER  >= 363
        DcmElement *elem = DcmItem::newDicomElement(DCM_SeriesInstanceUID);
#endif
#endif
        elem->putString(qPrintable(seriesInstanceUID));
        dcmDatasetToRetrieve->insert(elem, OFTrue);
        retrieveLevel = "SERIES";
    }

    if (!sopInstanceUID.isEmpty())
    {
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER == 361
        DcmElement *elem = newDicomElement(DCM_SOPInstanceUID);
#else //if  PACKAGE_VERSION_NUMBER >= 363
        DcmElement *elem = DcmItem::newDicomElement(DCM_SOPInstanceUID);
#endif
#endif
        elem->putString(qPrintable(sopInstanceUID));
        dcmDatasetToRetrieve->insert(elem, OFTrue);
        retrieveLevel = "IMAGE";
    }

    //We specify at what level the QueryRetrieve is done
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER == 361
    DcmElement *elemQueryRetrieveLevel = newDicomElement(DCM_QueryRetrieveLevel);
#else //if  PACKAGE_VERSION_NUMBER >= 363
    DcmElement *elemQueryRetrieveLevel = DcmItem::newDicomElement(DCM_QueryRetrieveLevel);
#endif
#endif

    elemQueryRetrieveLevel->putString(qPrintable(retrieveLevel));
    dcmDatasetToRetrieve->insert(elemQueryRetrieveLevel, OFTrue);

    return dcmDatasetToRetrieve;
}

PACSRequestStatus::RetrieveRequestStatus RetrieveDICOMFilesFromPACS::
getDIMSEStatusCodeAsRetrieveRequestStatus(unsigned int dimseStatusCode)
{
    PACSRequestStatus::RetrieveRequestStatus retrieveRequestStatus;

    // In PS 3.4, section C.4.2.1.5, table C.4-2 we can find a description of the errors.
    // In PS 3.4, section C.4.2.3.1 the general types of error are described
    // Failure or Refused: Some images could not be downloaded
    // Warning: At least one image could be downloaded
    // Success: All images have been downloaded successfully

    // For details on "related fields" see PS 3.7, Annex C - Status Type Enconding

    if (dimseStatusCode == STATUS_Success)
    {
        return PACSRequestStatus::RetrieveOk;
    }

    switch (dimseStatusCode)
    {
    case STATUS_MOVE_Failed_MoveDestinationUnknown:
        retrieveRequestStatus = PACSRequestStatus::RetrieveDestinationAETileUnknown;
        break;

    case STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches:
    case STATUS_MOVE_Refused_OutOfResourcesSubOperations:
    case STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass:
    case STATUS_MOVE_Failed_UnableToProcess:
        retrieveRequestStatus = PACSRequestStatus::RetrieveFailureOrRefused;
        break;

    case STATUS_MOVE_Warning_SubOperationsCompleteOneOrMoreFailures:
        retrieveRequestStatus = PACSRequestStatus::RetrieveSomeDICOMFilesFailed;
        break;

    case STATUS_MOVE_Cancel_SubOperationsTerminatedDueToCancelIndication:
        // The user has requested to cancel the download
        retrieveRequestStatus = PACSRequestStatus::RetrieveCancelled;
        break;

    default:
        retrieveRequestStatus = PACSRequestStatus::RetrieveUnknowStatus;
        break;
    }

    return retrieveRequestStatus;
}

QString RetrieveDICOMFilesFromPACS::
getAbsoluteFilePathCompositeInstance(DcmDataset *imageDataset, QString fileName)
{
    QString absoluteFilePath = LocalDatabaseManager::getCachePath();
    QDir directory;
    const char *instanceUID;
    OFCondition dicomQueryStatus;

    dicomQueryStatus = imageDataset->findAndGetString(DCM_StudyInstanceUID, instanceUID, false);
    if (dicomQueryStatus.bad())
    {
        DEBUG_LOG("Failed to get study UID in dataset. Rao: " + QString(dicomQueryStatus.text()));
    }
    else
    {
        absoluteFilePath += QString(instanceUID) + "/";
    }

    // Check, if the studio directory is already created
    if (!directory.exists(absoluteFilePath))
    {
        directory.mkdir(absoluteFilePath);
    }

    dicomQueryStatus = imageDataset->findAndGetString(DCM_SeriesInstanceUID, instanceUID, false);
    if (dicomQueryStatus.bad())
    {
        DEBUG_LOG("Failed to get serial UID in dataset. Rao: " + QString(dicomQueryStatus.text()));
    }
    else
    {
        absoluteFilePath += QString(instanceUID) + "/";
    }

    // Let's check, if the directory of the series is already created, otherwise we create it
    if (!directory.exists(absoluteFilePath))
    {
        directory.mkdir(absoluteFilePath);
    }

    return absoluteFilePath + fileName;
}

}
