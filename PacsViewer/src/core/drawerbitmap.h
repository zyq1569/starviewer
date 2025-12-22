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

#ifndef UDGDRAWERBITMAP_H
#define UDGDRAWERBITMAP_H

#include "drawerprimitive.h"

#include <vtkSmartPointer.h>

class QColor;
class vtkImageActor;

namespace udg {

/**
Class that represents a primitive by bitmap. This map has a 1-bit color depth.
therefore it only accepts two different values. The bitmap will be defined by its length, height and data (byte array).

It allows us to define what is the background color (background), its opacity and what is the main color (foreground).
The value 0 will be taken as the background value, any other different value will be taken as the main color.

It also allows us to assign a source and spacing so we can adjust it to the image we are seeing.
 */
class DrawerBitmap : public DrawerPrimitive {
    Q_OBJECT
public:
    DrawerBitmap(QObject *parent = 0);
    ~DrawerBitmap();

    ///Assign the origin TODO Maybe with dimension 2 it would be enough
    void setOrigin(double origin[3]);
    
    ///Assign spacing TODO Maybe with dimension 2 it would be enough
    void setSpacing(double spacing[3]);

    ///Assign bitmap sizes and their data.
    /// The number of elements to be counted from date will be width * height
    void setData(unsigned int width, unsigned int height, unsigned char *data);
    
    /// Determines the opacity of the background value
    void setBackgroundOpacity(double opacity);

    /// Determines the color with which the background color is mapped
    void setBackgroundColor(const QColor &color);

    /// Determines the color with which the main color is mapped
    void setForegroundColor(const QColor &color);

    vtkProp* getAsVtkProp();

    double getDistanceToPoint(double *point3D, double closestPoint[3]);

    void getBounds(double bounds[6]);

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    /// Bitmap source
    double m_origin[3];

    ///Bitmap spacing
    double m_spacing[3];

    ///Bitmap sizes
    unsigned int m_width, m_height;
    
    /// Array with bitmap data
    unsigned char *m_data;
    
    /// Opacity of the background value. Its default value will be 0.0, ie transparent
    double m_backgroundOpacity;

    /// Color with which the background value will be painted. Its default value will be Qt :: black
    QColor m_backgroundColor;

    ///Color with which to paint what is not the background. Its default value will be Qt :: white
    QColor m_foregroundColor;

    /// Vtk actor with which we will paint the bitmap
    vtkSmartPointer<vtkImageActor> m_imageActor;
};

} // end namespace udg

#endif // UDGDRAWERBITMAP_H
