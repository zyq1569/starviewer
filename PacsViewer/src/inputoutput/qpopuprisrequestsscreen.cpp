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

#include "qpopuprisrequestsscreen.h"

#include "qpopuprisrequestsscreen.h"
#include "logging.h"
#include "starviewerapplication.h"
#include "retrievedicomfilesfrompacsjob.h"
#include "study.h"
#include "pacsrequeststatus.h"
#include "patient.h"

namespace udg {

QPopUpRISRequestsScreen::QPopUpRISRequestsScreen(QWidget *parent)
    : QNotificationPopup(parent)
{
    setOngoingOperationText(tr("%1 will proceed to retrieve it.").arg(ApplicationNameString));
}

void QPopUpRISRequestsScreen::queryStudiesByAccessionNumberStarted()
{
    // If another request arrives while the Timer to hide the PopUp has been activated or is being hidden,
    // we must make it not hide by
    // show the new request
    cancelTriggeredAnimations();

    setHeadline(tr("%1 has received a request from RIS to retrieve studies.").arg(ApplicationNameString));
    setOngoingOperationText(tr("Querying PACS..."));
    showOngoingOperationAnimation();

    m_pacsJobIDOfStudiesToRetrieve.clear();
    m_numberOfStudiesRetrieved = 0;
    m_numberOfStudiesToRetrieve = 0;
    m_numberOfStudiesFailedToRetrieve = 0;
}

void QPopUpRISRequestsScreen::addStudyToRetrieveFromPACSByAccessionNumber(PACSJobPointer retrieveDICOMFilesFromPACSJob)
{
    m_numberOfStudiesToRetrieve++;

    m_pacsJobIDOfStudiesToRetrieve.append(retrieveDICOMFilesFromPACSJob->getPACSJobID());
    refreshScreenRetrieveStatus(retrieveDICOMFilesFromPACSJob.objectCast<RetrieveDICOMFilesFromPACSJob>()->getStudyToRetrieveDICOMFiles());

    connect(retrieveDICOMFilesFromPACSJob.data(), SIGNAL(PACSJobFinished(PACSJobPointer)), SLOT(retrieveDICOMFilesFromPACSJobFinished(PACSJobPointer)));
    connect(retrieveDICOMFilesFromPACSJob.data(), SIGNAL(PACSJobCancelled(PACSJobPointer)), SLOT(retrieveDICOMFilesFromPACSJobCancelled(PACSJobPointer)));
}

void QPopUpRISRequestsScreen::addStudyRetrievedFromDatabaseByAccessionNumber(Study *study)
{
    m_numberOfStudiesToRetrieve++;
    m_numberOfStudiesRetrieved++;
    refreshScreenRetrieveStatus(study);
}

void QPopUpRISRequestsScreen::retrieveDICOMFilesFromPACSJobFinished(PACSJobPointer pacsJob)
{
    QSharedPointer<RetrieveDICOMFilesFromPACSJob> retrieveDICOMFilesFromPACSJob = pacsJob.objectCast<RetrieveDICOMFilesFromPACSJob>();

    if (retrieveDICOMFilesFromPACSJob.isNull())
    {
        ERROR_LOG("The completed PACSJob is not a RetrieveDICOMFilesFromPACSJob");
    }
    else
    {
        // If when the studio has been downloaded the PopUp has not yet moved down to the left, what we do is force them to move without waiting
        // Timer timeout, otherwise it could happen that we already have the viewfinders with the studio loaded and the PopUp still appears in the center of the annoying screen
        // to the user
        if (isMoveAnimationOnDelayPeriod())
        {
            cancelTriggeredAnimations();
            startMoveAnimation();
        }

        if (m_pacsJobIDOfStudiesToRetrieve.contains(retrieveDICOMFilesFromPACSJob->getPACSJobID()))
        {
            // If it is not in the list of PACSJob to download it means that it is from another RIS request that has been crushed by the current one
            // Because the QPopUpRisRequestScreen only follows the last RIS request we ignore them
            if (retrieveDICOMFilesFromPACSJob->getStatus() == PACSRequestStatus::RetrieveOk ||
                    retrieveDICOMFilesFromPACSJob->getStatus() == PACSRequestStatus::RetrieveSomeDICOMFilesFailed)
            {
                m_numberOfStudiesRetrieved++;
            }
            else
            {
                m_numberOfStudiesFailedToRetrieve++;
            }

            refreshScreenRetrieveStatus(retrieveDICOMFilesFromPACSJob->getStudyToRetrieveDICOMFiles());
        }
    }
}

void QPopUpRISRequestsScreen::retrieveDICOMFilesFromPACSJobCancelled(PACSJobPointer pacsJob)
{
    QSharedPointer<RetrieveDICOMFilesFromPACSJob> retrieveDICOMFilesFromPACSJob = pacsJob.objectCast<RetrieveDICOMFilesFromPACSJob>();

    if (retrieveDICOMFilesFromPACSJob.isNull())
    {
        ERROR_LOG("The completed PACSJob is not a RetrieveDICOMFilesFromPACSJob");
    }
    else
    {
        /// If the download has failed or has been canceled, we will remove the study from the list of studies to be downloaded
        if (m_pacsJobIDOfStudiesToRetrieve.removeOne(retrieveDICOMFilesFromPACSJob->getPACSJobID()))
        {
            m_numberOfStudiesToRetrieve--;
            /// If it is not in the list of PACSJob to download it means that it is from another
            ///RIS request that has been crushed by the current one
            /// Because the QPopUpRisRequestScreen only follows the last RIS request we ignore them
            refreshScreenRetrieveStatus(retrieveDICOMFilesFromPACSJob->getStudyToRetrieveDICOMFiles());
        }
    }
}

void QPopUpRISRequestsScreen::refreshScreenRetrieveStatus(Study *study)
{
    Q_UNUSED(study)

    if (m_numberOfStudiesRetrieved + m_numberOfStudiesFailedToRetrieve < m_numberOfStudiesToRetrieve)
    {
        setOngoingOperationText(tr("Retrieving study %1 of %2.").arg(m_numberOfStudiesRetrieved + m_numberOfStudiesFailedToRetrieve + 1).arg(m_numberOfStudiesToRetrieve));
    }
    else
    {
        showRetrieveFinished();
        hideWithDelay();
    }
}

void QPopUpRISRequestsScreen::setTimeOutToHidePopUpAfterStudiesHaveBeenRetrieved(int timeOutms)
{
    setHideDelay(timeOutms);
}

void QPopUpRISRequestsScreen::showNotStudiesFoundMessage()
{
    setOngoingOperationText(tr("No studies found."));
    hideOngoingOperationAnimation();
    hideWithDelay();
}

void QPopUpRISRequestsScreen::showRetrieveFinished()
{
    hideOngoingOperationAnimation();

    if (m_numberOfStudiesRetrieved == 0)
    {
        if (m_numberOfStudiesFailedToRetrieve == 0)
        {
            setOngoingOperationText(tr("No studies found.").arg(m_numberOfStudiesRetrieved));
        }
        else
        {
            setOngoingOperationText(tr("Unable to retrieve requested studies.").arg(ApplicationNameString));
        }
    }
    else if (m_numberOfStudiesFailedToRetrieve == 0)
    {
        if (m_numberOfStudiesRetrieved == 1)
        {
            setOngoingOperationText(tr("%1 study retrieved.").arg(m_numberOfStudiesRetrieved));
        }
        else
        {
            setOngoingOperationText(tr("%1 studies retrieved.").arg(m_numberOfStudiesRetrieved));
        }
    }
    else
    {
        setOngoingOperationText(tr("%1 studies retrieved, %2 failed.").arg(QString::number(m_numberOfStudiesRetrieved), QString::number(m_numberOfStudiesFailedToRetrieve)));
    }
}

QPopUpRISRequestsScreen::~QPopUpRISRequestsScreen()
{
}

};
