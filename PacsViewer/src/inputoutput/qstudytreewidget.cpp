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

#include "qstudytreewidget.h"

#include <QContextMenuEvent>

#include "logging.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "dicommask.h"

namespace udg {

const QString QStudyTreeWidget::TypeTextForDummyExpandableItems("DUMMY_EXPANDABLE_ITEM");

QStudyTreeWidget::QStudyTreeWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    m_studyTreeView->setColumnHidden(Type, true);
    m_studyTreeView->setColumnHidden(DICOMItemID, true);
    //// Hide the Time column, as the date and time are now displayed
    /// in the same column so you can sort the studies by date and time
    m_studyTreeView->setColumnHidden(Time, true);
    /// Hack: Because the width of the columns for IndexColumn is saved in the settings,
    //if we add a new column, we have to add it at the end we can't add it to its corresponding place
    /// since otherwise the width of the columns saved in the settings would be applied incorrectly.
    /// Suppose we have the columns PatientName, StudyID StudyDescription, if we add a new column next to it
    /// of PatientName in this new column will be applied the width that had StudyID,
    /// to StudyID the one of StudyDescription and so on, for that reason what we do if we add a new column
    /// is added to the end of the QTreeWidget and then we move it to its natural place
    m_studyTreeView->header()->moveSection(PatientBirth, PatientAge);
    /// Reordering the columns. We are using the column identifier just to point out wich
    /// column we are moving. We have to add a number to the original index
    /// because of the prior column moves. The column identifier is not bound to the actual column index.
    /// TODO This should be the default column order in the .ui file.
    /// The problem with the saved columns width stated before should be solved in another way.
    m_studyTreeView->header()->moveSection(Date + 1, 2);
    m_studyTreeView->header()->moveSection(Description + 2, 3);
    m_studyTreeView->header()->moveSection(Modality + 2, 4);

    ///We upload the images displayed on the QStudyTreeWidget
    m_iconOpenStudy = QIcon(":/images/icons/dicom-study.svg");
    m_iconCloseStudy = QIcon(":/images/icons/dicom-study-closed.svg");
    m_iconOpenSeries = QIcon(":/images/icons/dicom-series.svg");
    m_iconCloseSeries = QIcon(":/images/icons/dicom-series-closed.svg");
    m_iconDicomFile = QIcon(":/images/icons/dicom-document.svg");

    createConnections();

    m_studyTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    /// We indicate that the maximum level at which the
    /// Study / Series / Image tree can be issued by default is up to Image level
    m_maximumExpandTreeItemsLevel = ImageLevel;

    initialize();

    m_useDICOMSourceToDiscriminateStudies = true;
}

void QStudyTreeWidget::setUseDICOMSourceToDiscriminateStudies(bool discrimateStudiesByDicomSource)
{
    m_useDICOMSourceToDiscriminateStudies = discrimateStudiesByDicomSource;
}

bool QStudyTreeWidget::getUseDICOMSourceToDiscriminateStudies()
{
    return m_useDICOMSourceToDiscriminateStudies;
}

void QStudyTreeWidget::createConnections()
{
    connect(m_studyTreeView, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(doubleClicked(QTreeWidgetItem*, int)));
    connect(m_studyTreeView, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            SLOT (currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)));
    connect(m_studyTreeView, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT (itemExpanded(QTreeWidgetItem*)));
    connect(m_studyTreeView, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT (itemCollapsed(QTreeWidgetItem*)));
}

void QStudyTreeWidget::insertPatientList(QList<Patient*> patientList)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    foreach (Patient *patient, patientList)
    {
        insertPatient(patient);
    }

    QApplication::restoreOverrideCursor();
}

void QStudyTreeWidget::insertPatient(Patient *patient)
{
    if (patient->getNumberOfStudies() > 0)
    {
        m_studyTreeView->addTopLevelItems(fillPatient(patient));
        m_studyTreeView->clearSelection();

        /// There are studies that can share the same patient object,
        /// for example in DICOMDIR where the same patient has more than one study
        m_addedPatients.append(patient);
    }
}

void QStudyTreeWidget::insertSeriesList(const QString &studyInstanceUID, QList<Series*> seriesList)
{
    QTreeWidgetItem *studyItem = getStudyQTreeWidgetItem(studyInstanceUID, seriesList.at(0)->getDICOMSource());
    if (!studyItem)
    {
        ERROR_LOG("The study of where to insert the series was not found.");
        return;
    }

    foreach (Series *series, seriesList)
    {
        studyItem->addChild(fillSeries(series));
        /// FIXME: The Series object inherits from QObject, when a setParentStudy is done to it,
        /// as a parent of the Series QObject it is assigned the study object
        /// this assignment fails if series and study have been created in different
        /// threads such as PACS search. Two QObjects to be father and son have
        /// of being of the same thread
        series->setParentStudy(getStudyByDICOMItemID(studyItem->text(DICOMItemID).toInt()));
    }
}

void QStudyTreeWidget::insertImageList(const QString &studyInstanceUID, const QString &seriesInstanceUID, QList<Image*> imageList)
{
    QTreeWidgetItem *seriesItem = getSeriesQTreeWidgetItem(studyInstanceUID, seriesInstanceUID, imageList.at(0)->getDICOMSource());

    if (!seriesItem)
    {
        ERROR_LOG("The series from which the images are to be inserted was not found.");
        return;
    }

    foreach (Image *image, imageList)
    {
        m_addedImagesByDICOMItemID[m_nextDICOMItemIDOfImage] = image;
        ///FIXME: L'objecte Image hereda de QObject, quan se li fa un setParentSeries,
        /// com a parent del QObject d'Image se li assigna l'objecte series
        /// this assignment fails if series and image have been created in different
        /// threads such as PACS search. Two QObjects to be father and son have
        /// of being of the same thread
        image->setParentSeries(getSeriesByDICOMItemID(seriesItem->text(DICOMItemID).toInt()));

        QTreeWidgetItem *newImageItem = new QTreeWidgetItem();

        newImageItem->setText(DICOMItemID, QString::number(m_nextDICOMItemIDOfImage++));
        newImageItem->setIcon(ObjectName, m_iconDicomFile);
        /// We paddle it so we can sort the column, as it is sorted by String
        newImageItem->setText(ObjectName, tr("File %1").arg(image->getInstanceNumber().rightJustified(4, ' ')));
        newImageItem->setText(UID, image->getSOPInstanceUID());
        newImageItem->setText(Type, "IMAGE");

        seriesItem->addChild(newImageItem);
    }
}

void QStudyTreeWidget::removeStudy(const QString &studyInstanceUIDToRemove, const DICOMSource &dicomSourceStudyToRemove)
{
    QTreeWidgetItem *studyItem = getStudyQTreeWidgetItem(studyInstanceUIDToRemove, dicomSourceStudyToRemove);

    if (studyItem)
    {
        delete studyItem;
    }

    m_studyTreeView->clearSelection();
    // We do not delete the HashTable studio will already be deleted when we clean the HashTable
}

void QStudyTreeWidget::removeSeries(const QString &studyInstanceUID, const QString &seriesInstanceUID, const DICOMSource &dicomSourceSeriesToRemove)
{
    QTreeWidgetItem *seriesItem = getSeriesQTreeWidgetItem(studyInstanceUID, seriesInstanceUID, dicomSourceSeriesToRemove);

    if (seriesItem)
    {
        if (seriesItem->parent()->childCount() == 1)
        {
            // If the studio only has this series we delete the entire studio
            Study *studyToRemove = getStudyByDICOMItemID(seriesItem->parent()->text(DICOMItemID).toInt());
            removeStudy(studyToRemove->getInstanceUID(), studyToRemove->getDICOMSource());
        }
        else
        {
            delete seriesItem;
        }
    }

    m_studyTreeView->clearSelection();
}

QList<QPair<DicomMask, DICOMSource> > QStudyTreeWidget::getDicomMaskOfSelectedItems()
{
    QList<QPair<DicomMask, DICOMSource> > dicomMaskDICOMSourceList;

    foreach (QTreeWidgetItem *item, m_studyTreeView->selectedItems())
    {
        QPair<DicomMask, DICOMSource> qpairDicomMaskDICOMSource;
        bool ok;

        if (isItemStudy(item))
        {
            // It's a study
            Study *selectedStudy = getStudyByDICOMItemID(item->text(DICOMItemID).toInt());

            qpairDicomMaskDICOMSource.first = DicomMask::fromStudy(selectedStudy, ok);
            qpairDicomMaskDICOMSource.second = selectedStudy->getDICOMSource();

            dicomMaskDICOMSourceList.append(qpairDicomMaskDICOMSource);
        }
        else if (isItemSeries(item))
        {
            // If the parent studio is not selected
            if (!item->parent()->isSelected())
            {
                Series *selectedSeries = getSeriesByDICOMItemID(item->text(DICOMItemID).toInt());

                qpairDicomMaskDICOMSource.first = DicomMask::fromSeries(selectedSeries, ok);
                qpairDicomMaskDICOMSource.second = selectedSeries->getDICOMSource();

                dicomMaskDICOMSourceList.append(qpairDicomMaskDICOMSource);
            }
        }
        else if (isItemImage(item))
        {
            // If the parent series and the parent studio are not selected
            if (!item->parent()->isSelected() && !item->parent()->parent()->isSelected())
            {
                Image *selectedImage = getImageByDICOMItemID(item->text(DICOMItemID).toInt());

                qpairDicomMaskDICOMSource.first = DicomMask::fromImage(selectedImage, ok);
                qpairDicomMaskDICOMSource.second = selectedImage->getDICOMSource();

                dicomMaskDICOMSourceList.append(qpairDicomMaskDICOMSource);
            }
        }
    }

    return dicomMaskDICOMSourceList;
}

void QStudyTreeWidget::setSortByColumn(QStudyTreeWidget::ColumnIndex col, Qt::SortOrder sortOrder)
{
    m_studyTreeView->sortItems(col, sortOrder);
    m_studyTreeView->clearSelection();
}

QStudyTreeWidget::ColumnIndex QStudyTreeWidget::getSortColumn()
{
    return (QStudyTreeWidget::ColumnIndex) m_studyTreeView->header()->sortIndicatorSection();;
}

Qt::SortOrder QStudyTreeWidget::getSortOrderColumn()
{
    return (Qt::SortOrder) m_studyTreeView->header()->sortIndicatorOrder();
}

void QStudyTreeWidget::sort()
{
    m_studyTreeView->sortItems(m_studyTreeView->sortColumn(), m_studyTreeView->header()->sortIndicatorOrder());
}

Study* QStudyTreeWidget::getStudy(const QString &studyInstanceUID, const DICOMSource &dicomSourceOfStudy)
{
    QTreeWidgetItem *studyItem = getStudyQTreeWidgetItem(studyInstanceUID, dicomSourceOfStudy);
    Study *study = NULL;

    if (studyItem)
    {
        study = getStudyByDICOMItemID(studyItem->text(DICOMItemID).toInt());
    }

    return study;
}

Series* QStudyTreeWidget::getSeries(const QString &studyInstanceUID, const QString &seriesInstanceUID,
                                    const DICOMSource &dicomSourceOfSeries)
{
    QTreeWidgetItem *seriesItem = getSeriesQTreeWidgetItem(studyInstanceUID, seriesInstanceUID, dicomSourceOfSeries);
    Series *series = NULL;

    if (seriesItem)
    {
        series = getSeriesByDICOMItemID(seriesItem->text(DICOMItemID).toInt());
    }

    return series;
}

void QStudyTreeWidget::setContextMenu(QMenu *contextMenu)
{
    m_contextMenu = contextMenu;
}

void QStudyTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_studyTreeView->selectedItems().isEmpty())
    {
        m_contextMenu->exec(event->globalPos());
    }
}

QTreeWidget* QStudyTreeWidget::getQTreeWidget() const
{
    return m_studyTreeView;
}

void QStudyTreeWidget::setMaximumExpandTreeItemsLevel(QStudyTreeWidget::ItemTreeLevels maximumExpandTreeItemsLevel)
{
    m_maximumExpandTreeItemsLevel = maximumExpandTreeItemsLevel;
}

QStudyTreeWidget::ItemTreeLevels QStudyTreeWidget::getMaximumExpandTreeItemsLevel()
{
    return m_maximumExpandTreeItemsLevel;
}

void QStudyTreeWidget::setCurrentSeries(const QString &studyInstanceUID, const QString &seriesInstanceUID,
                                        const DICOMSource &dicomSource)
{
    QTreeWidgetItem *seriesItem = getSeriesQTreeWidgetItem(studyInstanceUID, seriesInstanceUID, dicomSource);

    if (!seriesItem)
    {
        return;
    }

    if (seriesItem->parent()->isExpanded())
    {
        /// We check that the parent element is deployed because otherwise
        /// Qt pops up if we assign as a current element an invisible element
        m_studyTreeView->setCurrentItem (seriesItem);
    }
}

void QStudyTreeWidget::clear()
{
    m_studyTreeView->clear();

    qDeleteAll(m_addedImagesByDICOMItemID);
    qDeleteAll(m_adddSeriesByDICOMItemID);
    qDeleteAll(m_addedStudiesByDICOMItemID);
    qDeleteAll(m_addedPatients);

    m_addedPatients.clear();
    m_addedStudiesByDICOMItemID.clear();
    m_adddSeriesByDICOMItemID.clear();
    m_addedImagesByDICOMItemID.clear();

    initialize();
}

void QStudyTreeWidget::initialize()
{
    m_nextIDICOMItemIDOfStudy = 0;
    m_nextDICOMItemIDOfSeries = 0;
    m_nextDICOMItemIDOfImage = 0;

    m_qTreeWidgetItemHasBeenDoubleClicked = false;
    m_oldCurrentStudy = NULL;
    m_oldCurrentSeries = NULL;
}

QTreeWidgetItem* QStudyTreeWidget::getStudyQTreeWidgetItem(const QString &studyUID, const DICOMSource &studyDICOMSource)
{
    QList<QTreeWidgetItem*> qTreeWidgetItemsStudy(m_studyTreeView->findItems(studyUID, Qt::MatchExactly, UID));

    foreach(QTreeWidgetItem *studyItem, qTreeWidgetItemsStudy)
    {
        if (isItemStudy(studyItem) && (getStudyByDICOMItemID(
                                           studyItem->text(DICOMItemID).toInt())->getDICOMSource() == studyDICOMSource
                                       || !m_useDICOMSourceToDiscriminateStudies))
        {
            return studyItem;
        }
    }

    return NULL;
}

QTreeWidgetItem* QStudyTreeWidget::getSeriesQTreeWidgetItem(const QString &studyInstanceUID,
                                                            const QString &seriesInstanceUID, const DICOMSource &seriesDICOMSource)
{
    QTreeWidgetItem *studyItem = getStudyQTreeWidgetItem(studyInstanceUID, seriesDICOMSource);

    if (!studyItem)
    {
        return NULL;
    }

    for (int index = 0; index < studyItem->childCount(); index++)
    {
        if (studyItem->child(index)->text(UID) == seriesInstanceUID)
        {
            return studyItem->child(index);
        }
    }

    return NULL;
}

bool QStudyTreeWidget::isItemStudy(QTreeWidgetItem *item)
{
    return item->text(Type) == "STUDY";
}

bool QStudyTreeWidget::isItemSeries(QTreeWidgetItem *item)
{
    return item->text(Type) == "SERIES";
}

bool QStudyTreeWidget::isItemImage(QTreeWidgetItem *item)
{
    return item->text(Type) == "IMAGE";
}

QList<QTreeWidgetItem*> QStudyTreeWidget::fillPatient(Patient *patient)
{
    QList<QTreeWidgetItem*> qtreeWidgetItemList;

    foreach (Study *studyToInsert, patient->getStudies())
    {
        //// If the study already exists in StudyTreeView we delete it
        removeStudy(patient->getStudies().at(0)->getInstanceUID(), patient->getStudies().at(0)->getDICOMSource());

        // We insert the study in the list of studies
        m_addedStudiesByDICOMItemID[m_nextIDICOMItemIDOfStudy] = studyToInsert;

        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(DICOMItemID, QString::number(m_nextIDICOMItemIDOfStudy++));
        item->setIcon(ObjectName, m_iconCloseStudy);
        item->setText(ObjectName, patient->getFullName());
        item->setText(PatientID, patient->getID());
        item->setText(PatientBirth, formatDateTime(patient->getBirthDate(), QTime()));
        item->setText(PatientAge, formatAge(studyToInsert->getPatientAge()));
        item->setText(Modality, studyToInsert->getModalitiesAsSingleString());
        item->setText(Description, studyToInsert->getDescription());
        /// todo: Shouldn't the studio return the formatted date? Problem
        ///  we need the date to be in yyyy / mm / dd format to be able to sort by date
        item->setText(Date, formatDateTime(studyToInsert->getDate(), studyToInsert->getTime()));
        item->setText(StudyID, tr("Study %1").arg(studyToInsert->getID()));
        item->setText(Institution, studyToInsert->getInstitutionName());
        item->setText(AccNumber, studyToInsert->getAccessionNumber());
        item->setText(UID, studyToInsert->getInstanceUID());
        // We indicate that this is a study
        item->setText(Type, "STUDY");
        item->setText(RefPhysName, studyToInsert->getReferringPhysiciansName());

        // Check if the TreeItem should expand based on the maximum level they have told us we can expand
        if (m_maximumExpandTreeItemsLevel > StudyLevel)
        {
            /// Because for each study item we have child items that are series,
            /// and that consulting the series for each study is
            /// an expensive operation (for example when consulting the pacs)
            /// we will only insert the series so that you can
            /// consult the user when making a study expansion, but for the "+"
            /// button to display the study to appear we insert a blank item
            item->addChild(createDummyQTreeWidgetItem());
        }
        qtreeWidgetItemList.append(item);
    }

    return qtreeWidgetItemList;
}

QTreeWidgetItem* QStudyTreeWidget::fillSeries(Series *series)
{
    m_adddSeriesByDICOMItemID[m_nextDICOMItemIDOfSeries]= series;

    QTreeWidgetItem *seriesItem = new QTreeWidgetItem();

    seriesItem->setText(DICOMItemID, QString::number(m_nextDICOMItemIDOfSeries++));
    seriesItem->setIcon(ObjectName, m_iconCloseSeries);
    // We paddle it so we can sort the column, as it is sorted by String
    seriesItem->setText(ObjectName, tr("Series %1").arg(series->getSeriesNumber().rightJustified(4, ' ')));
    seriesItem->setText(Modality, series->getModality());

    // We remove the blank spaces from the front and back
    seriesItem->setText(Description, series->getDescription().simplified());

    seriesItem->setText(Date, formatDateTime(series->getDate(), series->getTime()));
    seriesItem->setText(UID, series->getInstanceUID());
    // Indiquem que es tracta d'una sèrie
    seriesItem->setText(Type, "SERIES");

    seriesItem->setText(ProtocolName, series->getProtocolName());
    seriesItem->setText(PPStartDate, series->getPerformedProcedureStepStartDate());
    seriesItem->setText(PPStartTime, series->getPerformedProcedureStepStartTime());
    seriesItem->setText(ReqProcID, series->getRequestedProcedureID());
    seriesItem->setText(SchedProcStep, series->getScheduledProcedureStepID());

    //// Check if the TreeItem should expand based on the maximum level they have told us we can expand
    if (m_maximumExpandTreeItemsLevel > SeriesLevel)
    {
        /// Because for each item series we have child items that are images, and that consulting the images for each series is
        /// an expensive operation (for example when consulting the pacs) we will only insert the series so that you can
        /// consult the user when expanding the series, but for the "+" button to unfold the series to appear we insert a blank item
        seriesItem->addChild(createDummyQTreeWidgetItem());
    }
    return seriesItem;
}

Study* QStudyTreeWidget::getStudyByDICOMItemID(int studyDICOMItemID)
{
    Study *study = NULL;

    if (m_addedStudiesByDICOMItemID.contains(studyDICOMItemID))
    {
        study = m_addedStudiesByDICOMItemID[studyDICOMItemID];
    }

    return study;
}

Series* QStudyTreeWidget::getSeriesByDICOMItemID(int seriesDICOMItemID)
{
    Series *series = NULL;

    if (m_adddSeriesByDICOMItemID.contains(seriesDICOMItemID))
    {
        series = m_adddSeriesByDICOMItemID[seriesDICOMItemID];
    }

    return series;
}

Image* QStudyTreeWidget::getImageByDICOMItemID(int imageDICOMItemID)
{
    Image *image = NULL;

    if (m_addedImagesByDICOMItemID.contains(imageDICOMItemID))
    {
        image = m_addedImagesByDICOMItemID[imageDICOMItemID];
    }

    return image;
}

Study* QStudyTreeWidget::getStudyByQTreeWidgetItem(QTreeWidgetItem *qTreeWidgetItem)
{
    Study *study = NULL;

    if (qTreeWidgetItem)
    {
        if (isItemStudy(qTreeWidgetItem))
        {
            study = getStudyByDICOMItemID(qTreeWidgetItem->text(DICOMItemID).toInt());
        }
        else if (isItemSeries(qTreeWidgetItem))
        {
            study = getStudyByDICOMItemID(qTreeWidgetItem->parent()->text(DICOMItemID).toInt());
        }
        else if (isItemImage(qTreeWidgetItem))
        {
            study = getStudyByDICOMItemID(qTreeWidgetItem->parent()->parent()->text(DICOMItemID).toInt());
        }
    }

    return study;
}

Series* QStudyTreeWidget::getSeriesByQTreeWidgetItem(QTreeWidgetItem *qTreeWidgetItem)
{
    Series *series = NULL;

    if (qTreeWidgetItem)
    {
        if (isItemSeries(qTreeWidgetItem))
        {
            series = getSeriesByDICOMItemID(qTreeWidgetItem->text(DICOMItemID).toInt());
        }
        else if (isItemImage(qTreeWidgetItem))
        {
            series = getSeriesByDICOMItemID(qTreeWidgetItem->parent()->text(DICOMItemID).toInt());
        }
    }

    return series;
}

Image* QStudyTreeWidget::getImageByQTreeWidgetItem(QTreeWidgetItem *qTreeWidgetItem)
{
    Image *image = NULL;

    if (qTreeWidgetItem)
    {
        if (isItemImage(qTreeWidgetItem))
        {
            image = getImageByDICOMItemID(qTreeWidgetItem->text(DICOMItemID).toInt());
        }
    }

    return image;
}

QString QStudyTreeWidget::formatAge(const QString &age) const
{
    QString text(age);

    if (text.length() > 0)
    {
        /// We subtract the 0 from before the years, the PACS sends for example: 047Y we return 47Y
        if (text.at(0) == '0')
        {
            text.replace(0, 1, " ");
        }
    }

    return text;
}

QString QStudyTreeWidget::formatDateTime(const QDate &date, const QTime &time) const
{
    QString formatedDateTimeAsQString = "";

    if (!date.isNull() && !time.isNull())
    {
        formatedDateTimeAsQString = date.toString(Qt::ISODate) + "   " + time.toString(Qt::ISODate);
    }
    else if (!date.isNull())
    {
        formatedDateTimeAsQString = date.toString(Qt::ISODate);
    }

    return formatedDateTimeAsQString;
}

void QStudyTreeWidget::currentItemChanged(QTreeWidgetItem *currentItem, QTreeWidgetItem *)
{
    if (currentItem)
    {
        Study *currentStudy = getStudyByQTreeWidgetItem(currentItem);
        Series *currentSeries = getSeriesByQTreeWidgetItem(currentItem);

        if (currentStudy != m_oldCurrentStudy)
        {
            m_oldCurrentStudy = currentStudy;
            emit(currentStudyChanged(currentStudy));
        }

        if (currentSeries != m_oldCurrentSeries)
        {
            m_oldCurrentSeries = currentSeries;
            emit(currentSeriesChanged(currentSeries));
        }
    }
    else
    {
        emit notCurrentItemSelected();
    }
}

QTreeWidgetItem* QStudyTreeWidget::createDummyQTreeWidgetItem()
{
    QTreeWidgetItem *dummyItem = new QTreeWidgetItem();
    dummyItem->setText(Type, TypeTextForDummyExpandableItems);

    return dummyItem;
}

bool QStudyTreeWidget::isDummyQTreeWidgetItem(QTreeWidgetItem *dummyQTreeWidgetItem)
{
    return dummyQTreeWidgetItem->text(Type) == TypeTextForDummyExpandableItems;
}

void QStudyTreeWidget::itemExpanded(QTreeWidgetItem *itemExpanded)
{
    /// In case the item arrives with the empty text, we do nothing
    /// This happens in very specific situations when the '*' key is used to expand the item
    if (isDummyQTreeWidgetItem(itemExpanded))
    {
        return;
    }

    /// The QTreeWidget after double clicking expands or collapses the item depending on its status, we are not interested
    /// to do this, for this reason in case of a collapse or expand signal, what we do is check if that item ends
    /// to double click, if so we cancel the action of expanding
    if (!m_qTreeWidgetItemHasBeenDoubleClicked)
    {
        /// Because we insert a blank item to simulate children from studies
        /// and series every time they expand we have to remove the blank item and
        /// we emit a signal so that whoever collects it is in charge of taking the corresponding
        /// steps to expand the study or image with their relevant children
        qDeleteAll(itemExpanded->takeChildren());

        if (isItemStudy(itemExpanded))
        {
            itemExpanded->setIcon(ObjectName, m_iconOpenStudy);
            emit (requestedSeriesOfStudy(getStudyByDICOMItemID(itemExpanded->text(DICOMItemID).toInt())));
        }
        else if (isItemSeries(itemExpanded))
        {
            itemExpanded->setIcon(ObjectName, m_iconOpenSeries);
            emit (requestedImagesOfSeries(getSeriesByDICOMItemID(itemExpanded->text(DICOMItemID).toInt())));
        }
    }
    else
    {
        m_qTreeWidgetItemHasBeenDoubleClicked = false;
        m_studyTreeView->collapseItem(itemExpanded);
    }
}

void QStudyTreeWidget::itemCollapsed(QTreeWidgetItem *itemCollapsed)
{
    /// The QTreeWidget after double clicking expands or collapses the item
    /// depending on its status, we are not interested
    /// to do this, for this reason in case of a collapse or expand signal, what we do is check if that item ends
    /// to double click, if so we cancel the action of collapsing

    if (!m_qTreeWidgetItemHasBeenDoubleClicked)
    {
        if (isItemStudy(itemCollapsed))
        {
            itemCollapsed->setIcon(ObjectName, m_iconCloseStudy);
        }
        else if (isItemSeries(itemCollapsed))
        {
            itemCollapsed->setIcon(ObjectName, m_iconCloseSeries);
        }
    }
    else
    {
        m_qTreeWidgetItemHasBeenDoubleClicked = false;
        m_studyTreeView->expandItem (itemCollapsed);
    }

}

void QStudyTreeWidget::doubleClicked(QTreeWidgetItem *item, int)
{
    // Double-clicking on the QTreeWidget automatically expands or hides the object
    if (item == NULL)
    {
        return;
    }

    if (isItemStudy(item))
    {
        emit(studyDoubleClicked());
    }
    else if (isItemSeries(item))
    {
        emit(seriesDoubleClicked());
    }
    else if (isItemImage(item))
    {
        emit(imageDoubleClicked());
    }

    /// Due to the behavior of the tree widget when a double click is made,
    /// the item collapses or expands depending on its status, as
    /// we do not want the double click to expand or collapse, we save for which element
    /// the double click has been made, to cancel it when it is detected
    /// an expand signal or collapse item
    m_qTreeWidgetItemHasBeenDoubleClicked = true;
}

void QStudyTreeWidget::keyPressEvent(QKeyEvent*)
{
    /// Left empty to eat all key press events, especially for the Enter key.
    /// The Enter key press is handled with a connection to the itemActivated() signal but it's not consumed,
    /// and we don't want the event to be propagated to the parent widget.
}

}
