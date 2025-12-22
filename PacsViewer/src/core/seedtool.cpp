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

#include "seedtool.h"
#include "seedtooldata.h"
#include "q2dviewer.h"
#include "drawerpoint.h"
#include "drawer.h"
// Vtk's
#include <vtkCommand.h>
// Qt's
#include <QVector>

namespace udg {

SeedTool::SeedTool(QViewer *viewer, QObject *parent)
    : Tool(viewer, parent)
{
    m_toolName = "SeedTool";
    m_hasSharedData = false;
    m_hasPersistentData = true;

    m_myData = new SeedToolData;

    m_2DViewer = Q2DViewer::castFromQViewer(viewer);

    m_state = None;
    m_drawn = false;
    m_myData->setVolume(m_2DViewer->getMainInput());
}

SeedTool::~SeedTool()
{
}

void SeedTool::handleEvent(unsigned long eventID)
{
    switch (eventID)
    {
    case vtkCommand::LeftButtonPressEvent:
        setSeed();
        break;

    case vtkCommand::MouseMoveEvent:
        doSeeding();
        break;

    case vtkCommand::LeftButtonReleaseEvent:
        endSeeding();
        break;

    default:
        break;
    }
}

void SeedTool::setToolData(ToolData *data)
{
    // We make this comparison because sometimes the date we already have on m_myData happens to us
    if (m_myData != data)
    {
        //We create the data again
        m_toolData = data;
        m_myData = qobject_cast<SeedToolData*>(data);
        //If we have data it means that we have already painted the seed before if the volume has changed
        if (m_2DViewer->getMainInput() != m_myData->getVolume())
        {
            // Input change
            m_drawn = false;
            m_myData->setVolume(m_2DViewer->getMainInput());
            // If we have data it means that the viewer has deleted the point so we set it to 0 to be recreated
            m_myData->setPoint(NULL);
        }
        else
        {
            // Tool change
            m_drawn = true;
            m_2DViewer->getDrawer()->erasePrimitive(m_myData->getPoint());
            m_myData->setPoint(NULL);
            m_myData->setSeedPosition(m_myData->getSeedPosition());
            m_2DViewer->getDrawer()->draw(m_myData->getPoint(), m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
        }
    }
}

void SeedTool::setSeed()
{
    Q_ASSERT(m_2DViewer);

    m_state = Seeding;
    updateSeedPosition();
}

void SeedTool::doSeeding()
{
    Q_ASSERT(m_2DViewer);

    if (m_state == Seeding)
    {
        updateSeedPosition();
    }
}

void SeedTool::endSeeding()
{
    m_state = None;
}

void SeedTool::updateSeedPosition()
{
    Q_ASSERT(m_2DViewer);

    double xyz[3];
    if (m_2DViewer->getCurrentCursorImageCoordinate(xyz))
    {
        QVector<double> seedPosition(3);
        seedPosition[0] = xyz[0];
        seedPosition[1] = xyz[1];
        seedPosition[2] = xyz[2];

        //DEBUG_LOG(QString("Seed Pos: [%1,%2,%3]").arg(seedPosition[0]).arg(seedPosition[1]).arg(seedPosition[2]));

        m_myData->setSeedPosition(seedPosition);
        // TODO I'm trying to make it work for now, but it has to be fixed
        // Only "seedChanged ()" should be issued and that's it
        m_2DViewer->setSeedPosition(xyz);
        emit seedChanged(seedPosition[0], seedPosition[1], seedPosition[2]);

        if (!m_drawn)
        {
            m_2DViewer->getDrawer()->draw(m_myData->getPoint(), m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
            m_drawn = true;
        }
        else
        {
            m_myData->getPoint()->update();
            m_2DViewer->render();
        }

        //m_2DViewer->getDrawer()->draw(m_myData->getPoint(), m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
    }
}

void SeedTool::setSeed(QVector<double> seedPosition, int slice)
{
    Q_ASSERT(m_2DViewer);

    m_myData->setSeedPosition(seedPosition);
    double xyz[3];
    xyz[0] = seedPosition[0];
    xyz[1] = seedPosition[1];
    xyz[2] = seedPosition[2];
    m_2DViewer->setSeedPosition(xyz);
    emit seedChanged(seedPosition[0], seedPosition[1], seedPosition[2]);
    //DEBUG_LOG(QString("Seed Pos: [%1,%2,%3], slice = %4").arg(seedPosition[0]).arg(seedPosition[1]).arg(seedPosition[2]).arg(slice));
    m_2DViewer->getDrawer()->draw(m_myData->getPoint(), m_2DViewer->getView(), slice);
}

ToolData *SeedTool::getToolData() const
{
    return m_myData;
}

}
