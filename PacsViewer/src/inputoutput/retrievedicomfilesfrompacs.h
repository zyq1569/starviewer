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
    Aquesta classe s'encarrega d'interactuars amb els PACS, responent als serveis move i store
  */
class RetrieveDICOMFilesFromPACS : public QObject, public DIMSECService {
Q_OBJECT
public:
    RetrieveDICOMFilesFromPACS(PacsDevice pacs);

    /// Starts the download
    PACSRequestStatus::RetrieveRequestStatus retrieve(const QString &studyInstanceUID, const QString &seriesInstanceUID = "", const QString &sopInstanceUID = "");

    /// Cancel·la la descàrrega. La cancel·lació de la descàrrega és assíncrona, quan l'estudi s'ha cancel·lat es retorna l'Status RetrieveCancelled
    /// El motiu de que sigui assíncron és perquè mentre s'està processant la descàrrega d'una imatge no es pot cancel·lar la descàrrega, només es pot
    /// fer just després d'haver rebut una imatge.
    void requestCancel();

    /// Retorna el número d'imatges descarregades
    int getNumberOfDICOMFilesRetrieved();

signals:
    /// Signal que indica que s'ha descarregat un fitxer
    void DICOMFileRetrieved(DICOMTagReader *dicomTagReader, int numberOfImagesRetrieved);

private:
    /// En aquesta funció acceptem la connexió que se'ns sol·licita per transmetre'ns imatges, i indiquem quins transfer syntax suportem
    OFCondition acceptSubAssociation(T_ASC_Network *associationNetwork, T_ASC_Association **association);

    /// Responem a una petició d'echo
    OFCondition echoSCP(T_ASC_Association *association, T_DIMSE_Message *dimseMessage, T_ASC_PresentationContextID presentationContextID);

    /// Responem a una petició per guardar una imatge
    OFCondition storeSCP(T_ASC_Association *association, T_DIMSE_Message *messagge, T_ASC_PresentationContextID presentationContextID);

    /// Accepta la connexió que ens fa el PACS, per convertir-nos en un scp
    OFCondition subOperationSCP(T_ASC_Association **subAssociation);

    /// Guarda una composite instance descarregada
    OFCondition save(DcmFileFormat *fileRetrieved, QString dicomFileAbsolutePath);

    /// Retorna el nom del fitxer amb que s'ha de guardar l'objecte descarregat, composa el path on s'ha de guardar més el nom del fitxer.
    /// Si el path on s'ha de guardar la imatge no existeix, el crea
    QString getAbsoluteFilePathCompositeInstance(DcmDataset *imageDataset, QString fileName);

    ///Retorna el DcmDataset amb les dades de l'estudi sol·licitat per descarregar
    DcmDataset* getDcmDatasetOfImagesToRetrieve(const QString &studyInstanceUID, const QString &seriesInstanceUID, const QString &sopInstanceUID);

    /// Configura l'objecte MoveRequest per la descàrrega de fitxers DICOM
    T_DIMSE_C_MoveRQ getConfiguredMoveRequest(T_ASC_Association *association);

    /// Translates DIMSE status code to PACSRequestStatus::RetrieveRequestStatus
    PACSRequestStatus::RetrieveRequestStatus getDIMSEStatusCodeAsRetrieveRequestStatus(unsigned int dimseStatusCode);

    /// Callback de move, semblaria que s'executa cada vegada que s'ha descarregat una imatge
    static void moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *moveRequest, int responseCount, T_DIMSE_C_MoveRSP *moveResponse);

    /// Aquesta funció s'encarrega de guardar cada trama DICOM que rebem
    static void storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *storeRequest, char *imageFileName,
                                 DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *storeResponse, DcmDataset **statusDetail);

    /// Callback que s'ha executat quan s'inicia un suboperació de descàrrega d'imatges
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
