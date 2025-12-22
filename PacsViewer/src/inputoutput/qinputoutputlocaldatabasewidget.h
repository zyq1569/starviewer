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


#ifndef UDGQINPUTOUTPUTLOCALDATABASEWIDGET_H
#define UDGQINPUTOUTPUTLOCALDATABASEWIDGET_H

#include "ui_qinputoutputlocaldatabasewidgetbase.h"

#include "localdatabasemanager.h"
#include "qdeleteoldstudiesthread.h"
#include "dicommask.h"
#include "pacsdevice.h"
#include "pacsjob.h"

#include <QMenu>

// Fordward declarations
class QString;

namespace udg {

// Fordward declarations
class Patient;
class StatsWatcher;
class QCreateDicomdir;
class Study;
class QWidgetSelectPacsToStoreDicomImage;
class SendDICOMFilesToPACSJob;
class PacsManager;

/**
Widget in which it controls the input / output operations of the local database
*/
class QInputOutputLocalDatabaseWidget : public QWidget, private Ui::QInputOutputLocalDatabaseWidgetBase {
    Q_OBJECT

public:
    QInputOutputLocalDatabaseWidget(QWidget *parent = 0);
    ~QInputOutputLocalDatabaseWidget();

    /// We specify the instance of PacsManager used for operations with PACS
    void setPacsManager(PacsManager *pacsManager);

    /// Check out the open dicomdir studies that meet the search mask
    void queryStudy(DicomMask queryMask);

    /// We pass a point to the interface that creates dicomdir, so that it can communicate to ask
    /// if the study before being deleted is in the list of studies to create a new dicomdir, or to indicate
    /// that has been added asked to add a new study to study list to create dicomdir
    /// EVERYTHING should you look for a more elegant way to communicate the two classes, do a singleton of QCreateDicomdir?
    void setQCreateDicomdir(QCreateDicomdir *qcreateDicomdir);

    ///Clears the search results shown
    void clear();

    ///Sends images passed by parameter to the specified PACS
    void sendDICOMFilesToPACS(PacsDevice pacsDevice, QList<Image*> images);

public slots:

    /// Emet signal selectedPatients indicating that studies have been selected for display
    /// We add a third parameter "loadOnly" that will tell us if we only want to load the data without having to do a "view"
    /// This is how we differentiate the cases in which we want to load patient data "in the background" (we only merge patient data and that's it,
    /// without applying visible changes on the interface) useful for cases of loading previous studies, for example.
    void view(QList<DicomMask> dicomMaskStudiesToView, bool loadOnly = false);
    void view(QString studyInstanceUID, bool loadOnly = false);

    ///Add the study with the Study Instance UID passed as a parameter to the Widget
    void addStudyToQStudyTreeWidget(QString studyInstanceUID);

    /// Remove the study with the Instance Study UID passed by QStudyTreeWidget parameter
    void removeStudyFromQStudyTreeWidget(QString studyInstanceUID);

signals:
    /// Signal issued to indicate that search fields are cleared
    void clearSearchTexts();

    /// Signal that is emitted to indicate that a study has been requested to be viewed
    /// We add a second parameter to indicate if we want to do
    /// a "view" or only load the patient data in the background and enough
    void viewPatients(QList<Patient*> patientsToView, bool onlyLoad);

private:
    /// Creates connections between signals and slots
    void createConnections();

    /// Generates the context menu that appears when we right-click on an item in the StudyTreeWidget
    void createContextMenuQStudyTreeWidget();

    /// Show the error that occurred with the operations in the database
    bool showDatabaseManagerError(LocalDatabaseManager::LastError error, const QString &doingWhat = "");

    /// Delete old studies
    // TODO This responsibility for deleting old studies when starting the application should
    // move to another location, is not responsible for this inference
    void deleteOldStudies();

    /// Returns all images of a patient
    QList<Image*> getAllImagesFromPatient(Patient *patient);

private slots:
    /// Displays series from a study, queries them in the dicomdir, and displays them in the tree widget
    void requestedSeriesOfStudy(Study *studyRequestedSeries);

    /// Show SeriesListWidget preview of currently selected series in QStudyTreeWidget
    void setSeriesToSeriesListWidget(Study *study);

    void currentSeriesOfQStudyTreeWidgetChanged(Series *series);

    void currentSeriesChangedOfQSeriesListWidget(const QString &studyInstanceUID, const QString &seriesInstanceUID);

    /// Deletes selected studies in the QStudyTreeWidgetView from the database
    void deleteSelectedItemsFromLocalDatabase();

    /// Slot that fires when the thread that deleted old studies is finished,
    /// this slot checks that no deletion error has occurred
    /// the old studies
    void deleteOldStudiesThreadFinished();

    /// Add the selected studies to the list of studies to convert to dicomdir
    void addSelectedStudiesToCreateDicomdirList();

    /// View the QSeriesListWidget double-click study
    void viewFromQSeriesListWidget(QString studyInstanceUID, QString seriesInstanceUID);

    /// View the selected studies in the QStudyTreeWidget
    void viewFromQStudyTreeWidget();

    ///It signals that the selected studies should be saved to the PACS
    void selectedStudiesStoreToPacs();

    /// Save the position of the splitter when they have moved it
    void qSplitterPositionChanged();

    ///Saves the selected studies to the PACS that the user has selected
    void sendSelectedStudiesToSelectedPacs();

    ///Slot that activates when a SendDICOMFilesToPACSJob ends
    void sendDICOMFilesToPACSJobFinished(PACSJobPointer);

    /// Every time we find a new Job we check if it is a RetrieveDICOMFileFromPACSJob
    ///  and if so we connect with the Signal StudyFromCacheWillBeDeleted
    /// if you delete studies from the cache you can remove them from the QStudyTreeWidget
    void newPACSJobEnqueued(PACSJobPointer);

private:
    QMenu m_contextMenuQStudyTreeWidget;
    QDeleteOldStudiesThread m_qdeleteOldStudiesThread;
    QCreateDicomdir *m_qcreateDicomdir;
    StatsWatcher *m_statsWatcher;
    QWidgetSelectPacsToStoreDicomImage *m_qwidgetSelectPacsToStoreDicomImage;
    PacsManager *m_pacsManager;
};

};// end namespace udg

#endif
