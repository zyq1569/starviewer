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

#include "drawercrosshair.h"

#include "q2dviewer.h"
#include "logging.h"
#include "drawerline.h"
#include "mathtools.h"
// Vtk
#include <vtkActor2D.h>
#include <vtkPropAssembly.h>

namespace udg {

DrawerCrossHair::DrawerCrossHair(QObject *parent)
: DrawerPrimitive(parent)
{
    m_vtkPropAssembly = NULL;
    m_lineUp = new DrawerLine();
    m_lineDown = new DrawerLine();
    m_lineLeft = new DrawerLine();
    m_lineRight = new DrawerLine();
    m_lineFront = new DrawerLine();
    m_lineBack = new DrawerLine();
}

DrawerCrossHair::~DrawerCrossHair()
{
    emit dying(this);

    if (m_vtkPropAssembly)
    {
        m_vtkPropAssembly->Delete();
    }
}

void DrawerCrossHair::setCentrePoint(double x, double y, double z)
{
    m_centrePoint[0] = x;
    m_centrePoint[1] = y;
    m_centrePoint[2] = z;

    // Assignem els punts a la línia 1
    m_lineUp->setFirstPoint(m_centrePoint[0], m_centrePoint[1] - 6, m_centrePoint[2]);
    m_lineUp->setSecondPoint(m_centrePoint[0], m_centrePoint[1] - 1, m_centrePoint[2]);

    // Assignem els punts a la línia 2
    m_lineDown->setFirstPoint(m_centrePoint[0], m_centrePoint[1] + 6, m_centrePoint[2]);
    m_lineDown->setSecondPoint(m_centrePoint[0], m_centrePoint[1] + 1, m_centrePoint[2]);

    // Assignem els punts a la línia 3
    m_lineLeft->setFirstPoint(m_centrePoint[0] - 6, m_centrePoint[1], m_centrePoint[2]);
    m_lineLeft->setSecondPoint(m_centrePoint[0] - 1, m_centrePoint[1], m_centrePoint[2]);

    // Assignem els punts a la línia 4
    m_lineRight->setFirstPoint(m_centrePoint[0] + 6, m_centrePoint[1], m_centrePoint[2]);
    m_lineRight->setSecondPoint(m_centrePoint[0] + 1, m_centrePoint[1], m_centrePoint[2]);

    // Assignem els punts a la línia 5
    m_lineBack->setFirstPoint(m_centrePoint[0], m_centrePoint[1], m_centrePoint[2] - 6);
    m_lineBack->setSecondPoint(m_centrePoint[0], m_centrePoint[1], m_centrePoint[2] - 1);

    // Assignem els punts a la línia 6
    m_lineFront->setFirstPoint(m_centrePoint[0], m_centrePoint[1], m_centrePoint[2] + 6);
    m_lineFront->setSecondPoint(m_centrePoint[0], m_centrePoint[1], m_centrePoint[2] + 1);

    emit changed();
}

vtkPropAssembly* DrawerCrossHair::getAsVtkPropAssembly()
{
    if (!m_vtkPropAssembly)
    {
        m_vtkPropAssembly = vtkPropAssembly::New();

        // Li donem els atributs
        updateVtkActorProperties();

        m_vtkPropAssembly->AddPart(m_lineUp->getAsVtkProp());
        m_vtkPropAssembly->AddPart(m_lineDown->getAsVtkProp());
        m_vtkPropAssembly->AddPart(m_lineLeft->getAsVtkProp());
        m_vtkPropAssembly->AddPart(m_lineRight->getAsVtkProp());
        m_vtkPropAssembly->AddPart(m_lineBack->getAsVtkProp());
        m_vtkPropAssembly->AddPart(m_lineFront->getAsVtkProp());
    }
    return m_vtkPropAssembly;
}

vtkProp* DrawerCrossHair::getAsVtkProp()
{
    return (vtkProp*)getAsVtkPropAssembly();
}

void DrawerCrossHair::update()
{
    m_lineUp->update();
    m_lineDown->update();
    m_lineLeft->update();
    m_lineRight->update();
    m_lineBack->update();
    m_lineFront->update();
}

void DrawerCrossHair::updateVtkProp()
{
    if (m_vtkPropAssembly)
    {
        m_lineUp->update();
        m_lineDown->update();
        m_lineLeft->update();
        m_lineRight->update();
        m_lineBack->update();
        m_lineFront->update();

        this->setModified(false);
    }
    else
    {
        DEBUG_LOG("No es pot actualitzar la línia, ja que encara no s'ha creat!");
    }
}

void DrawerCrossHair::updateVtkActorProperties()
{
    m_lineUp->setLineWidth(2.0);
    m_lineDown->setLineWidth(2.0);
    m_lineLeft->setLineWidth(2.0);
    m_lineRight->setLineWidth(2.0);
    m_lineBack->setLineWidth(2.0);
    m_lineFront->setLineWidth(2.0);
}

double DrawerCrossHair::getDistanceToPoint(double *point3D, double closestPoint[3])
{
    closestPoint[0] = m_centrePoint[0];
    closestPoint[1] = m_centrePoint[1];
    closestPoint[2] = m_centrePoint[2];
    return MathTools::getDistance3D(m_centrePoint, point3D);
}

void DrawerCrossHair::getBounds(double bounds[6])
{
    bounds[0] = m_lineLeft->getFirstPoint()[0];
    bounds[1] = m_lineRight->getFirstPoint()[0];
    bounds[2] = m_lineUp->getFirstPoint()[1];
    bounds[3] = m_lineDown->getFirstPoint()[1];
    bounds[4] = m_lineBack->getFirstPoint()[2];
    bounds[5] = m_lineFront->getFirstPoint()[2];
}

void DrawerCrossHair::setVisibility(bool visible)
{
    m_isVisible = visible;
    m_lineUp->setVisibility(visible);
    m_lineDown->setVisibility(visible);
    m_lineLeft->setVisibility(visible);
    m_lineRight->setVisibility(visible);
    m_lineBack->setVisibility(visible);
    m_lineFront->setVisibility(visible);
    emit changed();
}

}
