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

#include "genericdistancetool.h"

#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>

#include "q2dviewer.h"
#include "drawer.h"
#include "drawerline.h"
#include "drawertext.h"
#include "distancemeasurecomputer.h"

namespace udg {

GenericDistanceTool::GenericDistanceTool(QViewer *viewer, QObject *parent)
 : MeasurementTool(viewer, parent)
{
    m_hasSharedData = false;
}

GenericDistanceTool::~GenericDistanceTool()
{
}

void GenericDistanceTool::handleEvent(long unsigned eventID)
{
    if (!m_2DViewer || !m_2DViewer->hasInput())
    {
        return;
    }
    
    switch (eventID)
    {
        case vtkCommand::LeftButtonPressEvent:
            handleLeftButtonPress();
            break;

        case vtkCommand::MouseMoveEvent:
            handleMouseMove();
            break;

        case vtkCommand::KeyPressEvent:
            handleKeyPress();
            break;
    }
}

void GenericDistanceTool::handleKeyPress()
{
    int keyCode = m_2DViewer->getInteractor()->GetKeyCode();
    if (keyCode == 27) // Esc
    {
        abortDrawing();
    }
}

MeasureComputer* GenericDistanceTool::getMeasureComputer() const
{
    return new DistanceMeasureComputer(m_distanceLine);
}

void GenericDistanceTool::drawMeasurement()
{
    DrawerText *text = new DrawerText;
    text->setText(getMeasurementString());

    placeMeasurementText(text);
}

void GenericDistanceTool::placeMeasurementText(DrawerText *text)
{
    // We place text to the left or right of the second point depending on the way line had been drawn
    int xIndex = m_2DViewer->getView().getXIndex();
    double *firstPoint = m_distanceLine->getFirstPoint();
    double *secondPoint = m_distanceLine->getSecondPoint();

    // Applying 5-pixel padding
    const double Padding = 5.0;
    double textPadding;
    if (firstPoint[xIndex] <= secondPoint[xIndex])
    {
        textPadding = Padding;
        text->setHorizontalJustification("Left");
    }
    else
    {
        textPadding = -Padding;
        text->setHorizontalJustification("Right");
    }

    double secondPointInDisplay[3];
    // Converting secondPoint to display coordinates
    m_2DViewer->computeWorldToDisplay(secondPoint[0], secondPoint[1], secondPoint[2], secondPointInDisplay);
    // Applying padding and converting back to world coordinates
    double attachmentPoint[3];
    m_2DViewer->computeDisplayToWorld(secondPointInDisplay[0] + textPadding, secondPointInDisplay[1], secondPointInDisplay[2], attachmentPoint);

    text->setAttachmentPoint(attachmentPoint);
    m_2DViewer->getDrawer()->draw(text, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
}

} // End namespace udg
