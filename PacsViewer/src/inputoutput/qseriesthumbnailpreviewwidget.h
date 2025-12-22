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

#ifndef UDGQSERIESTHUMBNAILPREVIEWTWIDGET_H
#define UDGQSERIESTHUMBNAILPREVIEWTWIDGET_H

#include "ui_qseriesthumbnailpreviewwidgetbase.h"

namespace udg {

class Series;

/**
   Show us a thumbnail preview of past series in the Widget.
*/
class QSeriesThumbnailPreviewWidget : public QWidget, private Ui::QSeriesThumbnailPreviewWidgetBase {
    Q_OBJECT
public:
    /// Class builder
    QSeriesThumbnailPreviewWidget(QWidget *parent = 0);

    /// Inserts information from a series
    void insertSeries(QString studyInstanceUID, Series *series);

    /// Deletes the series from the list with the UID passed by parameter
    void removeSeries(const QString &seriesInstanceUID);

public slots:
    ///Sets which series is selected
    void setCurrentSeries(const QString &seriesUID);

    ///Clears the widget and leaves it without any Series thumbnail
    void clear();

signals:
    /// When we select a series it emits a signal
    /// for QStudyTreeWidget to have the same series selected
    void seriesThumbnailClicked(const QString studyInstanceUID, const QString seriesInstanceUID);

    /// When double-clicked it emits a signal, so that the series is displayed
    void seriesThumbnailDoubleClicked(QString studyInstanceUID, QString seriesInstanceUID);

private:
    /// Create connections
    void createConnections();

    /// Returns the description by the thumbnail of the series
    QString getSeriesThumbnailDescription(Series *series);

private slots:
    /// Slot that is activated when a thumbnail is clicked
    void seriesClicked(QString IDThumbnail);

    /// Slot that is activated when a double click has been made on a thumbnail
    void seriesDoubleClicked(QString IDThumbnail);

private:
    //We save for each series to which studio it belongs
    QHash<QString, QString> m_studyInstanceUIDBySeriesInstanceUID;

    //Modalities of series that are not images, like (KO, PR, SR)
    QStringList m_DICOMModalitiesNonImage;
    //Indicates to which last row we inserted a series that was an image
    int m_positionOfLastInsertedThumbnail;
};

}

#endif
