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

#include "imageorientation.h"

#include <QStringList>

#include "dicomvaluerepresentationconverter.h"
#include "logging.h"

namespace udg {

// Constant to define what we consider an empty vector
const QVector3D EmptyVector(.0,.0,.0);

ImageOrientation::ImageOrientation()
{
    setVectorValuesToDefault();
}

ImageOrientation::~ImageOrientation()
{
}

ImageOrientation::ImageOrientation(const QVector3D &rowVector, const QVector3D &columnVector)
{
    setRowAndColumnVectors(rowVector, columnVector);
}

bool ImageOrientation::setDICOMFormattedImageOrientation(const QString &imageOrientation)
{
    if (imageOrientation.isEmpty())
    {
        setVectorValuesToDefault();

        return true;
    }

    //From here we check that there is the correct number of elements
    QStringList list = imageOrientation.split(DICOMValueRepresentationConverter::ValuesSeparator);
    if (list.size() != 6)
    {
        DEBUG_LOG("Image Orientation (Patient) does not have the 6 expected items. DICOM inconsistency.");
        ERROR_LOG("Image Orientation (Patient) does not have the 6 expected items. DICOM inconsistency");

        return false;
    }

    // Tenim 6 elements, cal comprovar que siguin valors vàlids
    bool validValues;
    QVector<double> convertedValues = DICOMValueRepresentationConverter::decimalStringToDoubleVector(imageOrientation, &validValues);

    if (!validValues)
    {
        DEBUG_LOG("Image Orientation (Patient) values are invalid. DICOM inconsistency.");
        ERROR_LOG("Image Orientation (Patient) values are invalid. DICOM inconsistency.");
        
        return false;
    }

    // At this point, we have 6 values and they are valid
    // We assign the converted values to the corresponding vectors
    QVector3D row(convertedValues.at(0), convertedValues.at(1), convertedValues.at(2));
    QVector3D column(convertedValues.at(3), convertedValues.at(4), convertedValues.at(5));
    setRowAndColumnVectors(row, column);

    return true;
}

QString ImageOrientation::getDICOMFormattedImageOrientation() const
{
    if (m_rowVector == EmptyVector && m_columnVector == EmptyVector)
    {
        return QString();
    }
    else
    {
        return this->convertToDICOMFormatString(m_rowVector, m_columnVector);
    }
}

void ImageOrientation::setRowAndColumnVectors(const QVector3D &rowVector, const QVector3D &columnVector)
{
    m_rowVector = rowVector;
    m_columnVector = columnVector;
    m_normalVector = QVector3D::crossProduct(m_rowVector, m_columnVector);
}

QVector3D ImageOrientation::getRowVector() const
{
    return m_rowVector;
}

QVector3D ImageOrientation::getColumnVector() const
{
    return m_columnVector;
}

QVector3D ImageOrientation::getNormalVector() const
{
    return m_normalVector;
}

bool ImageOrientation::operator==(const ImageOrientation &imageOrientation) const
{
    return (m_rowVector == imageOrientation.m_rowVector) && (m_columnVector == imageOrientation.m_columnVector);
}

QString ImageOrientation::convertToDICOMFormatString(const QVector3D &rowVector, const QVector3D &columnVector) const
{
    QString imageOrientationString;

    imageOrientationString = QString::number(rowVector.x()) + DICOMValueRepresentationConverter::ValuesSeparator;
    imageOrientationString += QString::number(rowVector.y()) + DICOMValueRepresentationConverter::ValuesSeparator;
    imageOrientationString += QString::number(rowVector.z()) + DICOMValueRepresentationConverter::ValuesSeparator;

    imageOrientationString += QString::number(columnVector.x()) + DICOMValueRepresentationConverter::ValuesSeparator;
    imageOrientationString += QString::number(columnVector.y()) + DICOMValueRepresentationConverter::ValuesSeparator;
    imageOrientationString += QString::number(columnVector.z());

    return imageOrientationString;
}

void ImageOrientation::setVectorValuesToDefault()
{
    m_rowVector = EmptyVector;
    m_columnVector = EmptyVector;
    m_normalVector = EmptyVector;
}

} // End namespace udg
