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

#ifndef UDGQTHUMBNAILSPREVIEWWIDGET
#define UDGQTHUMBNAILSPREVIEWWIDGET

#include "ui_qthumbnailsspreviewwidgetbase.h"
#include <QWidget>
#include <QHash>
#include <QStringList>
#include <QAbstractItemView>

class QPixmap;
class QListWidgetItem;

namespace udg {


/**
This class is a widget that allows us to preview thumbnails.
We can add thumbnails, remove, know which is selected, ...
*/
class QThumbnailsPreviewWidget : public QWidget, private Ui::QThumbnailsPreviewWidgetBase {
    Q_OBJECT
public:
    ///Class builder
    QThumbnailsPreviewWidget(QWidget *parent = 0);

    /// We add the thumbnail at the end. The ID parameter is a unique identifier of the thumbnail.
    void append(QString IDThumbnail, const QPixmap &thumbnail, QString thumbnailDescription);

    /// Inserts the thumbnail to the specified position.
    /// If the position is greater than the number of current items, insert it at the end
    void insert(int position, QString IDThumbnail, const QPixmap &thumbnail, QString thumbnailDescription);

    /// Remove thumbnail from preview.
    void remove(QString IDThumbnail);

    /// Select Thumbnail with last ID
    void setCurrentThumbnail(QString IDThumbnail);

    ///Returns the selected Thumbnail ID
    QStringList getSelectedThumbnailsID();

    /// Assigns / Gets the selection mode of the control's Thumbnails. The default value is SingleSelection
    void setSelectionMode(QAbstractItemView::SelectionMode selectionMode);
    QAbstractItemView::SelectionMode getSelectionMode();

public slots:
    ///Clean the ListWidget of series
    void clear();

signals:
    /// Signal that is emitted when they have clicked a thumbnail
    void thumbnailClicked(const QString &IDThumbnail);
    /// Signal that is emitted when they have double clicked on a thumbnail
    void thumbnailDoubleClicked(const QString IDThumbnail);

private:
    /// We create a QListWidgetItem from the past data
    QListWidgetItem* createQListWidgetItem(QString IDThumbnail, const QPixmap &thumbnail, QString thumbnailDescription);

    /// Returns a getQListWidgetItem from its ID, if it does not find it returns null
    QListWidgetItem *getQListWidgetItem(QString IDThumbnail);

    ///Creates signal and slot connections
    void createConnections();

private slots:
    /// Slot that is activated when a series is selected,
    /// sends signal to QStudyTreeWidget, because it selects the same series as QSeriesListWidget
    /// @param selected series
    void itemClicked(QListWidgetItem *item);

    /// Slot that is activated when folding is done
    /// @param item of the Selected series
    void itemDoubleClicked(QListWidgetItem *item);

private:
    ///Image size scaled x-axis
    static const int scaledSeriesSizeX;
    ///Image size scaled on y axis
    static const int scaledSeriesSizeY;

    /// We save for each series to which studio it belongs
    QHash<QString, QString> m_HashSeriesStudy;
    ///Modalities of series that are not images, like (KO, PR, SR)
    QStringList m_nonDicomImageSeriesList;

    //Indicates to which last row we inserted a series that was an image
    int m_lastInsertedImageRow;
};

}

#endif
