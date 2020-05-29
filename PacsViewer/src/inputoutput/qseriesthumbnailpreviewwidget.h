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
    Ens mostrar una previsualització en forma de thumbnail de les sèries passades al Widget.
  */
class QSeriesThumbnailPreviewWidget : public QWidget, private Ui::QSeriesThumbnailPreviewWidgetBase {
Q_OBJECT
public:
    /// Constructor de la classe
    QSeriesThumbnailPreviewWidget(QWidget *parent = 0);

    /// Insereix l'informació d'una sèrie
    void insertSeries(QString studyInstanceUID, Series *series);

    /// Esborra de la llista la serie amb el UID passat per paràmetre
    void removeSeries(const QString &seriesInstanceUID);

public slots:
    /// Estableix quina és la série seleccionada
    void setCurrentSeries(const QString &seriesUID);

    /// Neteja el widget i el deixa sense cap thumbnail de Series
    void clear();

signals:
    /// Quan seleccionem una sèrie emeiteix una signal per a que el QStudyTreeWidget, tingui seleccionada la mateixa sèrie
    void seriesThumbnailClicked(const QString studyInstanceUID, const QString seriesInstanceUID);

    /// Quan es fa doble click emiteix un signal, perquè la sèrie sigui visualitzada
    void seriesThumbnailDoubleClicked(QString studyInstanceUID, QString seriesInstanceUID);

private:
    /// Crea les connexions
    void createConnections();

    /// Retorna la descripció pel thumbnail de la sèrie
    QString getSeriesThumbnailDescription(Series *series);

private slots:
    /// Slot que s'activa quan s'ha fet click sobre un thumbnail
    void seriesClicked(QString IDThumbnail);

    /// Slot que s'activa quan s'ha fet doble click sobre un thumbnail
    void seriesDoubleClicked(QString IDThumbnail);

private:
    //Guardem per cada sèrie a quin estudi pertany
    QHash<QString, QString> m_studyInstanceUIDBySeriesInstanceUID;

    //Modalitats de sèries que no són images, com (KO, PR, SR)
    QStringList m_DICOMModalitiesNonImage;
    //Indica a quina ha estat la última fila que hem inseritat una sèrie que era una imatge
    int m_positionOfLastInsertedThumbnail;
};

}

#endif
