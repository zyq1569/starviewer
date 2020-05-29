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

#include "querypacs.h"

#include <dimse.h>
#include <ofcond.h>
#include <diutil.h>
#include <dcsequen.h>

#include "pacsconnection.h"
#include "image.h"
#include "study.h"
#include "series.h"
#include "patient.h"
#include "createinformationmodelobject.h"
#include "dicomtagreader.h"
#include "logging.h"
#include "inputoutputsettings.h"
#include "dicommasktodcmdataset.h"

namespace udg {

// Constant que contindrà quin Abanstract Syntax de Find utilitzem entre els diversos que hi ha utilitzem
static const char *FindStudyAbstractSyntax = UID_FINDStudyRootQueryRetrieveInformationModel;

QueryPacs::QueryPacs(PacsDevice pacsDevice)
 : DIMSECService()
{
    m_pacsDevice = pacsDevice;
    m_pacsConnection = NULL;
    m_resultsDICOMSource.addRetrievePACS(pacsDevice);

    m_patientStudyListGot = false;
    m_seriesListGot = false;
    m_imageListGot = false;

    this->setUpAsCFind();
}

QueryPacs::~QueryPacs()
{
    //Esborrem els llistats de resultats de cerca que no ens hagin demanat a través dels mètodes get
    if (!m_patientStudyListGot)
    {
        foreach(Patient *patient, m_patientStudyList)
        {
            qDeleteAll(patient->getStudies());
            delete patient;
        }
    }

    if (!m_seriesListGot)
    {
        qDeleteAll(m_seriesList);
    }

    if (!m_imageListGot)
    {
        qDeleteAll(m_imageList);
    }
}

void QueryPacs::foundMatchCallback(void *callbackData, T_DIMSE_C_FindRQ *request, int responseCount, T_DIMSE_C_FindRSP *rsp,
    DcmDataset *responseIdentifiers)
{
    Q_UNUSED(rsp);
    Q_UNUSED(responseCount);

    QueryPacs *queryPacsCaller = (QueryPacs*)callbackData;

    if (queryPacsCaller->m_cancelQuery)
    {
        // Hem de comprovar si ja haviem demanat cancel·lar la Query. És degut a que tot i que demanem cancel·lar la query actual
        // el PACS ens envia els dataset que havia posat a la pila de la xarxa just abans de rebre el cancel·lar la query, per tant
        // pot ser que tot i havent demanat cancel·lar la query rebem algun resultat més, per això comprovem si ja havíem demanat
        // cancel·lar la query per no tornar-la  demanar, quan rebem aquests resultats que ja s'havien posat a la pila de la xarxa.
        // http://forum.dcmtk.org/viewtopic.php?t=2143
        if (!queryPacsCaller->m_cancelRequestSent)
        {
            queryPacsCaller->cancelQuery(request);
            queryPacsCaller->m_cancelRequestSent = true;
        }
    }
    else
    {
        DICOMTagReader *dicomTagReader = new DICOMTagReader("", responseIdentifiers);
        QString queryRetrieveLevel = dicomTagReader->getValueAttributeAsQString(DICOMQueryRetrieveLevel);

        if (queryRetrieveLevel == "STUDY")
        {
            // En el cas que l'objecte que cercàvem fos un estudi
            queryPacsCaller->addPatientStudy(dicomTagReader);
        }
        else if (queryRetrieveLevel == "SERIES")
        {
            // Si la query retorna un objecte sèrie
            queryPacsCaller->addPatientStudy(dicomTagReader);
            queryPacsCaller->addSeries(dicomTagReader);
        }
        else if (queryRetrieveLevel == "IMAGE")
        {
            // Si la query retorna un objecte imatge
            queryPacsCaller->addPatientStudy(dicomTagReader);
            queryPacsCaller->addSeries(dicomTagReader);
            queryPacsCaller->addImage(dicomTagReader);
        }
    }
}

PACSRequestStatus::QueryRequestStatus QueryPacs::query()
{
    m_pacsConnection = new PACSConnection(m_pacsDevice);
    T_DIMSE_C_FindRQ findRequest;
    T_DIMSE_C_FindRSP findResponse;

    if (!m_pacsConnection->connectToPACS(PACSConnection::Query))
    {
        ERROR_LOG("S'ha produit un error al intentar connectar al PACS per fer query. AE Title: " + m_pacsDevice.getAETitle());
        delete m_pacsConnection;
        return PACSRequestStatus::QueryCanNotConnectToPACS;
    }

    // Figure out which of the accepted presentation contexts should be used
    m_presId = ASC_findAcceptedPresentationContextID(m_pacsConnection->getConnection(), FindStudyAbstractSyntax);
    if (m_presId == 0)
    {
        ERROR_LOG("El PACS no ha acceptat el nivell de cerca d'estudis FINDStudyRootQueryRetrieveInformationModel");
        delete m_pacsConnection;
        return PACSRequestStatus::QueryFailedOrRefused;
    }

    // Prepare the transmission of data
    bzero((char*) &findRequest, sizeof(findRequest));
    findRequest.MessageID = m_pacsConnection->getConnection()->nextMsgID;
    strcpy(findRequest.AffectedSOPClassUID, FindStudyAbstractSyntax);
    findRequest.DataSetType = DIMSE_DATASET_PRESENT;

    DcmDataset *statusDetail = NULL;
    DcmDataset *dcmDatasetToQuery = DicomMaskToDcmDataset().getDicomMaskAsDcmDataset(m_dicomMask);

    // Finally conduct transmission of data
    OFCondition condition = DIMSE_findUser(m_pacsConnection->getConnection(), m_presId, &findRequest, dcmDatasetToQuery, foundMatchCallback, this, DIMSE_NONBLOCKING,
                                           Settings().getValue(InputOutputSettings::PACSConnectionTimeout).toInt(), &findResponse, &statusDetail);

    m_pacsConnection->disconnect();

    if (!condition.good())
    {
        ERROR_LOG(QString("Error al fer una consulta al PACS %1, descripcio error: %2").arg(m_pacsDevice.getAETitle(), condition.text()));
    }

    PACSRequestStatus::QueryRequestStatus queryRequestStatus = getDIMSEStatusCodeAsQueryRequestStatus(findResponse.DimseStatus);
    processServiceClassProviderResponseStatus(findResponse.DimseStatus, statusDetail);
    
    // Dump status detail information if there is some
    if (statusDetail != NULL)
    {
        delete statusDetail;
    }
    delete dcmDatasetToQuery;
    delete m_pacsConnection;

    return queryRequestStatus;
}

PACSRequestStatus::QueryRequestStatus QueryPacs::query(const DicomMask &mask)
{
    m_cancelQuery = false;
    m_cancelRequestSent = false;

    m_dicomMask = mask;

    return query();
}

void QueryPacs::cancelQuery()
{
    // Indiquem que s'ha de cancel·lar la query, el mètode foundMatchCallback, comprova el flag cada vegada que rep un resultat DICOM
    // que compleix amb la màscara de cerca
    m_cancelQuery = true;
}

void QueryPacs::cancelQuery(T_DIMSE_C_FindRQ *request)
{
    INFO_LOG(QString("Demanem cancel.lar al PACS %1 l'actual query").arg(m_pacsDevice.getAETitle()));

    // Tots els PACS està obligats pel DICOM Conformance a implementar la cancel·lació
    OFCondition cond = DIMSE_sendCancelRequest(m_pacsConnection->getConnection(), m_presId, request->MessageID);
    if (cond.bad())
    {
        ERROR_LOG("S'ha produit el seguent error al cancel.lar la query: " + QString(cond.text()));
        INFO_LOG(QString("Aborto la connexio amb el PACS %1").arg(m_pacsDevice.getAETitle()));

        // Si hi hagut un error demanant el cancel·lar, abortem l'associació, d'aquesta manera segur que cancel·lem la query
        ASC_abortAssociation(m_pacsConnection->getConnection());
    }
}

void QueryPacs::addPatientStudy(DICOMTagReader *dicomTagReader)
{
    Patient *patient = CreateInformationModelObject::createPatient(dicomTagReader);
    Study *study = CreateInformationModelObject::createStudy(dicomTagReader);
    study->setInstitutionName(m_pacsDevice.getInstitution());
    study->setDICOMSource(m_resultsDICOMSource);

    patient->addStudy(study);
    m_patientStudyList.append(patient);
}

void QueryPacs::addSeries(DICOMTagReader *dicomTagReader)
{
    Series *series = CreateInformationModelObject::createSeries(dicomTagReader);
    series->setDICOMSource(m_resultsDICOMSource);

    // TODO: Si ens fan una cerca a nivell d'imatge inserirem la mateixa serie tantes vegades com images tenim, s'hauria de comprovar si ja conté
    // la sèrie la llista abans d'afegir-la
    m_seriesList.append(series);
}

void QueryPacs::addImage(DICOMTagReader *dicomTagReader)
{
    Image *image = CreateInformationModelObject::createImage(dicomTagReader);
    image->setDICOMSource(m_resultsDICOMSource);

    m_imageList.append(image);
}

QList<Patient*> QueryPacs::getQueryResultsAsPatientStudyList()
{
    m_patientStudyListGot = true;
    return m_patientStudyList;
}

QList<Series*> QueryPacs::getQueryResultsAsSeriesList()
{
    m_seriesListGot = true;
    return m_seriesList;
}

QList<Image*> QueryPacs::getQueryResultsAsImageList()
{
    m_imageListGot = true;
    return m_imageList;
}

PACSRequestStatus::QueryRequestStatus QueryPacs::getDIMSEStatusCodeAsQueryRequestStatus(unsigned int dimseStatusCode)
{
    // Al PS 3.4, secció C.4.1.1.4, taula C.4-1 podem trobar un descripció dels errors.
    // Per a detalls sobre els "related fields" consultar PS 3.7, Annex C - Status Type Enconding

    if (dimseStatusCode == STATUS_Success)
    {
        return PACSRequestStatus::QueryOk;
    }

    PACSRequestStatus::QueryRequestStatus queryRequestStatus;

    switch (dimseStatusCode)
    {
        case STATUS_FIND_Refused_OutOfResources:
        case STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass:
        case STATUS_FIND_Failed_UnableToProcess:
            queryRequestStatus = PACSRequestStatus::QueryFailedOrRefused;
            break;

        case STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest:
            // L'usuari ha sol·licitat cancel·lar la descàrrega
            queryRequestStatus = PACSRequestStatus::QueryCancelled;
            break;

        default:
            queryRequestStatus = PACSRequestStatus::QueryUnknowStatus;
            break;
    }

    return queryRequestStatus;
}

}
