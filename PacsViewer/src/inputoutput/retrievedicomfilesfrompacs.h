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

#ifndef RETRIEVEDICOMFILESFROMPACS_H
#define RETRIEVEDICOMFILESFROMPACS_H

#include <QObject>
#include <ofcond.h>
#include <assoc.h>

#include "pacsdevice.h"
#include "pacsrequeststatus.h"
#include "dimsecservice.h"

struct T_DIMSE_C_MoveRQ;
struct T_DIMSE_C_MoveRSP;
struct T_DIMSE_C_StoreRQ;
struct T_DIMSE_StoreProgress;
struct T_DIMSE_C_StoreRSP;
struct T_DIMSE_Message;

class DcmDataset;
class DcmFileFormat;

namespace udg {

class DicomMask;
class DICOMTagReader;
class PACSConnection;

/**
    This class is responsible for interacting with PACS, responding to move and store services
*/
class RetrieveDICOMFilesFromPACS : public QObject, public DIMSECService {
    Q_OBJECT
public:
    RetrieveDICOMFilesFromPACS(PacsDevice pacs);

    /// Starts the download
    PACSRequestStatus::RetrieveRequestStatus retrieve(const QString &studyInstanceUID, const QString &seriesInstanceUID = "", const QString &sopInstanceUID = "");

    /// Cancel download. Download cancellation is asynchronous, when the studio has been canceled the Status RetrieveCancelled is returned
    /// The reason it is asynchronous is because while the download of an image is being processed, the download cannot be canceled, only
    /// do right after receiving an image.
    void requestCancel();

    ///Returns the number of downloaded images
    int getNumberOfDICOMFilesRetrieved();

signals:
    /// Signal indicating that a file has been downloaded
    void DICOMFileRetrieved(DICOMTagReader *dicomTagReader, int numberOfImagesRetrieved);

private:
    /// In this function we accept the connection that is requested to us to transmit images to us,
    /// and indicate which transfer syntax we support
    OFCondition acceptSubAssociation(T_ASC_Network *associationNetwork, T_ASC_Association **association);

    ///We respond to an echo request
    OFCondition echoSCP(T_ASC_Association *association, T_DIMSE_Message *dimseMessage, T_ASC_PresentationContextID presentationContextID);

    /// We respond to a request to save an image
    OFCondition storeSCP(T_ASC_Association *association, T_DIMSE_Message *messagge, T_ASC_PresentationContextID presentationContextID);

    ///Accept the connection that the PACS makes to us, to become a scp
    OFCondition subOperationSCP(T_ASC_Association **subAssociation);

    /// Guarda una composite instance descarregada
    OFCondition save(DcmFileFormat *fileRetrieved, QString dicomFileAbsolutePath);

    /// Returns the name of the file to save the downloaded object,
    /// composes the path where the file name should be saved most.
    /// If the path where the image is to be saved does not exist, create it
    QString getAbsoluteFilePathCompositeInstance(DcmDataset *imageDataset, QString fileName);

    ///Returns the DcmDataset with the study data requested for download
    DcmDataset* getDcmDatasetOfImagesToRetrieve(const QString &studyInstanceUID, const QString &seriesInstanceUID, const QString &sopInstanceUID);

    /// Configure the MoveRequest object to download DICOM files
    T_DIMSE_C_MoveRQ getConfiguredMoveRequest(T_ASC_Association *association);

    /// Translates DIMSE status code to PACSRequestStatus::RetrieveRequestStatus
    PACSRequestStatus::RetrieveRequestStatus getDIMSEStatusCodeAsRetrieveRequestStatus(unsigned int dimseStatusCode);

    ///Callback from move, it would seem to run every time an image has been downloaded
    static void moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *moveRequest, int responseCount, T_DIMSE_C_MoveRSP *moveResponse);

    /// This function is responsible for saving each DICOM frame we receive
    static void storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *storeRequest, char *imageFileName,
                                 DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *storeResponse, DcmDataset **statusDetail);

    /// Callback that was executed when an image download suboperation was started
    static void subOperationCallback(void *subOperationCallbackData, T_ASC_Network *associationNetwork, T_ASC_Association **subAssociation);

private:
    struct StoreSCPCallbackData
    {
        DcmFileFormat *dcmFileFormat;
        RetrieveDICOMFilesFromPACS *retrieveDICOMFilesFromPACS;
        QString fileName;
    };

    struct MoveSCPCallbackData
    {
        T_ASC_Association *association;
        T_ASC_PresentationContextID presentationContextId;
        RetrieveDICOMFilesFromPACS *retrieveDICOMFilesFromPACS;
    };

    /// Request DICOM association;
    PacsDevice m_pacs;
    PACSConnection *m_pacsConnection;

    int m_numberOfImagesRetrieved;

    bool m_abortIsRequested;

};

};
#endif
