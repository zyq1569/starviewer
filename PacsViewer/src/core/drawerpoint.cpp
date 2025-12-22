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

#include "drawerpoint.h"
#include "logging.h"
// Vtk
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>

namespace udg {

DrawerPoint::DrawerPoint(QObject *parent)
    : DrawerPrimitive(parent), m_pointRadius(2.0), m_pointActor(0), m_pointSphere(0), m_pointMapper(0)
{
}

DrawerPoint::~DrawerPoint()
{
    if (m_pointActor)
    {
        m_pointActor->Delete();
    }
    if (m_pointSphere)
    {
        m_pointSphere->Delete();
    }
    if (m_pointMapper)
    {
        m_pointMapper->Delete();
    }
}

void DrawerPoint::setPosition(double point[3])
{
    m_position[0] = point[0];
    m_position[1] = point[1];
    m_position[2] = point[2];

    emit changed();
}

void DrawerPoint::setPosition(QVector<double> point)
{
    m_position[0] = point[0];
    m_position[1] = point[1];
    m_position[2] = point[2];

    emit changed();
}

void DrawerPoint::setRadius(double radius)
{
    m_pointRadius = radius;
}

vtkProp* DrawerPoint::getAsVtkProp()
{
    if (!m_pointActor)
    {
        // We create the m_vtkActor pipeline
        m_pointActor = vtkActor::New();
        m_pointSphere = vtkSphereSource::New();
        m_pointMapper = vtkPolyDataMapper::New();
        m_pointMapper->SetInputConnection(m_pointSphere->GetOutputPort());
        m_pointActor->SetMapper(m_pointMapper);
    }

    // We give it the attributes
    m_pointSphere->SetCenter(m_position);

    updateVtkActorProperties();

    return m_pointActor;
}

void DrawerPoint::update()
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

void DrawerPoint::updateVtkProp()
{
    if (m_pointActor)
    {
        // We assign the properties of the point
        m_pointSphere->SetCenter(m_position);
        updateVtkActorProperties();
        this->setModified(false);
    }
    else
    {
        DEBUG_LOG("The point cannot be updated, as it has not yet been created!");
    }
}

void DrawerPoint::updateVtkActorProperties()
{
    vtkProperty *properties = m_pointActor->GetProperty();

    // Coordinate system
    // TODO It is a 3D mapper and therefore does not have this method It remains to be seen if this can mean any
    // problem with the management of coordinate systems
    // m_pointMapper-> SetTransformCoordinate (this-> getVtkCoordinateObject ());

    // Assign thickness of the sphere
    m_pointSphere->SetRadius(m_pointRadius);

    //We assign opacity to the point
    properties->SetOpacity(m_opacity);

    // Mirem la visibilitat de l'm_vtkActor
    m_pointActor->SetVisibility(this->isVisible());

    // Assignem color
    QColor color = this->getColor();
    properties->SetColor(color.redF(), color.greenF(), color.blueF());
}

double DrawerPoint::getDistanceToPoint(double *point3D, double closestPoint[3])
{
    closestPoint[0] = m_position[0];
    closestPoint[1] = m_position[1];
    closestPoint[2] = m_position[2];

    return sqrt((point3D[0] - m_position[0]) * (point3D[0] - m_position[0]) + (point3D[1] - m_position[1]) * (point3D[1] - m_position[1]) +
            (point3D[2] - m_position[2]) * (point3D[2] - m_position[2]));
}

void DrawerPoint::getBounds(double bounds[6])
{
    for (int i = 0; i < 3; i++)
    {
        bounds[i * 2] = m_position[i] - m_pointRadius;
        bounds[i * 2 + 1] = m_position[i] + m_pointRadius;
    }
}

}
