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

#include "ellipticalroitool.h"
#include "q2dviewer.h"
#include "logging.h"
#include "series.h"
#include "drawer.h"
#include "drawerpolygon.h"
#include "drawertext.h"
#include "mathtools.h"

#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>

namespace udg {

EllipticalROITool::EllipticalROITool(QViewer *viewer, QObject *parent)
    : ROITool(viewer, parent), m_state(Ready)
{
    m_toolName = "EllipticalROITool";

    connect(m_2DViewer, SIGNAL(volumeChanged(Volume*)), SLOT(initialize()));
}

EllipticalROITool::~EllipticalROITool()
{
    deleteTemporalRepresentation();
}

void EllipticalROITool::deleteTemporalRepresentation()
{
    if (!m_roiPolygon.isNull() && m_state == FirstPointFixed)
    {
        m_roiPolygon->decreaseReferenceCount();
        delete m_roiPolygon;
        m_2DViewer->render();
    }

    m_roiPolygon = 0;
    m_state = Ready;
}

void EllipticalROITool::handleEvent(long unsigned eventID)
{
    if (!m_2DViewer->hasInput())
    {
        return;
    }

    switch (eventID)
    {
    case vtkCommand::LeftButtonPressEvent:
        handlePointAddition();
        break;

    case vtkCommand::MouseMoveEvent:
        simulateEllipse();
        break;

    case vtkCommand::LeftButtonReleaseEvent:
        closeForm();
        break;
    case vtkCommand::KeyPressEvent:
        int keyCode = m_2DViewer->getInteractor()->GetKeyCode();
        if (keyCode == 27) // ESC
        {
            deleteTemporalRepresentation();
        }
        break;
    }
}

void EllipticalROITool::setTextPosition(DrawerText *text)
{
    double bounds[6];
    m_roiPolygon->getBounds(bounds);

    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);

    double attachmentPoint[3];
    attachmentPoint[xIndex] = (bounds[xIndex * 2] + bounds[xIndex * 2 + 1]) / 2.0;
    attachmentPoint[yIndex] = m_secondPoint[yIndex];
    attachmentPoint[zIndex] = m_secondPoint[zIndex];

    const double Padding = 5.0;
    double paddingY = 0.0;

    if (m_secondPoint[yIndex] >= bounds[yIndex * 2 + 1])
    {
        if (m_2DViewer->getView() == OrthogonalPlane::XYPlane)
        {
            paddingY = -Padding;
            text->setVerticalJustification("Top");
        }
        else
        {
            paddingY = Padding;
            text->setVerticalJustification("Bottom");
        }
    }
    else
    {
        if (m_2DViewer->getView() == OrthogonalPlane::XYPlane)
        {
            paddingY = Padding;
            text->setVerticalJustification("Bottom");
        }
        else
        {
            paddingY = -Padding;
            text->setVerticalJustification("Top");
        }
    }

    double attachmentPointInDisplay[3];
    // We pass attachmentPoint to display coordinates
    m_2DViewer->computeWorldToDisplay(attachmentPoint[0], attachmentPoint[1], attachmentPoint[2], attachmentPointInDisplay);
    // We apply padding and return to world coordinates
    m_2DViewer->computeDisplayToWorld(attachmentPointInDisplay[0], attachmentPointInDisplay[1] + paddingY, attachmentPointInDisplay[2], attachmentPoint);

    text->setAttachmentPoint(attachmentPoint);
}

void EllipticalROITool::handlePointAddition()
{
    if (m_state == Ready)
    {
        m_2DViewer->getEventWorldCoordinate(m_firstPoint);
        m_2DViewer->putCoordinateInCurrentImageBounds(m_firstPoint);

        memcpy(m_secondPoint, m_firstPoint, sizeof(double) * 3);

        m_state = FirstPointFixed;
    }
}

void EllipticalROITool::simulateEllipse()
{
    if (m_state == FirstPointFixed)
    {
        // We get the second point
        m_2DViewer->getEventWorldCoordinate(m_secondPoint);
        m_2DViewer->putCoordinateInCurrentImageBounds(m_secondPoint);

        // If we hadn’t created the polygon yet, we do
        if (!m_roiPolygon)
        {
            m_roiPolygon = new DrawerPolygon;
            //This prevents the primitive from being deleted during editing by external events
            m_roiPolygon->increaseReferenceCount();
            m_2DViewer->getDrawer()->draw(m_roiPolygon);
        }

        // We update the form and render
        updatePolygonPoints();
        m_2DViewer->render();
    }
}

void EllipticalROITool::computeEllipseCentre(double centre[3])
{
    for (int i = 0; i < 3; ++i)
    {
        centre[i] = m_firstPoint[i] + (m_secondPoint[i] - m_firstPoint[i]) * 0.5;
    }
}

void EllipticalROITool::updatePolygonPoints()
{
    double centre[3];
    computeEllipseCentre(centre);

    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);

    //Algorithm for calculating the ellipse, extracted from http://en.wikipedia.org/wiki/Ellipse#Elipses_in_computer_graphics
    double xRadius = (m_secondPoint[xIndex] - m_firstPoint[xIndex]) * 0.5;
    double yRadius = (m_secondPoint[yIndex] - m_firstPoint[yIndex]) * 0.5;
    double depthValue = centre[zIndex];

    double beta = MathTools::degreesToRadians(360);
    double sinusBeta = sin(beta);
    double cosinusBeta = cos(beta);

    const int polygonPoints = 50;
    double alpha = 0.0;
    int vertixIndex = 0;
    for (double i = 0; i < 360.0; i += 360.0 / polygonPoints)
    {
        alpha = MathTools::degreesToRadians(i);
        double sinusAlpha = sin(alpha);
        double cosinusAlpha = cos(alpha);

        double polygonPoint[3];

        polygonPoint[xIndex] = centre[xIndex] + (xRadius * cosinusAlpha * cosinusBeta - yRadius * sinusAlpha * sinusBeta);
        polygonPoint[yIndex] = centre[yIndex] + (xRadius * cosinusAlpha * sinusBeta + yRadius * sinusAlpha * cosinusBeta);
        polygonPoint[zIndex] = depthValue;

        m_roiPolygon->setVertix(vertixIndex++, polygonPoint);
    }

    m_roiPolygon->update();
}

void EllipticalROITool::closeForm()
{
    /// It is necessary to check if there is an object created as
    /// it could not have been created if a double click had been made,
    /// for example, since it would not have gone through the mouse move event,
    ///  which is when the primitive is created.
    if (m_roiPolygon)
    {
        equalizeDepth();
        printData();
        // We release the primitive so that it can be erased
        m_roiPolygon->decreaseReferenceCount();
        // We paint the primitive in the corresponding place
        m_2DViewer->getDrawer()->erasePrimitive(m_roiPolygon);
        m_2DViewer->getDrawer()->draw(m_roiPolygon, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
        //We initialize the pointer to 0
        m_roiPolygon = 0;
    }

    m_state = Ready;
}

void EllipticalROITool::initialize()
{
    // We release the primitives so that they can be erased
    if (!m_roiPolygon.isNull())
    {
        m_roiPolygon->decreaseReferenceCount();
        delete m_roiPolygon;
        m_2DViewer->render();
    }

    m_roiPolygon = 0;
    m_state = Ready;
}

void EllipticalROITool::equalizeDepth()
{
    int zIndex = m_2DViewer->getView().getZIndex();
    double z = m_2DViewer->getCurrentDisplayedImageDepth();
    m_firstPoint[zIndex] = z;
    m_secondPoint[zIndex] = z;
    updatePolygonPoints();
}

}
