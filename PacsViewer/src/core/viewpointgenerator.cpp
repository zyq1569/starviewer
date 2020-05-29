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

#include "viewpointgenerator.h"

#include "sphereuniformpointcloudgenerator.h"
#include "vector3.h"

namespace udg {

Vector3 ViewpointGenerator::up(const Vector3 &viewpoint)
{
    double max = qMax(qAbs(viewpoint.x), qMax(qAbs(viewpoint.y), qAbs(viewpoint.z)));
    Vector3 up(qMax(max - qAbs(viewpoint.y - viewpoint.z), 0.0), qAbs(viewpoint.x) + qAbs(viewpoint.z), qAbs(viewpoint.y));
    return up.normalize();
}

ViewpointGenerator::ViewpointGenerator()
{
}

ViewpointGenerator::~ViewpointGenerator()
{
}

void ViewpointGenerator::setToUniform4(float radius)
{
    m_viewpoints.clear();

    const double UNIT = radius / sqrt(3.0);

                    // 0
    m_viewpoints << Vector3(UNIT, UNIT, UNIT)
                    // 1
                 << Vector3(-UNIT, -UNIT, UNIT)
                    // 2
                 << Vector3(-UNIT, UNIT, -UNIT)
                    // 3
                 << Vector3(UNIT, -UNIT, -UNIT);

                    // 0
    m_neighbours << (QVector<int>() << 1 << 2 << 3)
                    // 1
                 << (QVector<int>() << 0 << 2 << 3)
                    // 2
                 << (QVector<int>() << 0 << 1 << 3)
                    // 3
                 << (QVector<int>() << 0 << 1 << 2);
}

void ViewpointGenerator::setToUniform6(float radius)
{
    m_viewpoints.clear();

                    // 0
    m_viewpoints << Vector3(radius, 0.0, 0.0)
                    // 1
                 << Vector3(-radius, 0.0, 0.0)
                    // 2
                 << Vector3(0.0, radius, 0.0)
                    // 3
                 << Vector3(0.0, -radius, 0.0)
                    // 4
                 << Vector3(0.0, 0.0, radius)
                    // 5
                 << Vector3(0.0, 0.0, -radius);

                    // 0
    m_neighbours << (QVector<int>() << 2 << 3 << 4 << 5)
                    // 1
                 << (QVector<int>() << 2 << 3 << 4 << 5)
                    // 2
                 << (QVector<int>() << 0 << 1 << 4 << 5)
                    // 3
                 << (QVector<int>() << 0 << 1 << 4 << 5)
                    // 4
                 << (QVector<int>() << 0 << 1 << 2 << 3)
                    // 5
                 << (QVector<int>() << 0 << 1 << 2 << 3);
}

void ViewpointGenerator::setToUniform8(float radius)
{
    m_viewpoints.clear();

    const double UNIT = radius / sqrt(3.0);

                    // 0
    m_viewpoints << Vector3(UNIT, UNIT, UNIT)
                    // 1
                 << Vector3(UNIT, UNIT, -UNIT)
                    // 2
                 << Vector3(UNIT, -UNIT, UNIT)
                    // 3
                 << Vector3(UNIT, -UNIT, -UNIT)
                    // 4
                 << Vector3(-UNIT, UNIT, UNIT)
                    // 5
                 << Vector3(-UNIT, UNIT, -UNIT)
                    // 6
                 << Vector3(-UNIT, -UNIT, UNIT)
                    // 7
                 << Vector3(-UNIT, -UNIT, -UNIT);

                    // 0
    m_neighbours << (QVector<int>() << 1 << 2 << 4)
                    // 1
                 << (QVector<int>() << 0 << 3 << 5)
                    // 2
                 << (QVector<int>() << 0 << 3 << 6)
                    // 3
                 << (QVector<int>() << 1 << 2 << 7)
                    // 4
                 << (QVector<int>() << 0 << 5 << 6)
                    // 5
                 << (QVector<int>() << 1 << 4 << 7)
                    // 6
                 << (QVector<int>() << 2 << 4 << 7)
                    // 7
                 << (QVector<int>() << 3 << 5 << 6);
}

void ViewpointGenerator::setToUniform12(float radius)
{
    m_viewpoints.clear();

    const double PHI = (1.0 + sqrt(5.0)) / 2.0;

                    // 0
    m_viewpoints << Vector3(0.0, 1.0, PHI).normalize() * radius
                    // 1
                 << Vector3(0.0, 1.0, -PHI).normalize() * radius
                    // 2
                 << Vector3(0.0, -1.0, PHI).normalize() * radius
                    // 3
                 << Vector3(0.0, -1.0, -PHI).normalize() * radius
                    // 4
                 << Vector3(1.0, PHI, 0.0).normalize() * radius
                    // 5
                 << Vector3(1.0, -PHI, 0.0).normalize() * radius
                    // 6
                 << Vector3(-1.0, PHI, 0.0).normalize() * radius
                    // 7
                 << Vector3(-1.0, -PHI, 0.0).normalize() * radius
                    // 8
                 << Vector3(PHI, 0.0, 1.0).normalize() * radius
                    // 9
                 << Vector3(PHI, 0.0, -1.0).normalize() * radius
                    // 10
                 << Vector3(-PHI, 0.0, 1.0).normalize() * radius
                    // 11
                 << Vector3(-PHI, 0.0, -1.0).normalize() * radius;

                    // 0
    m_neighbours << (QVector<int>() << 2 << 4 << 6 << 8 << 10)
                    // 1
                 << (QVector<int>() << 3 << 4 << 6 << 9 << 11)
                    // 2
                 << (QVector<int>() << 0 << 5 << 7 << 8 << 10)
                    // 3
                 << (QVector<int>() << 1 << 5 << 7 << 9 << 11)
                    // 4
                 << (QVector<int>() << 0 << 1 << 6 << 8 <<  9)
                    // 5
                 << (QVector<int>() << 2 << 3 << 7 << 8 <<  9)
                    // 6
                 << (QVector<int>() << 0 << 1 << 4 << 0 << 11)
                    // 7
                 << (QVector<int>() << 2 << 3 << 5 << 0 << 11)
                    // 8
                 << (QVector<int>() << 0 << 2 << 4 << 5 <<  9)
                    // 9
                 << (QVector<int>() << 1 << 3 << 4 << 5 <<  8)
                    // 10
                 << (QVector<int>() << 0 << 2 << 6 << 7 << 11)
                    // 11
                 << (QVector<int>() << 1 << 3 << 6 << 7 << 10);
}

void ViewpointGenerator::setToUniform20(float radius)
{
    m_viewpoints.clear();

    const double UNIT = radius / sqrt(3.0);
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double INV_PHI = 1.0 / PHI;

                    // 0
    m_viewpoints << Vector3(UNIT, UNIT, UNIT)
                    // 1
                 << Vector3(UNIT, UNIT, -UNIT)
                    // 2
                 << Vector3(UNIT, -UNIT, UNIT)
                    // 3
                 << Vector3(UNIT, -UNIT, -UNIT)
                    // 4
                 << Vector3(-UNIT, UNIT, UNIT)
                    // 5
                 << Vector3(-UNIT, UNIT, -UNIT)
                    // 6
                 << Vector3(-UNIT, -UNIT, UNIT)
                    // 7
                 << Vector3(-UNIT, -UNIT, -UNIT)
                    // 8
             << Vector3(0.0, INV_PHI, PHI).normalize() * radius
                    // 9
                 << Vector3(0.0, INV_PHI, -PHI).normalize() * radius
                    // 10
                 << Vector3(0.0, -INV_PHI, PHI).normalize() * radius
                    // 11
                 << Vector3(0.0, -INV_PHI, -PHI).normalize() * radius
                    // 12
                 << Vector3(INV_PHI, PHI, 0.0).normalize() * radius
                    // 13
                 << Vector3(INV_PHI, -PHI, 0.0).normalize() * radius
                    // 14
                 << Vector3(-INV_PHI, PHI, 0.0).normalize() * radius
                    // 15
                 << Vector3(-INV_PHI, -PHI, 0.0).normalize() * radius
                    // 16
                 << Vector3(PHI, 0.0, INV_PHI).normalize() * radius
                    // 17
                 << Vector3(PHI, 0.0, -INV_PHI).normalize() * radius
                    // 18
                 << Vector3(-PHI, 0.0, INV_PHI).normalize() * radius
                    // 19
                 << Vector3(-PHI, 0.0, -INV_PHI).normalize() * radius;

                    // 0
    m_neighbours << (QVector<int>() <<  8 << 12 << 16)
                    // 1
                 << (QVector<int>() <<  9 << 12 << 17)
                    // 2
                 << (QVector<int>() << 10 << 13 << 16)
                    // 3
                 << (QVector<int>() << 11 << 13 << 17)
                    // 4
                 << (QVector<int>() <<  8 << 14 << 18)
                    // 5
                 << (QVector<int>() <<  9 << 14 << 19)
                    // 6
                 << (QVector<int>() << 10 << 15 << 18)
                    // 7
                 << (QVector<int>() << 11 << 15 << 19)
                    // 8
                 << (QVector<int>() <<  0 <<  4 << 10)
                    // 9
                 << (QVector<int>() <<  1 <<  5 << 11)
                    // 10
                 << (QVector<int>() <<  2 <<  6 <<  8)
                    // 11
                 << (QVector<int>() <<  3 <<  7 <<  9)
                    // 12
                 << (QVector<int>() <<  0 <<  1 << 14)
                    // 13
                 << (QVector<int>() <<  2 <<  3 << 15)
                    // 14
                 << (QVector<int>() <<  4 <<  5 << 12)
                    // 15
                 << (QVector<int>() <<  7 <<  6 << 13)
                    // 16
                 << (QVector<int>() <<  0 <<  2 << 17)
                    // 17
                 << (QVector<int>() <<  1 <<  3 << 16)
                    // 18
                 << (QVector<int>() <<  4 <<  6 << 19)
                    // 19
                 << (QVector<int>() <<  5 <<  7 << 18);
}

void ViewpointGenerator::setToQuasiUniform(unsigned char depth, float radius)
{
    SphereUniformPointCloudGenerator cloud(radius, depth);
    cloud.createPOVCloud();
    m_viewpoints = cloud.getVertices();
    m_neighbours = cloud.getNeighbours();
}

QVector<Vector3> ViewpointGenerator::viewpoints() const
{
    return m_viewpoints;
}

Vector3 ViewpointGenerator::viewpoint(int i) const
{
    return m_viewpoints.at(i);
}

QVector<int> ViewpointGenerator::neighbours(int i) const
{
    return m_neighbours.at(i);
}

}
