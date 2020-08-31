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

#include "relatedstudiesmanager.h"

#include "study.h"
#include "dicommask.h"
#include "patient.h"
#include "pacsmanager.h"
#include "queryscreen.h"
#include "singleton.h"
#include "pacsdevicemanager.h"
#include "logging.h"
#include "querypacsjob.h"
#include "inputoutputsettings.h"

namespace udg {

RelatedStudiesManager::RelatedStudiesManager()
{
    m_pacsManager = new PacsManager();
    m_studyInstanceUIDOfStudyToFindRelated = "invalid";

    Settings settings;
    m_searchRelatedStudiesByName = settings.getValue(InputOutputSettings::SearchRelatedStudiesByName).toBool();
}

RelatedStudiesManager::~RelatedStudiesManager()
{
    cancelCurrentQuery();
    deleteQueryResults();
}

void RelatedStudiesManager::queryMergedStudies(Patient *patient)
{
    INFO_LOG("Patient studies will be sought " + patient->getFullName() + " amb ID " + patient->getID());

    this->makeAsynchronousStudiesQuery(patient);
}

void RelatedStudiesManager::queryMergedPreviousStudies(Study *study)
{
    INFO_LOG("Previous patient studies will be sought " + study->getParentPatient()->getFullName() + " amb ID " + study->getParentPatient()->getID() +
             " of the study " + study->getInstanceUID() + "done to date " + study->getDate().toString());

    m_studyInstanceUIDOfStudyToFindRelated = study->getInstanceUID();

    this->makeAsynchronousStudiesQuery(study->getParentPatient(), study->getDate());
}

void RelatedStudiesManager::makeAsynchronousStudiesQuery(Patient *patient, QDate untilDate)
{
    initializeQuery();

    QList<PacsDevice> pacsDeviceListToQuery = PacsDeviceManager().getPACSList(PacsDeviceManager::PacsWithQueryRetrieveServiceEnabled, true);
    pacsDeviceListToQuery = PacsDeviceManager::removeDuplicateSamePACS(pacsDeviceListToQuery + getPACSRetrievedStudiesOfPatient(patient));

    if (pacsDeviceListToQuery.count() == 0)
    {
        // Otherwise there is no PACS for which to search by default we issue the queryFinished
        queryFinished();
        return;
    }

    QList<DicomMask> queryDicomMasksList = getDicomMasks(patient);

    if (queryDicomMasksList.count() == 0)
    {
        // If there is no query to do we end the search
        queryFinished();
    }
    else
    {
        // If they tell us they want the study's up to a date, we have to mark that date in the dicomMasks
        if (untilDate.isValid())
        {
            foreach (DicomMask dicomMask, queryDicomMasksList)
            {
                dicomMask.setStudyDate(QDate(), untilDate);
            }
        }

        foreach (const PacsDevice &pacsDevice, pacsDeviceListToQuery)
        {
            foreach (DicomMask queryDicomMask, queryDicomMasksList)
            {
                enqueueQueryPACSJobToPACSManagerAndConnectSignals(PACSJobPointer(new QueryPacsJob(pacsDevice, queryDicomMask, QueryPacsJob::study)));
            }
        }
    }
}

QList<Study*> RelatedStudiesManager::getStudiesFromDatabase(Patient *patient)
{
    QList<DicomMask> queryDicomMasksList = getDicomMasks(patient);
    LocalDatabaseManager database;
    QHash<QString, Study*> studies;

    foreach (const DicomMask &dicomMask, queryDicomMasksList)
    {
        foreach(Patient *p, database.queryPatientsAndStudies(dicomMask))
        {
            foreach (Study *study, p->getStudies())
            {
                studies.insert(study->getInstanceUID(), study);
            }
        }
    }
    return studies.values();
}

QList<DicomMask> RelatedStudiesManager::getDicomMasks(Patient *patient)
{
    QList<DicomMask> queryDicomMasksList;

    if (!patient->getID().isEmpty())
    {
        DicomMask maskQueryByID = getBasicDicomMask();
        maskQueryByID.setPatientID(patient->getID());
        queryDicomMasksList << maskQueryByID;
    }

    if (m_searchRelatedStudiesByName && !patient->getFullName().isEmpty())
    {
        DicomMask maskQueryByName = getBasicDicomMask();
        maskQueryByName.setPatientName(patient->getFullName());
        queryDicomMasksList << maskQueryByName;
    }

    return queryDicomMasksList;
}

void RelatedStudiesManager::initializeQuery()
{
    cancelCurrentQuery();

    /// Fem neteja de consultes anteriors
    deleteQueryResults();
    m_pacsDeviceIDErrorEmited.clear();
}

void RelatedStudiesManager::enqueueQueryPACSJobToPACSManagerAndConnectSignals(PACSJobPointer queryPACSJob)
{
    connect(queryPACSJob.data(), SIGNAL(PACSJobFinished(PACSJobPointer)), SLOT(queryPACSJobFinished(PACSJobPointer)));
    connect(queryPACSJob.data(), SIGNAL(PACSJobCancelled(PACSJobPointer)), SLOT(queryPACSJobCancelled(PACSJobPointer)));

    m_pacsManager->enqueuePACSJob(queryPACSJob);
    m_queryPACSJobPendingExecuteOrExecuting.insert(queryPACSJob->getPACSJobID(), queryPACSJob);
}

void RelatedStudiesManager::cancelCurrentQuery()
{
    foreach (PACSJobPointer queryPACSJob, m_queryPACSJobPendingExecuteOrExecuting)
    {
        m_pacsManager->requestCancelPACSJob(queryPACSJob);
        m_queryPACSJobPendingExecuteOrExecuting.remove(queryPACSJob->getPACSJobID());
    }

    m_studyInstanceUIDOfStudyToFindRelated = "invalid";
}

bool RelatedStudiesManager::isExecutingQueries()
{
    return !m_queryPACSJobPendingExecuteOrExecuting.isEmpty();
}

void RelatedStudiesManager::queryPACSJobCancelled(PACSJobPointer pacsJob)
{
    // This slot is also used in case any other class cancels a PACSJob of ours for us to find out
    QSharedPointer<QueryPacsJob> queryPACSJob = pacsJob.objectCast<QueryPacsJob>();

    if (queryPACSJob.isNull())
    {
        ERROR_LOG("The canceled PACSJob is not a QueryPACSJob");
    }
    else
    {
        m_queryPACSJobPendingExecuteOrExecuting.remove(queryPACSJob->getPACSJobID());

        if (m_queryPACSJobPendingExecuteOrExecuting.isEmpty())
        {
            queryFinished();
        }
    }
}

void RelatedStudiesManager::queryPACSJobFinished(PACSJobPointer pacsJob)
{
    QSharedPointer<QueryPacsJob> queryPACSJob = pacsJob.objectCast<QueryPacsJob>();

    if (queryPACSJob.isNull())
    {
        ERROR_LOG("The completed PACSJob is not a QueryPACSJob");
    }
    else
    {
        if (queryPACSJob->getStatus() == PACSRequestStatus::QueryOk)
        {
            mergeFoundStudiesInQuery(pacsJob);
        }
        else if (queryPACSJob->getStatus() != PACSRequestStatus::QueryCancelled)
        {
            errorQueringPACS(pacsJob);
        }

        m_queryPACSJobPendingExecuteOrExecuting.remove(queryPACSJob->getPACSJobID());

        if (m_queryPACSJobPendingExecuteOrExecuting.isEmpty())
        {
            queryFinished();
        }
    }
}

void RelatedStudiesManager::mergeFoundStudiesInQuery(PACSJobPointer queryPACSJob)
{
    if (queryPACSJob.objectCast<QueryPacsJob>()->getQueryLevel() != QueryPacsJob::study)
    {
        /// If the consultation was not of studies we are not interested, we only look for studies
        return;
    }

    foreach (Patient *patient, queryPACSJob.objectCast<QueryPacsJob>()->getPatientStudyList())
    {
        foreach (Study *study, patient->getStudies())
        {
            if (!isStudyInMergedStudyList(study) && !isMainStudy(study))
            {
                // If the study is not already on the list of added studies and it is not the same study for which we have been asked to
                // previous we add it
                m_mergedStudyList.append(study);
            }
        }
    }
}

void RelatedStudiesManager::errorQueringPACS(PACSJobPointer queryPACSJob)
{
    if (queryPACSJob.objectCast<QueryPacsJob>()->getStatus() != PACSRequestStatus::QueryOk &&
            queryPACSJob.objectCast<QueryPacsJob>()->getStatus() != PACSRequestStatus::QueryCancelled)
    {
        // Since we do two searches on the same pacs if one fails, the other will probably also fail to avoid sending
        // two error signals if both fail, since from the outside the number of queries must be transparent
        // which is done in the PACS, and they should receive a single error we check if we have the PACS ID in the signal list
        // of errors in issued PACS
        if (!m_pacsDeviceIDErrorEmited.contains(queryPACSJob->getPacsDevice().getID()))
        {
            m_pacsDeviceIDErrorEmited.append(queryPACSJob->getPacsDevice().getID());
            emit errorQueryingStudies(queryPACSJob->getPacsDevice());
        }
    }
}

void RelatedStudiesManager::queryFinished()
{
    /// When all the queries are finished is how much we issue with the previous studies found.
    /// We cannot broadcast the results we receive,
    /// because we have to merge the results received, so as not to have duplicates
    /// (Studies of the patient matiex that are in addition to a PACS)
    emit queryStudiesFinished(m_mergedStudyList);
}

bool RelatedStudiesManager::isStudyInMergedStudyList(Study *study)
{
    bool studyFoundInMergedList = false;

    foreach (Study *studyMerged, m_mergedStudyList)
    {
        if (study->getInstanceUID() == studyMerged->getInstanceUID())
        {
            studyFoundInMergedList = true;
            break;
        }
    }

    return studyFoundInMergedList;
}

bool RelatedStudiesManager::isMainStudy(Study *study)
{
    return study->getInstanceUID() == m_studyInstanceUIDOfStudyToFindRelated;
}

DicomMask RelatedStudiesManager::getBasicDicomMask()
{
    DicomMask dicomMask;

    /// We define the fields that the query must return
    dicomMask.setPatientName("");
    dicomMask.setPatientID("");
    dicomMask.setStudyID("");
    dicomMask.setStudyDescription("");
    dicomMask.setStudyModality("");
    dicomMask.setStudyDate(QDate(), QDate());
    dicomMask.setStudyTime(QTime(), QTime());
    dicomMask.setStudyInstanceUID("");

    return dicomMask;
}

RelatedStudiesManager::LoadStatus RelatedStudiesManager::loadStudy(Study *study)
{
    if (LocalDatabaseManager().studyExists(study->getInstanceUID()))
    {
        SingletonPointer<QueryScreen>::instance()->loadStudyFromDatabase(study->getInstanceUID());
        return Loaded;
    }
    else if (study->getDICOMSource().getRetrievePACS().count() > 0)
    {
        retrieveAndLoad(study, study->getDICOMSource().getRetrievePACS().at(0));

        return Retrieving;
    }
    else
    {
        return Failed;
    }
}

void RelatedStudiesManager::retrieve(Study *study, const PacsDevice &pacsDevice)
{
    retrieveAndApplyAction(study, pacsDevice, None);
}

void RelatedStudiesManager::retrieveAndLoad(Study *study, const PacsDevice &pacsDevice)
{
    retrieveAndApplyAction(study, pacsDevice, Load);
}

void RelatedStudiesManager::retrieveAndView(Study *study, const PacsDevice &pacsDevice)
{
    retrieveAndApplyAction(study, pacsDevice, View);
}

void RelatedStudiesManager::retrieveAndApplyAction(Study *study, const PacsDevice &pacsDevice, ActionsAfterRetrieve action)
{
    QInputOutputPacsWidget::ActionsAfterRetrieve queryScreenAction = QInputOutputPacsWidget::None;
    switch (action)
    {
    case None:
        queryScreenAction = QInputOutputPacsWidget::None;
        break;

    case View:
        queryScreenAction = QInputOutputPacsWidget::View;
        break;

    case Load:
        queryScreenAction = QInputOutputPacsWidget::Load;
        break;
    }
    
    QueryScreen *queryScreen = SingletonPointer<QueryScreen>::instance();
    queryScreen->retrieveStudy(queryScreenAction, pacsDevice, study);
    connect(queryScreen, SIGNAL(studyRetrieveFailed(QString)), SIGNAL(errorDownloadingStudy(QString)));
}

void RelatedStudiesManager::deleteQueryResults()
{
    QList<Patient*> patientsStudy;

    foreach (Study* study, m_mergedStudyList)
    {
        patientsStudy.append(study->getParentPatient());
    }

    qDeleteAll(m_mergedStudyList);
    qDeleteAll(patientsStudy);

    m_mergedStudyList.clear();
}

QList<PacsDevice> RelatedStudiesManager::getPACSRetrievedStudiesOfPatient(Patient *patient)
{
    QList<PacsDevice> pacsDeviceRetrievedStudies;

    foreach (Study *studyPatient, patient->getStudies())
    {
        pacsDeviceRetrievedStudies = PacsDeviceManager::removeDuplicateSamePACS(pacsDeviceRetrievedStudies + studyPatient->getDICOMSource().getRetrievePACS());
    }

    return pacsDeviceRetrievedStudies;
}

}
