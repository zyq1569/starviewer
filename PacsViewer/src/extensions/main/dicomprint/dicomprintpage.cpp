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

#include "dicomprintpage.h"

#include <QRegExp>
#include <QStringList>

#include "dicomprinter.h"

namespace udg {

void DicomPrintPage::setFilmSize(const QString &filmSize)
{
    m_filmSize = filmSize;
}

QString DicomPrintPage::getFilmSize() const
{
    return m_filmSize;
}

void DicomPrintPage::setFilmLayout(const QString &filmLayout)
{
    m_filmLayout = filmLayout;
}

QString DicomPrintPage::getFilmLayout() const
{
    return m_filmLayout;
}

int DicomPrintPage::getFilmLayoutColumns()
{
    // Hack per obtenir el número de columnes
    DicomPrinter dicomPrinter;

    dicomPrinter.setDefaultFilmLayout(m_filmLayout);

    return dicomPrinter.getDefaultFilmLayoutColumns();
}

int DicomPrintPage::getFilmLayoutRows()
{
    // Hack per obtenir el número de files
    DicomPrinter dicomPrinter;

    dicomPrinter.setDefaultFilmLayout(m_filmLayout);

    return dicomPrinter.getDefaultFilmLayoutRows();
}

void DicomPrintPage::setFilmOrientation(const QString &filmOrientation)
{
    m_filmOrientation = filmOrientation;
}

QString DicomPrintPage::getFilmOrientation() const
{
    return m_filmOrientation;
}

void DicomPrintPage::setMagnificationType(const QString &magnificationType)
{
    m_magnificationType = magnificationType;
}

QString DicomPrintPage::getMagnificationType() const
{
    return m_magnificationType;
}

void DicomPrintPage::setMaxDensity(ushort maxDensity)
{
    m_maxDensity = maxDensity;
}

ushort DicomPrintPage::getMaxDensity()
{
    return m_maxDensity;
}

void DicomPrintPage::setMinDensity(ushort minDensity)
{
    m_minDensity = minDensity;
}

ushort DicomPrintPage::getMinDensity()
{
    return m_minDensity;
}

void DicomPrintPage::setTrim(bool trim)
{
    m_trim = trim;
}

bool DicomPrintPage::getTrim()
{
    return m_trim;
}

void DicomPrintPage::setBorderDensity(const QString &borderDensity)
{
    m_borderDensity = borderDensity;
}

QString DicomPrintPage::getBorderDensity() const
{
    return m_borderDensity;
}

void DicomPrintPage::setEmptyImageDensity(const QString &emptyImageDensity)
{
    m_emptyImageDensity = emptyImageDensity;
}

QString DicomPrintPage::getEmptyImageDensity() const
{
    return m_emptyImageDensity;
}

void DicomPrintPage::setSmoothingType(const QString &smoothingType)
{
    m_smoothingType = smoothingType;
}

QString DicomPrintPage::getSmoothingType() const
{
    return m_smoothingType;
}

void DicomPrintPage::setPolarity(const QString &polarity)
{
    m_polarity = polarity;
}

QString DicomPrintPage::getPolarity() const
{
    return m_polarity;
}

void DicomPrintPage::setConfigurationInformation(const QString &configurationInformation)
{
    m_configurationInformation = configurationInformation;
}

QString DicomPrintPage::getConfigurationInformation() const
{
    return m_configurationInformation;
}

void DicomPrintPage::addAnnotation(int position, const QString &annotationText)
{
    // No sé perquè però a DCMTK si li passem una anotació amb el text buit, ja no imprimeix cap de les altres anotacions
    if (annotationText.length() > 0)
    {
        /// Mirant el DICOM Conformance de Kodak, Agfa, Sony, Codonics,.. veiem que cap deixa imprimir anotacions de més de 64 caràcters
        m_pageAnnotations.insert(position, annotationText.left(64));
    }
}

QMap<int, QString> DicomPrintPage::getPageAnnotations()
{
    return m_pageAnnotations;
}

void DicomPrintPage::setPageNumber(int pageNumber)
{
    m_pageNumber = pageNumber;
}

int DicomPrintPage::getPageNumber()
{
    return m_pageNumber;
}

void DicomPrintPage::setImagesToPrint(QList<QPair<Image*, DICOMPrintPresentationStateImage> > imagesToPrint)
{
    m_imagesToPrint = imagesToPrint;
}

QList<QPair<Image*, DICOMPrintPresentationStateImage> > DicomPrintPage::getImagesToPrint()
{
    return m_imagesToPrint;
}

}
