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

#include "drawerline.h"
#include "logging.h"
#include "mathtools.h"
// Vtk
#include <vtkLineSource.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkActor2D.h>
#include <vtkPropAssembly.h>

namespace udg {

DrawerLine::DrawerLine(QObject *parent)
    : DrawerPrimitive(parent), m_vtkLineSource(0), m_vtkActor(0),
      m_vtkBackgroundActor(0), m_vtkMapper(0), m_vtkPropAssembly(0)
{
}

DrawerLine::~DrawerLine()
{
    emit dying(this);

    if (m_vtkActor)
    {
        m_vtkActor->Delete();
    }

    if (m_vtkBackgroundActor)
    {
        m_vtkBackgroundActor->Delete();
    }

    if (m_vtkLineSource)
    {
        m_vtkLineSource->Delete();
    }

    if (m_vtkMapper)
    {
        m_vtkMapper->Delete();
    }
}

void DrawerLine::setFirstPoint(double point[3])
{
    this->setFirstPoint(point[0], point[1], point[2]);
}

void DrawerLine::setFirstPoint(double x, double y, double z)
{
    m_firstPoint[0] = x;
    m_firstPoint[1] = y;
    m_firstPoint[2] = z;

    emit changed();
}

void DrawerLine::setSecondPoint(double point[3])
{
    this->setSecondPoint(point[0], point[1], point[2]);
}

void DrawerLine::setSecondPoint(double x, double y, double z)
{
    m_secondPoint[0] = x;
    m_secondPoint[1] = y;
    m_secondPoint[2] = z;

    emit changed();
}

vtkProp* DrawerLine::getAsVtkProp()
{
    if (!m_vtkPropAssembly)
    {
        m_vtkPropAssembly = vtkPropAssembly::New();

        // We create the m_vtkActor pipeline
        m_vtkActor = vtkActor2D::New();
        m_vtkBackgroundActor = vtkActor2D::New();
        m_vtkLineSource = vtkLineSource::New();
        m_vtkMapper = vtkPolyDataMapper2D::New();

        // We assign the points to the line
        m_vtkLineSource->SetPoint1(m_firstPoint);
        m_vtkLineSource->SetPoint2(m_secondPoint);

        m_vtkActor->SetMapper(m_vtkMapper);
        m_vtkBackgroundActor->SetMapper(m_vtkMapper);
        m_vtkMapper->SetInputConnection(m_vtkLineSource->GetOutputPort());

        // We give it the attributes
        updateVtkActorProperties();

        m_vtkPropAssembly->AddPart(m_vtkBackgroundActor);
        m_vtkPropAssembly->AddPart(m_vtkActor);
    }
    return m_vtkPropAssembly;
}

double* DrawerLine::getFirstPoint()
{
    return m_firstPoint;
}

double* DrawerLine::getSecondPoint()
{
    return m_secondPoint;
}

void DrawerLine::update()
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

void DrawerLine::updateVtkProp()
{
    if (m_vtkPropAssembly)
    {
        // We assign the points to the line
        m_vtkLineSource->SetPoint1(m_firstPoint);
        m_vtkLineSource->SetPoint2(m_secondPoint);
        updateVtkActorProperties();
        this->setModified(false);
    }
    else
    {
        DEBUG_LOG("The line cannot be updated because it has not yet been created!");
    }
}

void DrawerLine::updateVtkActorProperties()
{
    vtkProperty2D *properties = m_vtkActor->GetProperty();
    vtkProperty2D *propertiesBackground = m_vtkBackgroundActor->GetProperty();

    //Coordinate system
    m_vtkMapper->SetTransformCoordinate(this->getVtkCoordinateObject());

    // Line style
    properties->SetLineStipplePattern(m_linePattern);
    propertiesBackground->SetLineStipplePattern(m_linePattern);

    // We assign line thickness
    properties->SetLineWidth(m_lineWidth);
    propertiesBackground->SetLineWidth(m_lineWidth + 2);

    // We assign opacity of the line
    properties->SetOpacity(m_opacity);
    propertiesBackground->SetOpacity(m_opacity);

    //Let's look at the visibility of the m_vtkActor
    m_vtkActor->SetVisibility(this->isVisible());
    m_vtkBackgroundActor->SetVisibility(this->isVisible());

    // Assignem color
    QColor color = this->getColor();
    properties->SetColor(color.redF(), color.greenF(), color.blueF());
    propertiesBackground->SetColor(0.0, 0.0, 0.0);
}

double DrawerLine::getDistanceToPoint(double *point3D, double closestPoint[3])
{
    return MathTools::getPointToFiniteLineDistance(point3D, m_firstPoint, m_secondPoint, closestPoint);
}

void DrawerLine::getBounds(double bounds[6])
{
    for (int i = 0; i < 3; i++)
    {
        if (m_firstPoint[i] < m_secondPoint[i])
        {
            bounds[i * 2] = m_firstPoint[i];
            bounds[i * 2 + 1] = m_secondPoint[i];
        }
        else
        {
            bounds[i * 2] = m_secondPoint[i];
            bounds[i * 2 + 1] = m_firstPoint[i];
        }
    }
}

}
