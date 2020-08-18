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

#include "qseriesthumbnailpreviewwidget.h"

#include <QString>

#include "series.h"

namespace udg {

QSeriesThumbnailPreviewWidget::QSeriesThumbnailPreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    m_DICOMModalitiesNonImage << "KO" << "PR" << "SR";
    m_positionOfLastInsertedThumbnail = -1;

    createConnections();
}

void QSeriesThumbnailPreviewWidget::insertSeries(QString studyInstanceUID, Series *series)
{
    QString seriesThumbnailDescription = getSeriesThumbnailDescription(series);
    m_studyInstanceUIDBySeriesInstanceUID[series->getInstanceUID()] = studyInstanceUID;

    // Comprovem la posició que hem d'inserir la sèrie, si és un DICOM Non-Image (no és una imatge) val final, sinó va després de la última imatge inserida
    if (m_DICOMModalitiesNonImage.contains(series->getModality()))
    {
        m_seriesThumbnailsPreviewWidget->append(series->getInstanceUID(), series->getThumbnail(), seriesThumbnailDescription);
    }
    else
    {
        // És una imatge
        m_positionOfLastInsertedThumbnail++;
        m_seriesThumbnailsPreviewWidget->insert(m_positionOfLastInsertedThumbnail, series->getInstanceUID(), series->getThumbnail(), seriesThumbnailDescription);
    }
}

void QSeriesThumbnailPreviewWidget::removeSeries(const QString &seriesInstanceUID)
{
    m_seriesThumbnailsPreviewWidget->remove(seriesInstanceUID);
}

void QSeriesThumbnailPreviewWidget::setCurrentSeries(const QString &seriesUID)
{
    m_seriesThumbnailsPreviewWidget->setCurrentThumbnail(seriesUID);
}

void QSeriesThumbnailPreviewWidget::clear()
{
    m_seriesThumbnailsPreviewWidget->clear();
    m_studyInstanceUIDBySeriesInstanceUID.clear();
    // Indiquem que la última imatge insertada està a la posició 0 perquè hem un clear
    m_positionOfLastInsertedThumbnail = -1;
}

void QSeriesThumbnailPreviewWidget::createConnections()
{
    connect(m_seriesThumbnailsPreviewWidget, SIGNAL(thumbnailClicked(QString)), this, SLOT(seriesClicked(QString)));
    connect(m_seriesThumbnailsPreviewWidget, SIGNAL(thumbnailDoubleClicked(QString)), this, SLOT(seriesDoubleClicked(QString)));
}

QString QSeriesThumbnailPreviewWidget::getSeriesThumbnailDescription(Series *series)
{
    QString thumbnailDescription;
    thumbnailDescription = tr(" Series ") + series->getSeriesNumber();

    if (series->getProtocolName().length() > 0)
    {
        thumbnailDescription += " " + series->getProtocolName();
    }
    thumbnailDescription +="\n";

    if (series->getNumberOfImages() > 0)
    {
        thumbnailDescription += QString::number(series->getNumberOfImages());

        if (series->getModality() == "KO")
        {
            thumbnailDescription += tr(" Key Object Note");
        }
        else if (series->getModality() == "PR")
        {
            thumbnailDescription += tr(" Presentation State");
        }
        else if (series->getModality() == "SR")
        {
            thumbnailDescription += tr(" Structured Report");
        }
        else
        {
            thumbnailDescription += tr(" Images");
        }
    }

    if (series->getNumberOfEncapsulatedDocuments() > 0)
    {
        thumbnailDescription += QString(tr("%1 Encapsulated Document(s)")).arg(series->getNumberOfEncapsulatedDocuments());
    }

    return thumbnailDescription;
}

void QSeriesThumbnailPreviewWidget::seriesClicked(QString IDThumbnail)
{
    emit(seriesThumbnailClicked(m_studyInstanceUIDBySeriesInstanceUID[IDThumbnail], IDThumbnail));
}

void QSeriesThumbnailPreviewWidget::seriesDoubleClicked(QString IDThumbnail)
{
    emit(seriesThumbnailDoubleClicked(m_studyInstanceUIDBySeriesInstanceUID[IDThumbnail], IDThumbnail));
}

}
