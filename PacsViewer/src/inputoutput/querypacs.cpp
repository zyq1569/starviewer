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

//Constant that will contain which Abanstract Syntax of Find we use among the various that we use
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
    //We delete listings of search results that we were not asked for through the get methods
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
        // We need to check if we have already asked to cancel the Query. This is because even though we ask you to cancel the current query
        // the PACS sends us the datasets it had put in the network stack just before receiving the query cancellation, therefore
        // it may be that despite having asked to cancel the query we receive some more results, so we check if we had already asked
        // cancel the query so as not to ask for it again, when we receive these results that had already been placed in the network stack.
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
            // In the event that the object we were looking for was a study
            queryPacsCaller->addPatientStudy(dicomTagReader);
        }
        else if (queryRetrieveLevel == "SERIES")
        {
            // If the query returns a serial object
            queryPacsCaller->addPatientStudy(dicomTagReader);
            queryPacsCaller->addSeries(dicomTagReader);
        }
        else if (queryRetrieveLevel == "IMAGE")
        {
            // If the query returns an image object
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
        ERROR_LOG("An error occurred while trying to connect to PACS to query. AE Title: " + m_pacsDevice.getAETitle());
        delete m_pacsConnection;
        return PACSRequestStatus::QueryCanNotConnectToPACS;
    }

    // Figure out which of the accepted presentation contexts should be used
    m_presId = ASC_findAcceptedPresentationContextID(m_pacsConnection->getConnection(), FindStudyAbstractSyntax);
    if (m_presId == 0)
    {
        ERROR_LOG("PACS has not accepted the FINDStudyRootQueryRetrieveInformationModel study search level");
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
    //    OFCondition condition = DIMSE_findUser(m_pacsConnection->getConnection(), m_presId, &findRequest, dcmDatasetToQuery, foundMatchCallback, this, DIMSE_NONBLOCKING,
    //                                           Settings().getValue(InputOutputSettings::PACSConnectionTimeout).toInt(), &findResponse, &statusDetail);
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER < 365
    OFCondition condition = DIMSE_findUser(m_pacsConnection->getConnection(), m_presId, &findRequest, dcmDatasetToQuery, foundMatchCallback, this, DIMSE_NONBLOCKING,
                                           Settings().getValue(InputOutputSettings::PACSConnectionTimeout).toInt(), &findResponse, &statusDetail);
#else //if  PACKAGE_VERSION_NUMBER == 365
    int responseCount = 0;
    OFCondition condition = DIMSE_findUser(m_pacsConnection->getConnection(), m_presId, &findRequest, dcmDatasetToQuery,responseCount, foundMatchCallback,
                                           this,DIMSE_NONBLOCKING,Settings().getValue(InputOutputSettings::PACSConnectionTimeout).toInt(), &findResponse, &statusDetail);
#endif
#endif
    m_pacsConnection->disconnect();

    if (!condition.good())
    {
        ERROR_LOG(QString("Error querying PACS %1, error description: %2").arg(m_pacsDevice.getAETitle(), condition.text()));
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
    /// We indicate that the query must be canceled, the foundMatchCallback method,
    /// checks the flag each time it receives a DICOM result
    /// that matches the search mask
    m_cancelQuery = true;
}

void QueryPacs::cancelQuery(T_DIMSE_C_FindRQ *request)
{
    INFO_LOG(QString("Please cancel PACS %1 for the current query").arg(m_pacsDevice.getAETitle()));

    // All PACS are required by DICOM Conformance to implement the cancellation
    OFCondition cond = DIMSE_sendCancelRequest(m_pacsConnection->getConnection(), m_presId, request->MessageID);
    if (cond.bad())
    {
        ERROR_LOG("The following error occurred while canceling the query: " + QString(cond.text()));
        INFO_LOG(QString("I abort the connection to the PACS %1").arg(m_pacsDevice.getAETitle()));

        // If there was an error asking you to cancel, we aborted the association, so we are sure to cancel the query
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

    /// TODO: If we do an image level search we will insert the same series as
    /// many times as we have images, we should check if it already contains
    /// the series the list before adding it
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
    // In PS 3.4, section C.4.1.1.4, table C.4-1 we can find a description of the errors.
    // For details on "related fields" see PS 3.7, Annex C - Status Type Enconding

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
        //The user has requested to cancel the download
        queryRequestStatus = PACSRequestStatus::QueryCancelled;
        break;

    default:
        queryRequestStatus = PACSRequestStatus::QueryUnknowStatus;
        break;
    }

    return queryRequestStatus;
}

}
