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

#ifndef UGDIMAGEORIENTATION_H
#define UGDIMAGEORIENTATION_H

#include <QString>
#include <QVector3D>

namespace udg {
/**
This class encapsulates the DICOM Image Orientation (Patient) attribute (0020,0037) which defines the directions of the vectors in the first row and column
of the image with respect to the patient. For more information see PS 3.3, section C.7.6.2.1.1.

In DICOM format this attribute consists of 6 values separated by '\', the first 3 being the vector of the row and the last 3 the vector of the column.

This class also keeps the normal of the plane formed by these two vectors.
*/
class ImageOrientation {
public:
    ImageOrientation();
    ~ImageOrientation();
    
    /// Builder from two 3D vectors
    ImageOrientation(const QVector3D &rowVector, const QVector3D &columnVector);
    
    /// Assigns the orientation provided in the format stipulated by DICOM: 6 numeric values separated by '\' or an empty string.
    /// If the string is not in the expected format, the values of the vectors are re-initialized and returned false, otherwise true.
    bool setDICOMFormattedImageOrientation(const QString &imageOrientation);

    /// Returns the orientation in the format stipulated by DICOM: 6 numeric values separated by '\'
    /// If no value has been entered previously, an empty string will be returned
    QString getDICOMFormattedImageOrientation() const;

    ///Assign the orientation through the 2 3D vectors corresponding to the row and column directions respectively
    void setRowAndColumnVectors(const QVector3D &rowVector, const QVector3D &columnVector);
    
    /// Returns the row, column and normal vectors respectively.
    /// In case no orientation has been assigned, the vectors will have no element
    QVector3D getRowVector() const;
    QVector3D getColumnVector() const;
    QVector3D getNormalVector() const;

    /// Equality operator
    bool operator==(const ImageOrientation &imageOrientation) const;

private:
    /// It returns the orientation of the two vectors in a string in the format stipulated by DICOM
    QString convertToDICOMFormatString(const QVector3D &rowVector, const QVector3D &columnVector) const;

    /// Initialize vectors to (0,0,0)
    void setVectorValuesToDefault();

private:
    /// The 3 vectors of image orientation.
    /// Row and column vectors will be obtained directly from
    /// the string assigned with setDICOMFormattedImageOrientation ()
    /// The normal vector of the image orientation will be
    /// calculated by making the vector product of the row and column vectors
    QVector3D m_rowVector;
    QVector3D m_columnVector;
    QVector3D m_normalVector;
};

} // End namespace udg

#endif
