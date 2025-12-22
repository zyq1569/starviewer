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

#ifndef UDGQPOPURISREQUESTSCREEN_H
#define UDGQPOPURISREQUESTSCREEN_H

#include "qnotificationpopup.h"

#include "pacsjob.h"

namespace udg {

class RetrieveDICOMFilesFromPACSJob;
class Patient;
class Study;

/**
    Class showing a PopUp indicating that a request has been received from the RIS to download a study
  */
class QPopUpRISRequestsScreen : public QNotificationPopup {
    Q_OBJECT
public:
    /// Constructor
    QPopUpRISRequestsScreen(QWidget *parent = 0);

    /// Destructor
    ~QPopUpRISRequestsScreen();

    ///It is indicated that studies with a certain accession number are being searched in PACS
    void queryStudiesByAccessionNumberStarted();

    /// We added a downloadable study that was requested through the rIS
    void addStudyToRetrieveFromPACSByAccessionNumber(PACSJobPointer retrieveDICOMFilesFromPACSJob);

    /// We add a study that has been obtained from the PACS database ask yourself in a request of the RIS
    /// HACK: As of right now we have no way of tracking the
    /// study requests to the database, we add this method which will show
    /// this study is already downloaded and will add it to the total studies
    void addStudyRetrievedFromDatabaseByAccessionNumber(Study *study);

    /// Mostra en el PopUp que no s'ha trobat cap estudi que compleixi els criteris de cerca
    void showNotStudiesFoundMessage();

    /// Specifies the seconds that the PopUp should take to hide
    /// after all studies have been downloaded
    /// Otherwise specified is 5000 ms
    void setTimeOutToHidePopUpAfterStudiesHaveBeenRetrieved(int timeOutms);

private slots:
    /// It is indicated that the download of the studies due to a request from the RIS has ended
    void retrieveDICOMFilesFromPACSJobFinished(PACSJobPointer pacsJob);

    /// It is stated that the download of the studies requested by the RIS has failed or has been canceled
    void retrieveDICOMFilesFromPACSJobCancelled(PACSJobPointer pacsJob);

private:
    ///Updates the download status on the screen
    void refreshScreenRetrieveStatus(Study *study);

    /// Indicates that the download is complete, indicates
    /// number of studies downloaded and after 5 seconds hides the PopUp
    void showRetrieveFinished();

private:
    QList<int> m_pacsJobIDOfStudiesToRetrieve;
    int m_numberOfStudiesRetrieved;
    int m_numberOfStudiesFailedToRetrieve;

    ///Save the number of studies to be downloaded
    int m_numberOfStudiesToRetrieve;
};

};

#endif
