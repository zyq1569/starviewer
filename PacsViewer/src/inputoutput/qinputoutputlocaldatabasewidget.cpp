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

#include "qinputoutputlocaldatabasewidget.h"

#include <QMessageBox>
#include <QShortcut>

#include "logging.h"
#include "starviewerapplication.h"
#include "dicommask.h"
#include "patient.h"
#include "statswatcher.h"
#include "qcreatedicomdir.h"
#include "inputoutputsettings.h"
#include "qwidgetselectpacstostoredicomimage.h"
#include "pacsmanager.h"
#include "senddicomfilestopacsjob.h"
#include "retrievedicomfilesfrompacsjob.h"
#include "shortcutmanager.h"
#include "usermessage.h"

namespace udg {

QInputOutputLocalDatabaseWidget::QInputOutputLocalDatabaseWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    createContextMenuQStudyTreeWidget();

    Settings settings;
    settings.restoreColumnsWidths(InputOutputSettings::LocalDatabaseStudyList, m_studyTreeWidget->getQTreeWidget());
    settings.restoreGeometry(InputOutputSettings::LocalDatabaseSplitterState, m_StudyTreeSeriesListQSplitter);

    QStudyTreeWidget::ColumnIndex sortByColumn = (QStudyTreeWidget::ColumnIndex)
            settings.getValue(InputOutputSettings::LocalDatabaseStudyListSortByColumn).toInt();

    Qt::SortOrder sortOrderColumn = (Qt::SortOrder) settings.getValue(InputOutputSettings::LocalDatabaseStudyListSortOrder).toInt();
    m_studyTreeWidget->setSortByColumn (sortByColumn, sortOrderColumn);

    m_statsWatcher = new StatsWatcher("QueryInputOutputLocalDatabaseWidget", this);
    m_statsWatcher->addClicksCounter(m_viewButton);

    m_studyTreeWidget->setMaximumExpandTreeItemsLevel(QStudyTreeWidget::SeriesLevel);
    m_studyTreeWidget->setUseDICOMSourceToDiscriminateStudies(false);

    m_qwidgetSelectPacsToStoreDicomImage = new QWidgetSelectPacsToStoreDicomImage();

    createConnections();

    /// Delete old studies from the cache.
    /// ATTENTION! It must be done after createConnections because otherwise
    /// we will not have connected to the signal to control the errors when deleting studies
    /// EVERYTHING: This should move out of here should not be the responsibility of this class
    deleteOldStudies();
}

QInputOutputLocalDatabaseWidget::~QInputOutputLocalDatabaseWidget()
{
    Settings settings;
    settings.saveColumnsWidths(InputOutputSettings::LocalDatabaseStudyList, m_studyTreeWidget->getQTreeWidget());

    //We save by which column the list of studies is ordered and in what order
    settings.setValue(InputOutputSettings::LocalDatabaseStudyListSortByColumn, m_studyTreeWidget->getSortColumn());
    settings.setValue(InputOutputSettings::LocalDatabaseStudyListSortOrder, m_studyTreeWidget->getSortOrderColumn());

}

void QInputOutputLocalDatabaseWidget::createConnections()
{
    connect(m_studyTreeWidget, SIGNAL(requestedSeriesOfStudy(Study*)), SLOT(requestedSeriesOfStudy(Study*)));

    connect(m_studyTreeWidget, SIGNAL(studyDoubleClicked()), SLOT(viewFromQStudyTreeWidget()));
    connect(m_studyTreeWidget, SIGNAL(seriesDoubleClicked()), SLOT(viewFromQStudyTreeWidget()));
    connect(m_studyTreeWidget, SIGNAL(imageDoubleClicked()), SLOT(viewFromQStudyTreeWidget()));

    connect(m_viewButton, SIGNAL(clicked()), SLOT(viewFromQStudyTreeWidget()));

    connect(m_seriesThumbnailPreviewWidget, SIGNAL(seriesThumbnailClicked(QString,QString)), this, SLOT(currentSeriesChangedOfQSeriesListWidget(QString, QString)));
    connect(m_seriesThumbnailPreviewWidget, SIGNAL(seriesThumbnailDoubleClicked(QString,QString)), SLOT(viewFromQSeriesListWidget(QString, QString)));
    connect(m_studyTreeWidget, SIGNAL(currentStudyChanged(Study*)), SLOT(setSeriesToSeriesListWidget(Study*)));
    connect(m_studyTreeWidget, SIGNAL(currentSeriesChanged(Series*)), SLOT(currentSeriesOfQStudyTreeWidgetChanged(Series*)));
    //If we go from having a selected item to not having one we tell the seriesListWidget not to show any preview
    connect(m_studyTreeWidget, SIGNAL(notCurrentItemSelected()), m_seriesThumbnailPreviewWidget, SLOT(clear()));

    // Connects to the signal indicating that the thread to delete old studies has ended
    connect(&m_qdeleteOldStudiesThread, SIGNAL(finished()), SLOT(deleteOldStudiesThreadFinished()));

    /// If we move the QSplitter we capture the signal to save its position
    connect(m_StudyTreeSeriesListQSplitter, SIGNAL(splitterMoved (int, int)), SLOT(qSplitterPositionChanged()));
    connect(m_qwidgetSelectPacsToStoreDicomImage, SIGNAL(selectedPacsToStore()), SLOT(sendSelectedStudiesToSelectedPacs()));
}

void QInputOutputLocalDatabaseWidget::createContextMenuQStudyTreeWidget()
{
    QAction *action;

    action = m_contextMenuQStudyTreeWidget.addAction(QIcon(":/images/icons/visibility.svg"), tr("&View"), this, SLOT(viewFromQStudyTreeWidget()));
    action->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::ViewSelectedStudies));
    (void) new QShortcut(action->shortcut(), this, SLOT(viewFromQStudyTreeWidget()));

    action = m_contextMenuQStudyTreeWidget.addAction(QIcon(":/images/icons/edit-delete.svg"), tr("&Delete"), this, SLOT(deleteSelectedItemsFromLocalDatabase()));
    action->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::DeleteSelectedLocalDatabaseStudies));
    (void) new QShortcut(action->shortcut(), this, SLOT(deleteSelectedItemsFromLocalDatabase()));

#ifndef STARVIEWER_LITE
    action = m_contextMenuQStudyTreeWidget.addAction(tr("Send to DICOMDIR List"), this, SLOT(addSelectedStudiesToCreateDicomdirList()));
    action->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::SendSelectedStudiesToDICOMDIRList));
    (void) new QShortcut(action->shortcut(), this, SLOT(addSelectedStudiesToCreateDicomdirList()));

    action = m_contextMenuQStudyTreeWidget.addAction(QIcon(":/images/icons/upload-media.svg"), tr("Send to PACS"), this, SLOT(selectedStudiesStoreToPacs()));
    action->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::StoreSelectedStudiesToPACS));
    (void) new QShortcut(action->shortcut(), this, SLOT(selectedStudiesStoreToPacs()));
#endif
    // We specify that it is the menu for the cache
    m_studyTreeWidget->setContextMenu(&m_contextMenuQStudyTreeWidget);
}

/// TODO a more elegant way of communicating the two
/// classes should be sought, to make a singletton of QCreateDicomdir?
void QInputOutputLocalDatabaseWidget::setQCreateDicomdir(QCreateDicomdir *qcreateDicomdir)
{
    m_qcreateDicomdir = qcreateDicomdir;
}

void QInputOutputLocalDatabaseWidget::clear()
{
    m_studyTreeWidget->clear();
    m_seriesThumbnailPreviewWidget->clear();
}

void QInputOutputLocalDatabaseWidget::setPacsManager(PacsManager *pacsManager)
{
    m_pacsManager = pacsManager;
    connect(pacsManager, SIGNAL(newPACSJobEnqueued(PACSJobPointer)), SLOT(newPACSJobEnqueued(PACSJobPointer)));
}

void QInputOutputLocalDatabaseWidget::queryStudy(DicomMask queryMask)
{
    LocalDatabaseManager localDatabaseManager;
    QList<Patient*> patientStudyList;

    StatsWatcher::log("Search for studies in the local database with parameters: " + queryMask.getFilledMaskFields());
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    clear();

    patientStudyList = localDatabaseManager.queryPatientsAndStudies(queryMask);

    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    // This method apart from being called when the user clicks the search button, is also called al
    // constructor of this class, so that when starting the application the list of studies is already displayed
    // that is in the local database. If the method does not find any study in the local database
    // throws the message that no studies have been found, but as it is not suitable, in this case
    // calls from the constructor to display the message that no studies were found when starting the application, which
    // is done to check that the window is active to launch the message. If the window is not active
    // means that the method has been invoked from the constructor
    if (patientStudyList.isEmpty() && isActiveWindow())
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, ApplicationNameString, tr("No study match found."));
    }
    else
    {
        //The list of studies is displayed
        m_studyTreeWidget->insertPatientList(patientStudyList);
        QApplication::restoreOverrideCursor();
    }
}

void QInputOutputLocalDatabaseWidget::addStudyToQStudyTreeWidget(QString studyUID)
{
    LocalDatabaseManager localDatabaseManager;
    DicomMask studyMask;
    QList<Patient*> patientList;

    studyMask.setStudyInstanceUID(studyUID);
    patientList = localDatabaseManager.queryPatientsAndStudies(studyMask);
    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    if (patientList.count() == 1)
    {
        m_studyTreeWidget->insertPatient(patientList.at(0));
        m_studyTreeWidget->sort();
    }
}

void QInputOutputLocalDatabaseWidget::removeStudyFromQStudyTreeWidget(QString studyInstanceUID)
{
    m_studyTreeWidget->removeStudy(studyInstanceUID);
}

void QInputOutputLocalDatabaseWidget::requestedSeriesOfStudy(Study *study)
{ 
    INFO_LOG("Search for series in the study's source cache " + study->getInstanceUID());

    // We prepare the mask and look for the series in the cache
    DicomMask mask;
    mask.setStudyInstanceUID(study->getInstanceUID());

    LocalDatabaseManager localDatabaseManager;
    QList<Series*> seriesList = localDatabaseManager.querySeries(mask);

    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    if (seriesList.isEmpty())
    {
        QMessageBox::information(this, ApplicationNameString, tr("No series match for this study.") + "\n");
        return;
    }
    else
    {
        // We insert the series information into the study
        m_studyTreeWidget->insertSeriesList(study->getInstanceUID(), seriesList);
    }
}

void QInputOutputLocalDatabaseWidget::setSeriesToSeriesListWidget(Study *currentStudy)
{
    m_seriesThumbnailPreviewWidget->clear();

    if (!currentStudy)
    {
        return;
    }

    INFO_LOG("Search series in the studio cache" + currentStudy->getInstanceUID());

    DicomMask mask;
    mask.setStudyInstanceUID(currentStudy->getInstanceUID());

    LocalDatabaseManager localDatabaseManager;
    QList<Series*> seriesList = localDatabaseManager.querySeries(mask);
    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    foreach (Series *series, seriesList)
    {
        m_seriesThumbnailPreviewWidget->insertSeries(currentStudy->getInstanceUID(), series);
    }

    qDeleteAll(seriesList);
}

void QInputOutputLocalDatabaseWidget::currentSeriesOfQStudyTreeWidgetChanged(Series *series)
{
    if (series)
    {
        m_seriesThumbnailPreviewWidget->setCurrentSeries(series->getInstanceUID());
    }
}

void QInputOutputLocalDatabaseWidget::currentSeriesChangedOfQSeriesListWidget(const QString &studyInstanceUID, const QString &seriesInstanceUID)
{
    m_studyTreeWidget->setCurrentSeries(studyInstanceUID, seriesInstanceUID);
}

void QInputOutputLocalDatabaseWidget::deleteSelectedItemsFromLocalDatabase()
{
    QList<QPair<DicomMask, DICOMSource> > selectedDicomMaskDICOMSoruceToDelete = m_studyTreeWidget->getDicomMaskOfSelectedItems();

    if (!selectedDicomMaskDICOMSoruceToDelete.isEmpty())
    {
        QMessageBox::StandardButton response = QMessageBox::question(this, ApplicationNameString,
                                                                     tr("Are you sure you want to delete the selected items?"),
                                                                     QMessageBox::Yes | QMessageBox::No,
                                                                     QMessageBox::No);
        if (response == QMessageBox::Yes)
        {
            QApplication::setOverrideCursor(Qt::BusyCursor);
            LocalDatabaseManager localDatabaseManager;

            for (int index = 0; index < selectedDicomMaskDICOMSoruceToDelete.count(); index++)
            {
                DicomMask dicomMaskToDelete = selectedDicomMaskDICOMSoruceToDelete.at(index).first;
                if (m_qcreateDicomdir->studyExistsInDICOMDIRList(dicomMaskToDelete.getStudyInstanceUID()))
                {
                    Study *studyToDelete = m_studyTreeWidget->getStudy(dicomMaskToDelete.getStudyInstanceUID());
                    QString warningMessage;

                    if (dicomMaskToDelete.getSeriesInstanceUID().isEmpty())
                    {
                        warningMessage = tr("Study %1 of patient %2 is in use by the DICOMDIR list. If you want to delete "
                                            "this study you must remove it from the DICOMDIR list first.")
                                .arg(studyToDelete->getID(), studyToDelete->getParentPatient()->getFullName());
                    }
                    else
                    {
                        // EVERYTHING: We should show the Series ID instead of the Series UID
                        warningMessage = tr("The series with UID %1 of study %2 of patient %3 is in use by the DICOMDIR list. "
                                            "If you want to delete "
                                            "this series you must remove the study from the DICOMDIR list first.")
                                .arg(dicomMaskToDelete.getSeriesInstanceUID(), studyToDelete->getID(),
                                     studyToDelete->getParentPatient()->getFullName());
                    }

                    QMessageBox::warning(this, ApplicationNameString, warningMessage);
                }
                else
                {
                    if (!dicomMaskToDelete.getSeriesInstanceUID().isEmpty())
                    {
                        INFO_LOG(QString("User has indicated that he wants to clear series %1 from study %2 from the cache")
                                 .arg(dicomMaskToDelete.getSeriesInstanceUID(), dicomMaskToDelete.getStudyInstanceUID()));
                        localDatabaseManager.deleteSeries(dicomMaskToDelete.getStudyInstanceUID(), dicomMaskToDelete.getSeriesInstanceUID());

                        m_seriesThumbnailPreviewWidget->removeSeries(dicomMaskToDelete.getSeriesInstanceUID());
                        m_studyTreeWidget->removeSeries(dicomMaskToDelete.getStudyInstanceUID(), dicomMaskToDelete.getSeriesInstanceUID());
                    }
                    else
                    {
                        INFO_LOG(QString("The user has indicated that they want to clear the study from the cache %1")
                                 .arg(dicomMaskToDelete.getStudyInstanceUID()));
                        localDatabaseManager.deleteStudy(dicomMaskToDelete.getStudyInstanceUID());

                        m_seriesThumbnailPreviewWidget->clear();
                        removeStudyFromQStudyTreeWidget(dicomMaskToDelete.getStudyInstanceUID());
                    }

                    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
                    {
                        break;
                    }
                }
            }
            QApplication::restoreOverrideCursor();
        }
    }
    else
    {
        QMessageBox::information(this, ApplicationNameString, tr("Please select at least one item to delete."));
    }
}

void QInputOutputLocalDatabaseWidget::view(QString studyInstanceUID, bool loadOnly)
{
    DicomMask studyToView;
    studyToView.setStudyInstanceUID(studyInstanceUID);

    QList<DicomMask> studiesToView;
    studiesToView.append(studyToView);

    view(studiesToView, loadOnly);
}

void QInputOutputLocalDatabaseWidget::view(QList<DicomMask> dicomMaskStudiesToView, bool loadOnly)
{
    DicomMask patientToProcessMask;
    Patient *patient;
    QList<Patient*> selectedPatientsList;

    if (dicomMaskStudiesToView.isEmpty())
    {
        QMessageBox::warning(this, ApplicationNameString, tr("Select at least one study to view."));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    foreach (DicomMask dicomMaskStudyToView, dicomMaskStudiesToView)
    {
        LocalDatabaseManager localDatabaseManager;

        patientToProcessMask.setStudyInstanceUID(dicomMaskStudyToView.getStudyInstanceUID());

        patient = localDatabaseManager.retrieve(patientToProcessMask);

        if (showDatabaseManagerError(localDatabaseManager.getLastError()))
        {
            QApplication::restoreOverrideCursor();
            return;
        }

        if (patient)
        {
            patient->setSelectedSeries(dicomMaskStudyToView.getSeriesInstanceUID());
            selectedPatientsList << patient;
        }
        else
        {
            DEBUG_LOG("Could not get study with UID " + dicomMaskStudyToView.getStudyInstanceUID());
        }
    }

    if (selectedPatientsList.count() > 0)
    {
        DEBUG_LOG("Released signal to visualize patient study " + patient->getFullName());
        emit viewPatients(selectedPatientsList, loadOnly);
    }

    QApplication::restoreOverrideCursor();
}

void QInputOutputLocalDatabaseWidget::viewFromQStudyTreeWidget()
{
    QList<DicomMask> dicomMaskStudiesToView;
    QList<QPair<DicomMask, DICOMSource> > selectedDICOMItemsInQStudyTreeWidget = m_studyTreeWidget->getDicomMaskOfSelectedItems();

    for (int index = 0; index < selectedDICOMItemsInQStudyTreeWidget.count(); index++)
    {
        dicomMaskStudiesToView.append(selectedDICOMItemsInQStudyTreeWidget.at(index).first);
    }

    view(dicomMaskStudiesToView);
}

void QInputOutputLocalDatabaseWidget::viewFromQSeriesListWidget(QString studyInstanceUID, QString seriesInstanceUID)
{
    DicomMask studyToView;
    studyToView.setStudyInstanceUID(studyInstanceUID);
    studyToView.setSeriesInstanceUID(seriesInstanceUID);

    view(QList<DicomMask>() << studyToView);

    StatsWatcher::log("We open studio by selecting series from thumbnail");
}

// EVERYTHING instead of making a signal to the queryscreen, so that it tells the QInputOutPacsWidget to save a study to the PACS,
// this function should not be in charge of saving the study directly to the PACS, I understand that it is not the responsibility of
// QInputOutputPacsWidget
void QInputOutputLocalDatabaseWidget::selectedStudiesStoreToPacs()
{
    if (m_studyTreeWidget->getDicomMaskOfSelectedItems().count() == 0)
    {
        QMessageBox::warning(this, ApplicationNameString, tr("Select at least one item to send to PACS."));
    }
    else
    {
        m_qwidgetSelectPacsToStoreDicomImage->show();
    }
}

void QInputOutputLocalDatabaseWidget::addSelectedStudiesToCreateDicomdirList()
{
    DicomMask studyMask;
    LocalDatabaseManager localDatabaseManager;
    QList<Patient*> patientList;
    QList<Study*> studies;
    QList<QPair<DicomMask, DICOMSource> > selectedDICOMItemsFromQStudyTreeWidget = m_studyTreeWidget->getDicomMaskOfSelectedItems();

    for (int index = 0; index < selectedDICOMItemsFromQStudyTreeWidget.count(); index++)
    {
        studyMask.setStudyInstanceUID(selectedDICOMItemsFromQStudyTreeWidget.at(index).first.getStudyInstanceUID());
        patientList = localDatabaseManager.queryPatientsAndStudies(studyMask);
        if (showDatabaseManagerError(localDatabaseManager.getLastError()))
        {
            return;
        }

        // \ TODO This must be done because queryPatientStudy returns Patient list
        // We actually want study list with Patient data filled.
        if (patientList.size() != 1 && patientList.first()->getNumberOfStudies() != 1)
        {
            showDatabaseManagerError(LocalDatabaseManager::DatabaseCorrupted);
            return;
        }
        studies << patientList.first()->getStudies().first();
    }
    m_qcreateDicomdir->addStudies(studies);
}

/// EVERYTHING: This responsibility to delete old studies when starting the application
/// should be moved to another location, not liability
/// of this inference
void QInputOutputLocalDatabaseWidget::deleteOldStudies()
{
    Settings settings;
    /// Let's see if the delete settings option is enabled
    /// old studies not displayed in a given number of days
    /// we do the check, to avoid starting the thread if the old studies do not have to be deleted
    if (settings.getValue(InputOutputSettings::DeleteLeastRecentlyUsedStudiesInDaysCriteria).toBool())
    {
        m_qdeleteOldStudiesThread.deleteOldStudies();
    }
}

QList<Image*> QInputOutputLocalDatabaseWidget::getAllImagesFromPatient(Patient *patient)
{
    QList<Image*> images;

    foreach (Study *study, patient->getStudies())
    {
        foreach (Series *series, study->getSeries())
        {
            images.append(series->getImages());
        }
    }

    return images;
}

void QInputOutputLocalDatabaseWidget::deleteOldStudiesThreadFinished()
{
    showDatabaseManagerError(m_qdeleteOldStudiesThread.getLastError(), tr("deleting old studies"));
}

void QInputOutputLocalDatabaseWidget::qSplitterPositionChanged()
{
    Settings().saveGeometry(InputOutputSettings::LocalDatabaseSplitterState, m_StudyTreeSeriesListQSplitter);
}

void QInputOutputLocalDatabaseWidget::sendSelectedStudiesToSelectedPacs()
{
    foreach (PacsDevice pacsDevice, m_qwidgetSelectPacsToStoreDicomImage->getSelectedPacsToStoreDicomImages())
    {
        QList<QPair<DicomMask, DICOMSource> > dicomObjectsToSendToPACS = m_studyTreeWidget->getDicomMaskOfSelectedItems();

        for (int index = 0; index < dicomObjectsToSendToPACS.count(); index++)
        {
            DicomMask dicomMaskToSend = dicomObjectsToSendToPACS.at(index).first;
            LocalDatabaseManager localDatabaseManager;
            Patient *patient = localDatabaseManager.retrieve(dicomMaskToSend);

            if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
            {
                ERROR_LOG(QString("Error in database trying to get studies to be sent to PACS, Error: %1; StudyUID: %2")
                          .arg(localDatabaseManager.getLastError())
                          .arg(dicomMaskToSend.getStudyInstanceUID()));

                QString message = tr("There has been a database error while preparing the "
                                     "DICOM files to send to PACS %1. The DICOM files won't be sent.")
                        .arg(pacsDevice.getAETitle());
                message += "\n";
                message += UserMessage::getCloseWindowsAndTryAgainAdvice();
                message += "\n\n";
                message += UserMessage::getProblemPersistsAdvice();
                QMessageBox::critical(this, ApplicationNameString, message);
            }
            else
            {
                sendDICOMFilesToPACS(pacsDevice, getAllImagesFromPatient(patient));
            }
        }
    }
}

void QInputOutputLocalDatabaseWidget::sendDICOMFilesToPACS(PacsDevice pacsDevice, QList<Image*> images)
{
    PACSJobPointer sendDICOMFilesToPACSJob(new SendDICOMFilesToPACSJob(pacsDevice, images));
    connect(sendDICOMFilesToPACSJob.data(), SIGNAL(PACSJobFinished(PACSJobPointer)), SLOT(sendDICOMFilesToPACSJobFinished(PACSJobPointer)));
    m_pacsManager->enqueuePACSJob(sendDICOMFilesToPACSJob);
}

void QInputOutputLocalDatabaseWidget::sendDICOMFilesToPACSJobFinished(PACSJobPointer pacsJob)
{
    QSharedPointer<SendDICOMFilesToPACSJob> sendDICOMFilesToPACSJob = pacsJob.objectCast<SendDICOMFilesToPACSJob>();

    if (sendDICOMFilesToPACSJob->getStatus() != PACSRequestStatus::SendOk)
    {
        if (sendDICOMFilesToPACSJob->getStatus() == PACSRequestStatus::SendWarningForSomeImages ||
                sendDICOMFilesToPACSJob->getStatus() == PACSRequestStatus::SendSomeDICOMFilesFailed)
        {
            QMessageBox::warning(this, ApplicationNameString, sendDICOMFilesToPACSJob->getStatusDescription());
        }
        else
        {
            QMessageBox::critical(this, ApplicationNameString, sendDICOMFilesToPACSJob->getStatusDescription());
        }
    }
}

void QInputOutputLocalDatabaseWidget::newPACSJobEnqueued(PACSJobPointer pacsJob)
{
    /// Connect to the RetrieveDICOMFilesFromPACSJob signal that will be deleted
    /// a cache study to remove them from the QStudyTreeWidget when
    /// delete them
    /// TODO: RetrieveDICOMFilesFromPACS should not emit this signal,
    /// should be a CacheManager this way we would take the responsibility
    /// of RetrieveDICOMFilesFromPACS to do so, and also would not need to connect
    /// the signal every time they do a new Job. Once the
    /// CacheManager this method MUST DISAPPEAR, when removed
    /// this method remember to remove the include in "retrievedicomfilesfrompacsjob.h"
    if (pacsJob->getPACSJobType() == PACSJob::RetrieveDICOMFilesFromPACSJobType)
    {
        connect(pacsJob.objectCast<RetrieveDICOMFilesFromPACSJob>().data(), SIGNAL(studyFromCacheWillBeDeleted(QString)),
                SLOT(removeStudyFromQStudyTreeWidget(QString)));
    }
}

bool QInputOutputLocalDatabaseWidget::showDatabaseManagerError(LocalDatabaseManager::LastError error, const QString &doingWhat)
{
    QString message;

    if (!doingWhat.isEmpty())
    {
        message = tr("An error has occurred while ") + doingWhat + ":\n\n";
    }

    switch (error)
    {
    case LocalDatabaseManager::Ok:
        return false;
    case LocalDatabaseManager::DatabaseLocked:
        message += tr("The database is blocked by another process.");
        message += "\n";
        message += UserMessage::getCloseWindowsAndTryAgainAdvice();
        message += "\n\n";
        message += UserMessage::getProblemPersistsAdvice();
        break;
    case LocalDatabaseManager::DatabaseCorrupted:
        message += tr("Database is corrupted.");
        message += "\n";
        message += UserMessage::getCloseWindowsAndTryAgainAdvice();
        message += "\n\n";
        message += UserMessage::getProblemPersistsAdvice();
        break;
    case LocalDatabaseManager::SyntaxErrorSQL:
        message += tr("Database syntax error.");
        message += "\n";
        message += UserMessage::getCloseWindowsAndTryAgainAdvice();
        message += "\n\n";
        message += UserMessage::getProblemPersistsAdvice();
        break;
    case LocalDatabaseManager::DatabaseError:
        message += tr("An internal database error occurred.");
        message += "\n";
        message += UserMessage::getCloseWindowsAndTryAgainAdvice();
        message += "\n\n";
        message += UserMessage::getProblemPersistsAdvice();
        break;
    case LocalDatabaseManager::DeletingFilesError:
        message += tr("Some files cannot be deleted.");
        message += "\n";
        message += tr("These files have to be deleted manually.");
        break;
    default:
        message = tr("Unknown error.");
        break;
    }

    QApplication::restoreOverrideCursor();

    QMessageBox::critical(this, ApplicationNameString, message);

    return true;
}

};
