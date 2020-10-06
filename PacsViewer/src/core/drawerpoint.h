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

#ifndef UDGDRAWERPOINT_H
#define UDGDRAWERPOINT_H

#include "drawerprimitive.h"
// Qt's
#include <QVector>

class vtkSphereSource;
class vtkActor;
class vtkPolyDataMapper;

namespace udg {

/**
   Knitted primitive
*/
class DrawerPoint : public DrawerPrimitive {
    Q_OBJECT
public:
    DrawerPoint(QObject *parent = 0);
    ~DrawerPoint();

    /// We assign the point
    void setPosition(double point[3]);
    void setPosition(QVector<double> point);

    ///We assign the radius to the circle that defines the point
    void setRadius(double radius);

    vtkProp* getAsVtkProp();

    void getBounds(double bounds[6]);

    /// Calculates the distance it has from the past point per parameter
    double getDistanceToPoint(double *point3D, double closestPoint[3]);

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    ///Internal method for transforming the properties of the primitive to properties of vtk
    void updateVtkActorProperties();

private:
    ///Point position
    double m_position[3];

    /// Radius of the circle with which we draw the point
    double m_pointRadius;

    /// Actor drawing the seed
    vtkActor *m_pointActor;

    ///Sphere representing point of the seed
    vtkSphereSource *m_pointSphere;

    ///Point mapper
    vtkPolyDataMapper *m_pointMapper;
};

}

#endif
