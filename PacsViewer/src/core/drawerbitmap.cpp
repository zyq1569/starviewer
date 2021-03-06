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

#include "drawerbitmap.h"

#include "logging.h"
#include "mathtools.h"
#include "volumepixeldata.h"
#include "vtkimagedatacreator.h"

#include <QColor>

#include <vtkImageActor.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>

namespace udg {

DrawerBitmap::DrawerBitmap(QObject *parent)
    : DrawerPrimitive(parent)
{
    m_origin[0] = m_origin[1] = m_origin[2] = 0.0;
    m_spacing[0] = m_spacing[1] = m_spacing[2] = 1.0;
    m_width = m_height = 0;
    m_data = 0;
    m_backgroundOpacity = 0.0;
    m_backgroundColor = Qt::black;
    m_foregroundColor = Qt::white;
    m_imageActor = 0;
}

DrawerBitmap::~DrawerBitmap()
{
    emit dying(this);
}

void DrawerBitmap::setOrigin(double origin[3])
{
    memcpy(m_origin, origin, 3 * sizeof(double));
}

void DrawerBitmap::setSpacing(double spacing[3])
{
    memcpy(m_spacing, spacing, 3 * sizeof(double));
}

void DrawerBitmap::setData(unsigned int width, unsigned int height, unsigned char *data)
{
    m_width = width;
    m_height = height;
    m_data = data;
}

void DrawerBitmap::setBackgroundOpacity(double opacity)
{
    m_backgroundOpacity = opacity;
}

void DrawerBitmap::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

void DrawerBitmap::setForegroundColor(const QColor &color)
{
    m_foregroundColor = color;
}

vtkProp* DrawerBitmap::getAsVtkProp()
{
    if (!m_imageActor)
    {
        if (m_data)
        {
            VtkImageDataCreator imageDataCreator;
            imageDataCreator.setOrigin(m_origin);
            imageDataCreator.setSpacing(m_spacing);
            vtkSmartPointer<vtkImageData> imageData = imageDataCreator.createVtkImageData(m_width, m_height, 1, m_data);

            /// We build LUT to apply transparencies: made from the
            /// code of http://www.vtk.org/Wiki/VTK/Examples/Cxx/Images/Transparency
            vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
            lookupTable->SetNumberOfTableValues(2);
            lookupTable->SetRange(0.0, 1.0);
            // Valor 0 has m_backgroundOpacity i color  m_backgroundColor
            lookupTable->SetTableValue(0, m_backgroundColor.redF(), m_backgroundColor.greenF(), m_backgroundColor.blueF(), m_backgroundOpacity);
            // Value 1 has opaque with color m_foregroundColor
            lookupTable->SetTableValue(1, m_foregroundColor.redF(), m_foregroundColor.greenF(), m_foregroundColor.blueF(), 1.0);
            lookupTable->Build();

            vtkSmartPointer<vtkImageMapToColors> mapTransparency = vtkSmartPointer<vtkImageMapToColors>::New();
            mapTransparency->SetLookupTable(lookupTable);
            mapTransparency->SetInputData(imageData);
            mapTransparency->PassAlphaToOutputOn();

            // We create the actor
            m_imageActor = vtkSmartPointer<vtkImageActor>::New();
            mapTransparency->Update();
            m_imageActor->SetInputData(mapTransparency->GetOutput());
            m_imageActor->SetDisplayExtent(0, m_width - 1, 0, m_height - 1, 0, 0);
            m_imageActor->SetVisibility(this->isVisible());
        }
        else
        {
            DEBUG_LOG("Error passing bitmap data to vtkImageActor format or no data");
        }
    }

    return m_imageActor;
}

double DrawerBitmap::getDistanceToPoint(double *point3D, double closestPoint[3])
{
    /// If the point is inside the bitmap box (X, Y coordinates only),
    /// we will return distance 0 and as closestPoint the point3D
    /// If the dot is outside the bitmap box,
    /// the distance will be the distance to the edge closest to the point
    double bounds[6];
    getBounds(bounds);

    if (point3D[0] >= bounds[0] && point3D[0] <= bounds[1] &&
            point3D[1] >= bounds[2] && point3D[1] <= bounds[3])
    {
        memcpy(closestPoint, point3D, 3 * sizeof(double));
        return 0.0;
    }

    double topLeftCorner[3] = { bounds[0], bounds[2], bounds[4] };
    double topRightCorner[3] = { bounds[1], bounds[2], bounds[4] };
    double bottomRightCorner[3] = { bounds[1], bounds[3], bounds[4] };
    double bottomLeftCorner[3] = { bounds[0], bounds[3], bounds[4] };
    
    double minimumDistanceFound = MathTools::DoubleMaximumValue;
    double distance;
    double localClosestPoint[3];

    distance = MathTools::getPointToFiniteLineDistance(point3D, topLeftCorner, topRightCorner, localClosestPoint);
    if (distance < minimumDistanceFound)
    {
        minimumDistanceFound = distance;
        memcpy(closestPoint, localClosestPoint, 3 * sizeof(double));
    }

    distance = MathTools::getPointToFiniteLineDistance(point3D, topRightCorner, bottomRightCorner, localClosestPoint);
    if (distance < minimumDistanceFound)
    {
        minimumDistanceFound = distance;
        memcpy(closestPoint, localClosestPoint, 3 * sizeof(double));
    }
    
    distance = MathTools::getPointToFiniteLineDistance(point3D, bottomRightCorner, bottomLeftCorner, localClosestPoint);
    if (distance < minimumDistanceFound)
    {
        minimumDistanceFound = distance;
        memcpy(closestPoint, localClosestPoint, 3 * sizeof(double));
    }
    
    distance = MathTools::getPointToFiniteLineDistance(point3D, bottomLeftCorner, topLeftCorner, localClosestPoint);
    if (distance < minimumDistanceFound)
    {
        minimumDistanceFound = distance;
        memcpy(closestPoint, localClosestPoint, 3 * sizeof(double));
    }

    return minimumDistanceFound;
}

void DrawerBitmap::getBounds(double bounds[6])
{
    bounds[0] = m_origin[0];
    bounds[1] = m_origin[0] + m_spacing[0] * m_width;

    bounds[2] = m_origin[1];
    bounds[3] = m_origin[1] + m_spacing[1] * m_height;

    bounds[4] = m_origin[2];
    bounds[5] = m_origin[2] + m_spacing[2];
}

void DrawerBitmap::update()
{
    switch (m_internalRepresentation)
    {
    case VTKRepresentation:
        updateVtkProp();
        break;

    case OpenGLRepresentation:
        break;
    }
}

void DrawerBitmap::updateVtkProp()
{
    if (m_imageActor)
    {
        /// TODO At the moment we only see that once the bitmap is created,
        /// the only thing we can modify will be its visibility
        m_imageActor->SetVisibility(this->isVisible());
        this->setModified(false);
    }
    else
    {
        DEBUG_LOG("The bitmap cannot be updated, as the actor has not yet been created");
    }
}

} // end namespace udg
