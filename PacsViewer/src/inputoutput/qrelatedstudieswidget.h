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

#ifndef UDGQRELATEDSTUDIESWIDGET_H
#define UDGQRELATEDSTUDIESWIDGET_H

#include <QWidget>
#include <QSignalMapper>
#include <QPushButton>
#include <QLabel>
#include <QHash>
#include <QButtonGroup>
#include <QRadioButton>

class QTreeWidgetItem;

namespace udg {

class Study;
class RelatedStudiesManager;
class QueryScreen;
class Patient;
class QTreeWidgetWithSeparatorLine;

class QRelatedStudiesWidget : public QFrame {
    Q_OBJECT
public:
    QRelatedStudiesWidget(RelatedStudiesManager *relatedStudiesManager, QWidget *parent = 0);
    ~QRelatedStudiesWidget();

    /// Method for searching for patient-related studies.
    void searchStudiesOf(Patient *patient);

    /// Sets the study with the given studyUID as the current study.
    /// If the selected prior study is newer than this one, it is deselected.
    void setCurrentStudy(const QString &studyUID);

public slots:
    virtual void setVisible(bool visible);

    /// Update the list by checking those studies that are already in
    /// memory so that it is easy to identify them and they cannot be re-downloaded.
    void updateList();

    /// If no prior study is selected, it tries to select one taking into account the properties of the current study.
    /// It is currently only available for mammography and CR Thorax.
    void toggleComparativeMode();

signals:
    ///It is broadcast only when no other studies are already being downloaded.
    void downloadingStudies();

    /// It is broadcast when all the requested studies have been downloaded.
    void studiesDownloaded();

    /// Emit the StudyInstanceUID of the studies selected as the current and prior study.
    void workingStudiesChanged(const QString&, const QString&);

private:
    /// Creating connections
    void createConnections();
    /// Initialization of the QTreeWidget where the related studies will be displayed
    void initializeTree();
    /// Initialize the widget displayed when searching for related studies.
    void initializeLookingForStudiesWidget();
    /// Inserts a study into the tree and generates the container associated with that study.
    void insertStudyToTree(Study *study);
    /// Update the width of the QTreeWidget to make the horizontal scroll not appear and all info to be visible.
    void updateWidgetWidth();
    /// Update the height of the QTreeWidget to get as much content as possible.
    /// Note: The QRelatedStudiesWidget must be visible for it to work properly.
    void updateWidgetHeight();

    /// Methods to control when the signals \ sa downloadingStudies and \ sa studiesDownloaded;
    void increaseNumberOfDownladingStudies();
    void decreaseNumberOfDownladingStudies();

    void initializeSearch();

    /// Highlight the QTreeWidgetItem by putting the text it contains in bold
    void highlightQTreeWidgetItem(QTreeWidgetItem *item);

    /// Remove non-image modes from the list of modes
    QStringList removeNonImageModalities(const QStringList &studiesModalities);

    /// Whether to highlight the study in the QTreeWidget
    bool hasToHighlightStudy(Study *study);

    /// Make visible only those radio buttons that can be clicked taking into account
    /// the studies selected as the current and the prior at this time
    void updateVisibleCurrentRadioButtons();
    void updateVisiblePriorRadioButtons();

    /// Evaluate if both current and prior studies are loaded to notify the user has changed the working studies
    void notifyWorkingStudiesChangedIfReady();

    /// Insert the studies in the tree.
    void insertStudiesToTree(const QList<Study*> &studiesList);

private slots:
    /// Slot executed when the query is finished
    void queryStudiesFinished(const QList<Study*> &studiesList);

    /// Invokes the download and upload of the study identified with the provided uid.
    void retrieveAndLoadStudy(const QString &studyInstanceUID);

    /// They update the status of the object and the interface of the study in question.
    void studyRetrieveStarted(QString studyInstanceUID);
    void studyRetrieveFinished(QString studyInstanceUID);
    void studyRetrieveFailed(QString studyInstanceUID);
    void studyRetrieveCancelled(QString studyInstanceUID);

    /// Slot executed when a radio button is clicked.
    /// It will proceed to load the selected study an notify the working studies has changed.
    void currentStudyRadioButtonClicked(const QString &studyInstanceUID);
    void priorStudyRadioButtonClicked(const QString &studyInstanceUID);

    /// Slot executed when an study has added.
    void onStudyAdded(Study *study);

private:
    /// Enumeration created to determine if the signals that QueryScreen emits belong to any of the requests in this class
    enum Status { Initialized = 0, Pending = 1, Downloading = 2, Finished = 3, Failed = 4, Cancelled = 5 };
    enum Columns { CurrentStudy = 0, PriorStudy = 1, DownloadingStatus = 2, DownloadButton = 3, Modality = 4, Description = 5, Date = 6, Name = 7 };

    /// Container of objects associated with the study that serves to facilitate intercommunication
    struct StudyInfo
    {
        Study *study;
        QTreeWidgetItem *item;
        Status status;
        QLabel *statusIcon;
        QPushButton *downloadButton;
        QRadioButton *currentRadioButton;
        QRadioButton *priorRadioButton;
    };

    /// Structure that is responsible for storing the containers associated with each Study
    QHash<QString, StudyInfo*> m_infomationPerStudy;
    /// Widget used to display the list of related studies
    QTreeWidgetWithSeparatorLine *m_relatedStudiesTree;
    /// Widget that appears when querying possible related studies.
    QWidget *m_lookingForStudiesWidget;
    /// Object in charge of looking for related studies
    RelatedStudiesManager *m_relatedStudiesManager;
    /// Mapper used to know which download button is associated with which studio.
    QSignalMapper *m_signalMapper;
    /// Mapper used to know which download button is associated with which studio.
    QSignalMapper *m_currentStudySignalMapper;
    /// Mapper used to know which download button is associated with which studio.
    QSignalMapper *m_priorStudySignalMapper;
    /// Object used to invoke the download of studies.
    QueryScreen *m_queryScreen;
    /// Allows us to know which studies are being downloaded.
    int m_numberOfDownloadingStudies;
    /// Patient associated with the last search performed.
    Patient *m_patient;

    /// Button groups for current and prior radio buttons
    QButtonGroup m_currentStudyRadioGroup;
    QButtonGroup m_priorStudyRadioGroup;

    /// Working studies selected at a time
    QString m_studyInstanceUIDOfCurrentStudy;
    QString m_studyInstanceUIDOfPriorStudy;

    /// List with the modalities of the studies that we have to highlight
    QStringList m_modalitiesOfStudiesToHighlight;
};

}
#endif // UDGQRELATEDSTUDIESWIDGET_H
