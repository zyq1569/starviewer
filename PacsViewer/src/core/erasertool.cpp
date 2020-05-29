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

#include "erasertool.h"
#include "q2dviewer.h"
#include "drawer.h"
#include "drawerpolygon.h"
#include "mathtools.h"
// Vtk
#include <vtkCommand.h>

namespace udg {

EraserTool::EraserTool(QViewer *viewer, QObject *parent)
 : Tool(viewer, parent), m_state(None), m_polygon(0)
{
    m_toolName = "EraserTool";
    m_hasSharedData = false;

    m_2DViewer = Q2DViewer::castFromQViewer(viewer);

    connect(m_2DViewer, SIGNAL(volumeChanged(Volume*)), SLOT(reset()));
}

EraserTool::~EraserTool()
{
    if (m_polygon)
    {
        // Així alliberem la primitiva perquè pugui ser esborrada
        m_polygon->decreaseReferenceCount();
        delete m_polygon;
    }
}

void EraserTool::handleEvent(unsigned long eventID)
{
    switch (eventID)
    {
        case vtkCommand::LeftButtonPressEvent:
            startEraserAction();
            break;

        case vtkCommand::MouseMoveEvent:
            drawAreaOfErasure();
            break;

        case vtkCommand::LeftButtonReleaseEvent:
            erasePrimitive();
            reset();
            m_2DViewer->render();
            break;

        default:
            break;
    }
}

void EraserTool::startEraserAction()
{
    m_2DViewer->getEventWorldCoordinate(m_startPoint);
    // A l'agafar el primer punt inicialitzem l'start i l'end point per igual
    // simplement per què així és més segur que no tenir un valor arbitrari a endPoint
    m_endPoint[0] = m_startPoint[0];
    m_endPoint[1] = m_startPoint[1];
    m_endPoint[2] = m_startPoint[2];

    m_state = StartClick;
}

void EraserTool::drawAreaOfErasure()
{
    if (m_state == StartClick)
    {
        double p2[3], p3[3];
        int xIndex, yIndex, zIndex;

        m_2DViewer->getEventWorldCoordinate(m_endPoint);
        m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);

        // Calculem el segon punt i el tercer
        p2[xIndex] = m_endPoint[xIndex];
        p2[yIndex] = m_startPoint[yIndex];
        p2[zIndex] = m_2DViewer->getCurrentSlice();

        p3[xIndex] = m_startPoint[xIndex];
        p3[yIndex] = m_endPoint[yIndex];
        p3[zIndex] = m_2DViewer->getCurrentSlice();

        if (!m_polygon)
        {
            m_polygon = new DrawerPolygon;
            // Així evitem que durant l'edició la primitiva pugui ser esborrada per events externs
            m_polygon->increaseReferenceCount();
            m_polygon->addVertix(p2);
            m_polygon->addVertix(m_endPoint);
            m_polygon->addVertix(p3);
            m_polygon->addVertix(m_startPoint);
            m_2DViewer->getDrawer()->draw(m_polygon);
        }
        else
        {
            // Assignem els punts del polígon
            m_polygon->setVertix(0, p2);
            m_polygon->setVertix(1, m_endPoint);
            m_polygon->setVertix(2, p3);
            m_polygon->setVertix(3, m_startPoint);
            // Actualitzem els atributs de la polilinia
            m_polygon->update();
            m_2DViewer->render();
        }
    }
}

void EraserTool::erasePrimitive()
{
    if (!m_polygon)
    {
        DrawerPrimitive *primitiveToErase = getErasablePrimitive(m_startPoint, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
        if (primitiveToErase)
        {
            m_2DViewer->getDrawer()->erasePrimitive(primitiveToErase);
            m_2DViewer->render();
        }
    }
    else
    {
        double bounds[6];
        m_polygon->getBounds(bounds);
        m_2DViewer->getDrawer()->erasePrimitivesInsideBounds(bounds, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
    }
}

DrawerPrimitive* EraserTool::getErasablePrimitive(double point[3], const OrthogonalPlane &view, int slice)
{
    double closestPoint[3];
    DrawerPrimitive *nearestPrimitive = m_2DViewer->getDrawer()->getNearestErasablePrimitiveToPoint(point, view, slice, closestPoint);

    if (nearestPrimitive)
    {
        double displayPoint[3];
        m_2DViewer->computeWorldToDisplay(point[0], point[1], point[2], displayPoint);

        double closestDisplayPoint[3];
        m_2DViewer->computeWorldToDisplay(closestPoint[0], closestPoint[1], closestPoint[2], closestDisplayPoint);

        double displayDistance = MathTools::getDistance3D(displayPoint, closestDisplayPoint);
        // Si la distància entre els punts no està dins d'un llindar determinat, no considerem que la primitiva es pugui esborrar
        double proximityThreshold = 5.0;
        if (displayDistance > proximityThreshold)
        {
            nearestPrimitive = 0;
        }
    }

    return nearestPrimitive;
}

void EraserTool::reset()
{
    if (m_polygon)
    {
        // Així alliberem la primitiva perquè pugui ser esborrada
        m_polygon->decreaseReferenceCount();
        delete m_polygon;
        m_polygon = NULL;
    }

    m_state = None;
}

}
