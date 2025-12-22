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

#ifndef UDGQSTUDYTREEWIDGET_H
#define UDGQSTUDYTREEWIDGET_H

#include "ui_qstudytreewidgetbase.h"

#include <QMenu>
#include <QList>

#include "dicomsource.h"

// Forward declarations
class QString;

namespace udg {

// Forward declarations
class Study;
class Patient;
class Series;
class Image;
class DicomMask;

/**
This class will show studies and series in an organized and easy way.
This class is a modification of the QTreeWidget that has been adapted to display
 study / series / image search information.
The class maintains the list of Studies / Series / Image that are inserted,
and are removed by invoking the clean method of the class, so remember
that classes that invoke methods that return pointers to Study / Series / Image
will be responsible for making a copy if they need to
keep the object alive once cleaned.

This class has two behaviors based on setUseDICOMSourceToDiscriminateStudies,
which allows us to establish whether studies should be discriminated apart from with the UID
by DICOMSource as well. Depending on what is established, studies with
the same UID but different DICOMSource will be considered as the same study (duplicates) or will be considered
as different studies.
*/
class QStudyTreeWidget : public QWidget, private Ui::QStudyTreeWidgetBase {
    Q_OBJECT
public:
    enum ItemTreeLevels { StudyLevel = 0, SeriesLevel = 1, ImageLevel = 2 };

    ///Object Name is used to save the Patient Name,
    /// Series + Serial and Image Identifier + Image Identifier
    enum ColumnIndex { ObjectName = 0, PatientID = 1, PatientAge = 2, Description = 3, Modality = 4, Date = 5, Time = 6,
                       DICOMItemID = 7, Institution = 8, UID = 9, StudyID = 10, ProtocolName = 11, AccNumber = 12, Type = 13,
                       RefPhysName = 14, PPStartDate = 15, PPStartTime = 16, ReqProcID = 17, SchedProcStep = 18, PatientBirth = 19 };

    QStudyTreeWidget(QWidget *parent = 0);

    /// Indicates whether to use DICOMSource apart from InstanceUID to
    /// discriminate studies and consider them duplicate. By default, DICOMSource is used for
    /// discriminate studies
    void setUseDICOMSourceToDiscriminateStudies(bool discrimateStudiesByDicomSource);
    bool getUseDICOMSourceToDiscriminateStudies();

    /// Show past studies by parameters. If any of the studies already exist,
    /// overwrite the information. Depending on the value set by setUseDICOMSourceToDiscriminateStudies
    /// studies with the same UID but different DICOMSource can be considered duplicates
    void insertPatientList(QList<Patient*> patientList);

    ///Insert patient into QStudyTreeWiget. If the patient with that study
    /// already exists, overwrite the information
    void insertPatient(Patient *patient);

    ///Inserts a list of series into the currently selected studio.
    void insertSeriesList(const QString &studyIstanceUID, QList<Series*> seriesList);

    /// Inserts a list of images into the currently selected series
    void insertImageList(const QString &studyInstanceUID, const QString &seriesInstanceUID, QList<Image*> imageList);

    /// Removes study from the list
    void removeStudy(const QString &studyInstanceUIDToRemove, const DICOMSource &dicomSourceStudyToRemove = DICOMSource());

    /// Delete QStudyTreeWidget series, if it is the only studio series also delete studio, it doesn't make sense to have a studio without
    /// series to the TreeWidget
    void removeSeries(const QString &studyInstanceUIDToRemove, const QString &seriesInstanceUIDToRemove, const DICOMSource &dicomSourceSeriesToRemove = DICOMSource());

    /// For each selected item Study / Series / Image returns its DicomMask.
    /// If we have the case we have selected a study, and from that study we have
    /// selected two series or one of the images will only return to us the study,
    /// since the action that must be done on the selected elements
    /// if the studio is selecting it does not make sense to do it on the series and the images,
    /// the same happens if we have selected a series and images of it.
    /// For example if we have selected a study and two of its series and the user
    ///  indicates that he wants to download it does not make sense to
    /// download the study and then the two series
    /// I think QStudyTreeWidget should not be aware that to do any
    ///  action on a DICOM source like DICOMDIR, BD, PACS ... you need
    /// a DICOMMask with DICOMSource, but right now we need this method
    /// as it returns it returns us agglutinated to all elements selected independently
    /// that they are Study, Series, Image in a common object the DICOMMask,
    ///  but the classes that use this QStudyTreeWidget should check which are the
    /// selected studies, selected series and selected images invoking a method for each object type.
    QList<QPair<DicomMask, DICOMSource> > getDicomMaskOfSelectedItems();

    ///Indicates / Returns the column and address by which list is sorted
    void setSortByColumn(QStudyTreeWidget::ColumnIndex sortColumn, Qt::SortOrder sortOrder);
    QStudyTreeWidget::ColumnIndex getSortColumn();
    Qt::SortOrder getSortOrderColumn();

    /// Sort in descending order by the selected column
    void sort();

    /// Returns the study / series that has the UID and DICOMSource passed by parameter.
    /// The returned object will be destroyed when the method is invoked
    /// clean of this class, so whether this object can be used after invoking
    ///  the clean method is the responsibility of the class calling it
    /// make a copy
    Study* getStudy(const QString &studyInstanceUID, const DICOMSource &dicomSourceOfStudy = DICOMSource());
    Series* getSeries(const QString &studyInstanceUID, const QString &seriesInstanceUID, const DICOMSource &dicomSourceOfSeries = DICOMSource());

    ///Sets the Widget context menu
    void setContextMenu(QMenu *contextMenu);

    ///Returns the QTreeWidget that the widget contains
    QTreeWidget* getQTreeWidget() const;

    /// Assigns / Gets the maximum level to which items displayed in
    /// QStudyTreeWiget can be expanded, by default expands to Image level
    void setMaximumExpandTreeItemsLevel(QStudyTreeWidget::ItemTreeLevels maximumExpandTreeItemsLevel);
    QStudyTreeWidget::ItemTreeLevels getMaximumExpandTreeItemsLevel();

public slots:
    /// Indicate that we mark the series with the uid passed by parameter as selected
    void setCurrentSeries(const QString &studyInstanceUID, const QString &seriesInstanceUID, const DICOMSource &dicomSource = DICOMSource());

    ///Clear the TreeView, and delete the inserted Study * / Series * / Images *
    void clear();

signals:
    /// Signal each time we select a different study. The past Study as
    /// a parameter is destroyed by invoking the clean method, of this class
    /// it is the responsibility of the class receiving the signal to make
    ///  a copy to avoid problems if you have to use the Study later after a clean has been done
    void currentStudyChanged(Study *currentStudy);

    /// Signal that is emitted when we change selected series. The Series
    /// passed as a parameter is destroyed by invoking the clean method, of this class
    /// it is the responsibility of the class receiving the signal to make
    ///  a copy to avoid problems if you have to use the Study later after a clean has been done
    void currentSeriesChanged(Series *currentSeries);

    //TODO: DoubleClicked signals should not return studio / series / clicked image =
    /// Signal that is emitted when a double click has been made to a study
    void studyDoubleClicked();

    ///Signal that is emitted when a series has been double-clicked
    void seriesDoubleClicked();

    /// Signal that is emitted when an image has been double-clicked
    void imageDoubleClicked();

    /// Signal that is emitted when you go from having a selected item to having none selected
    void notCurrentItemSelected();

    ///Indicates when a user has requested to view the series / images of a studio / series
    void requestedSeriesOfStudy(Study *studyRequestedSeries);
    void requestedImagesOfSeries(Series *seriesRequestedImage);

protected:
    /// Displays the context menu
    void contextMenuEvent(QContextMenuEvent *event);

    void keyPressEvent(QKeyEvent*) override;

private:
    /// Creates signal and slot connections
    void createConnections();

    /// Initialize the required QWidget variables
    void initialize();

    ///Returns the QTreeWidgetItem object shown in the study that meets the past parameters
    QTreeWidgetItem* getStudyQTreeWidgetItem(const QString &studyUID, const DICOMSource &studyDICOMSource);

    ///Returns the QTtreeWidgeItem Object that is from the studio and series
    QTreeWidgetItem* getSeriesQTreeWidgetItem(const QString &studyUID, const QString &seriesUID, const DICOMSource &seriesDICOMSource);

    /// Tells us if the last item is a Study / Series / Image
    bool isItemStudy(QTreeWidgetItem *);
    bool isItemSeries(QTreeWidgetItem *);
    bool isItemImage(QTreeWidgetItem *);

    ///Returns QTreeWidgetItem list resulting from studies the patient has
    QList<QTreeWidgetItem*> fillPatient(Patient *);

    /// Given a series fill in a QTreeWidgetItem to display the information of the series
    QTreeWidgetItem* fillSeries(Series *serie);

    /// Returns Study / Series / Image from its DICOMItemID
    ///  if it does not find it returns null
    Study* getStudyByDICOMItemID(int studyDICOMItemID);
    Series* getSeriesByDICOMItemID(int seriesDICOMItemID);
    Image* getImageByDICOMItemID(int imageDICOMItemID);

    /// Returns Study / Series / Image to which a QTreeWidgeITem belongs.
    /// If we pass a QTreeWidgetItem to getStudyByQTreeWidgetItem which
    /// is an image it will return the study to us at
    /// to which the image belongs, if we getSeriesByQTreeWidgetItem we
    /// pass an item that is a Study will return us null
    Study* getStudyByQTreeWidgetItem(QTreeWidgetItem *qTreeWidgetItem);
    Series* getSeriesByQTreeWidgetItem(QTreeWidgetItem *qTreeWidgetItem);
    Image* getImageByQTreeWidgetItem(QTreeWidgetItem *qTreeWidgetItem);

    ///Format age to display on screen
    QString formatAge(const QString &age) const;

    /// Format the date and time passed to ISO 8601 extended
    ///  (YYYY-MM-DD HH: MM: SS) With this date format you can sort the studies by date / time
    /// If the time has no value it only returns the date,
    /// and if neither Date nor Time has value it returns empty string
    QString formatDateTime(const QDate &date, const QTime &time) const;

    /// Create an empty QTreeWidgetItem, so that the + icon
    ///  appears so you can deploy studio / series with the mouse
    QTreeWidgetItem* createDummyQTreeWidgetItem();

    /// Whether QTreeWidgetItem is a Dummy used to expand
    /// Studies / Series when we have not yet queried its Series / Images child elements
    bool isDummyQTreeWidgetItem(QTreeWidgetItem *);

private slots:
    /// It emits signal when a study or series different from the previous one is selected
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    ///It emits signal when an item expands, and has no child items
    void itemExpanded(QTreeWidgetItem *itemExpanded);

    ///It emits signal when an item collapses, and has no child items
    void itemCollapsed(QTreeWidgetItem *itemCollapsed);

    ///It emits a signal when a double click is made on an item
    void doubleClicked(QTreeWidgetItem *, int);

private:
    static const QString TypeTextForDummyExpandableItems;

    int m_nextIDICOMItemIDOfStudy;
    int m_nextDICOMItemIDOfSeries;
    int m_nextDICOMItemIDOfImage;

    QList<Patient*> m_addedPatients;
    QHash<int, Study*> m_addedStudiesByDICOMItemID;
    QHash<int, Series*> m_adddSeriesByDICOMItemID;
    QHash<int, Image*> m_addedImagesByDICOMItemID;

    /// Menu contextual
    QMenu *m_contextMenu;

    /// Strings to save values of the previous element
    Study *m_oldCurrentStudy;
    Series *m_oldCurrentSeries;

    bool m_qTreeWidgetItemHasBeenDoubleClicked;
    bool m_useDICOMSourceToDiscriminateStudies;

    /// Icons used as root in the TreeWidget
    QIcon m_iconOpenStudy, m_iconCloseStudy, m_iconOpenSeries, m_iconCloseSeries, m_iconDicomFile;

    QStudyTreeWidget::ItemTreeLevels m_maximumExpandTreeItemsLevel;
};

} // end namespace

#endif
