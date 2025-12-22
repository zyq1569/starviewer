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

#ifndef UDGDISPLAYSHUTTER_H
#define UDGDISPLAYSHUTTER_H

#include <QPolygon>

#include "vtkSmartPointer.h"

class QColor;
class QImage;

class vtkImageData;

namespace udg {

/*
A shutter display consists of a geometric shape (circle, rectangle or polygon)
 which defines the area that should be visible in the associated image. The part contained within the form is the visible and the
 what is left out is the occluded part.
 */
class DisplayShutter {
public:
    DisplayShutter();
    ~DisplayShutter();

    enum ShapeType { UndefinedShape = 0, RectangularShape, PolygonalShape, CircularShape };
    
    /// Assign / Get the shape of the shutter
    void setShape(ShapeType shape);
    ShapeType getShape() const;

    /// It returns the form to us in a text string according to the values established by the DICOM
    QString getShapeAsDICOMString() const;
    
    /// Value of gray with which the opaque part of the shutter should be painted
    void setShutterValue(unsigned short int value);
    unsigned short int getShutterValue() const;

    /// It returns the value of the shutter in the form of QColor
    QColor getShutterValueAsQColor() const;
    
    /// Methods for assigning points for each type of shape. In case the dot format does not match
    /// with the form already assigned, it will return false and no points will be saved. In case no form has been defined
    /// (shape = UndefinedShape) the method will automatically assign the appropriate shape
    /// by that method saving the indicated points.
    /// Assign the center and radius to the circular shape
    bool setPoints(const QPoint &centre, int radius);
    
    ///Assign the vertices to the polygonal shape
    bool setPoints(const QVector<QPoint> &vertices);
    
    /// Assign the upper left and lower right points that define the rectangular shape
    bool setPoints(const QPoint &topLeft, const QPoint &bottomRight);

    /// Assign points in text format. Each point will have the form x, x, two integer values (x) separated by commas (,)
    /// Each dot in the list will be separated by semicolons (;)
    /// For the rectangular shape we will have 2 points corresponding to the point
    /// upper left and at the lower right point of the rectangle
    /// For the circular shape the format will be center, radius where radius will be an integer value
    /// For the polygonal shape the format will be a list of points with a minimum of 3 elements
    /// In case the dot format does not match the shape of the shutter, it will return false, otherwise true
    /// If the form is indefinite, it will try to see if it corresponds
    /// with some defined point format and will assign it in case you find a match
    bool setPoints(const QString &pointsString);
    
    /// Retorna el shutter en forma de QPolygon
    QPolygon getAsQPolygon() const;

    /// Returns the shutter points in string format. He
    /// format will be the same as setPoints (const QString &)
    QString getPointsAsString() const;

    ///Returns the shutter as a QImage of width and height sizes
    QImage getAsQImage(int width, int height) const;

    /// Returns the shutter in vtkImageData format, with extent defined by given width and height.
    vtkSmartPointer<vtkImageData> getAsVtkImageData(int width, int height) const;
    
    ///Given a list of shutters, it returns the shutter resulting from the intersection of these.
    /// As for the resulting color, it will be the average of all the shutters in the list.
    static DisplayShutter intersection(const QList<DisplayShutter> &shuttersList);

private:
    /// Check that the format of the dot string is correct according to the given shape
    bool shapeMatchesPointsStringFormat(ShapeType shape, const QString &pointsString);

private:
    /// Forma del shutter
    ShapeType m_shape;

    /// Polygon that defines the shutter. Regardless of
    /// the shape you have defined will always be saved internally as a polygon.
    QPolygon m_shutterPolygon;
    
    ///Value of gray with which to paint the opaque part of the shutter
    unsigned short int m_shutterValue;
};

} // End namespace udg

#endif
