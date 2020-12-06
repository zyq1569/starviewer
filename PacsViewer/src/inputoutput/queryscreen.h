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

#ifndef UDGQUERYSCREEN_H
#define UDGQUERYSCREEN_H

#include "ui_queryscreenbase.h"

namespace udg {

class Status;
class QCreateDicomdir;
class DicomMask;
class QOperationStateScreen;
class StatsWatcher;
class RISRequestManager;
class PacsDevice;
class PacsManager;

/**
Aquesta classe crea la interfície princial de cerca,
i connecta amb el PACS i la bd dades local per donar els resultats finals
*/
class QueryScreen : public QDialog, private Ui::QueryScreenBase {
    Q_OBJECT
public:
    QueryScreen(QWidget *parent = 0);
    ~QueryScreen();

    /// Download the study requested in the Indicated PACS.
    /// The signal indicates the status of the download
    /// signal: studyRetrieveStarted (QString studyInstanceUID) -> Indicates that the study download has started
    /// signal: studyRetrieveFinished (QString studyInstanceUID) -> Indicates that the study download has finished
    /// signal: errorRetrievingStudy (QString studyInstanceUID) -> Indicates that a download error has occurred
    ///
    /// ATTENTION !! Because this class is a singleton there is the possibility of making signals from studies requested by others
    /// classes, therefore each class that uses this method and connects to the signals described above must maintain a
    /// contains a list of the requests you have made to find out if that signal affects you or not.
    ///
    /// @param actionAfterRetrieve Indicates the action to take once the study is downloaded
    /// @param pacsDevice PACS from where the study is downloaded
    /// @param study Object Study with the information of the study we want to download
    void retrieveStudy(QInputOutputPacsWidget::ActionsAfterRetrieve actionAfterRetrieve, const PacsDevice &pacsDevice, Study *study);

public slots:
    /// Obre un dicomdir
    void openDicomdir();

    /// Updates the settings that the user has been able to change from the settings dialog
    void updateConfiguration(const QString &configuration);

    /// If the window is not visible or is deleted from another,
    /// makes it visible and brings it in front of the windows.
    void bringToFront();

    /// Displays the PACS tab.
    /// EVERYTHING Leave the search fields as they were, make a clear or set default values?
    /// A "bool clear" parameter could be passed with default value (false, for example)
    void showPACSTab();

    ///Show all studies locally and reset search fields
    void showLocalExams();

    ///Clear the LineEdits from the form
    void clearTexts();

    /// Saves objects that met the mask passed by study parameters
    /// passed by parameter to the first PACS which is found as the default PACS
    void sendDicomObjectsToPacs(PacsDevice pacsDevice, QList<Image*> images);

    /// It communicates with the database widget and displays a study downloaded from the PACS
    void viewStudyFromDatabase(QString studyInstanceUID);

    /// Request a downloaded studio to be uploaded. Useful for cases such as pre-loading, for example.
    void loadStudyFromDatabase(QString studyInstanceUID);

signals:
    /// Signal that tells us which patients have been selected to visualize
    /// We add a second parameter to indicate whether these patients should be loaded only and whether they should be visualized
    void selectedPatients(QList<Patient*> selectedPatients, bool loadOnly = false);

    /// .
    void setPatientsThumbnail(QList<Patient*> selectedPatients, bool loadOnly = false);

    /// Indicates that a download error has occurred
    /// of a study requested through the public retrieveStudy method
    void studyRetrieveFailed(QString studyInstanceUID);

    /// Indicates that a single studio download is complete
    /// tendered through the public retrieveStudy method
    void studyRetrieveFinished(QString studyInstanceUID);

    /// Indicates that the download of a single study has begun
    /// tendered through the public retrieveStudy method
    void studyRetrieveStarted(QString studyInstanceUID);

    /// Indicates that a single studio download has been canceled
    /// tendered through the public retrieveStudy method
    void studyRetrieveCancelled(QString studyInstanceUID);

    ///It is emitted when the window is closed
    void closed();

protected:
    /// Event that activates when closing when they receive a closing event
    /// @param closing event
    void closeEvent(QCloseEvent *event);

private slots:
    /// Choose where to search, whether locally or PACS
    void searchStudy();

    /// When changing the page of the tab we have to change some parameters, such as activating the Retrieve button, etc.
    /// @param index of the tab to which it has been changed
    void refreshTab(int index);

    ///Show / hide advanced search fields
    void setAdvancedSearchVisible(bool visible);

#ifndef STARVIEWER_LITE
    /// Displays the QOperationStateScreen screen
    void showOperationStateScreen();
#endif

    /// Notify which studies have been chosen to upload and / or view
    /// We add a second parameter to indicate if we want to view or only load the data.
    void viewPatients(QList<Patient*>, bool loadOnly = false);

    /// Slot que s'activa quan s'ha produït un error al descarregar un estudi
    void studyRetrieveFailedSlot(QString studyInstanceUID);

    /// Slot that is activated when the download of a studio has finished
    void studyRetrieveFinishedSlot(QString studyInstanceUID);

    /// Slot that is activated when the download of a study begins
    void studyRetrieveStartedSlot(QString studyInstanceUID);

    ///Slot that activates when a studio download is canceled
    void studyRetrieveCancelledSlot(QString studyInstanceUID);

    /// Slot that is activated when a new PACSJob has been found if this one
    /// is uploading or downloading files showing the animated gif indicating that
    /// requests are being processed
    void newPACSJobEnqueued(PACSJobPointer pacsJob);

    /// Slot that is activated when a PACSJob has ended,
    /// checks if PacsManager is running more download or upload jobs
    /// if you are not running any more, the animated gif is hidden indicating that a request is being processed
    void pacsJobFinishedOrCancelled(PACSJobPointer pacsJob);

    /// Update according to the tab in which we find the visibility of the list of PACS
    /// The list can only be enabled or disabled when we are on the PACS tab
    void updatePACSNodesVisibility();

private:
#ifdef STARVIEWER_LITE
    // Al fer remove de la pestanya del pacs es canvia el index de cada tab, per això hem de redefinir-lo pel cas de StarviewerLite
    enum TabType { PACSQueryTab = -1, LocalDataBaseTab = 0, DICOMDIRTab = 1 };
#else
    enum TabType { LocalDataBaseTab = 0, PACSQueryTab = 1, DICOMDIRTab = 2 };
#endif

    /// Connect the relevant signals and slots
    void createConnections();

    /// Build the input mask for dicom from search widgets
    /// @return returns the mask of a dicom object
    DicomMask buildDicomMask();

    /// Check the requirements for using QueryScreen
    void checkRequirements();

    /// Checks the integrity of the database and images,
    /// checking that last time the starviewer
    /// would not have closed with a half-downloaded studio,
    /// and if so delete the half-downloaded study and leave the database in an intact state
    void checkDatabaseImageIntegrity();

    /// Check that the port through which the
    /// objects I say to download are not being used
    /// for no other application, if so give an error message
    void checkIncomingConnectionsPacsPortNotInUse();

    ///It initializes the necessary variables, it is called by the constructor
    void initialize();

    ///Read and apply configuration data
    void readSettings();

    /// Save the QueryScreen settings
    void writeSettings();

private:
    QCreateDicomdir *m_qcreateDicomdir;

    StatsWatcher *m_statsWatcher;

    /// List to control the download of which studies has been requested
    QStringList m_studyRequestedToRetrieveFromPublicMethod;
#ifndef STARVIEWER_LITE
    QOperationStateScreen *m_operationStateScreen;
    RISRequestManager *m_risRequestManager;
    PacsManager *m_pacsManager;
#endif

    /// Indicates when jobs are pending
    /// (waiting to run or running)
    int m_PACSJobsPendingToFinish;
};

};

#endif
