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

#ifndef UDGDRAWERLINE_H
#define UDGDRAWERLINE_H

#include "drawerprimitive.h"

class vtkLineSource;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPropAssembly;

namespace udg {

/**
    Line Primitive for Drawer
*/
class DrawerLine : public DrawerPrimitive {
    Q_OBJECT
public:
    DrawerLine(QObject *parent = 0);
    ~DrawerLine();

    /// We add the first point of the line
    void setFirstPoint(double point[3]);
    void setFirstPoint(double x, double y, double z);

    ///We add the second point of the line
    void setSecondPoint(double point[3]);
    void setSecondPoint(double x, double y, double z);

    vtkProp* getAsVtkProp();

    /// Returns the first point of the line
    double* getFirstPoint();

    /// Returns the second point of the line
    double* getSecondPoint();

    ///Calculates the distance it has from the past point per parameter
    double getDistanceToPoint(double *point3D, double closestPoint[3]);

    void getBounds(double bounds[6]);

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    /// Internal method for transforming the properties of the primitive to properties of vtk
    void updateVtkActorProperties();

private:
    /// First point of the line
    double m_firstPoint[3];

    ///Second point of the line
    double m_secondPoint[3];

    /// Vtk structures, to build the line
    vtkLineSource *m_vtkLineSource;
    vtkActor2D *m_vtkActor;
    vtkActor2D *m_vtkBackgroundActor;
    vtkPolyDataMapper2D *m_vtkMapper;

    /// vtkProp to group the actor and the background to simulate the outline
    vtkPropAssembly *m_vtkPropAssembly;
};

}

#endif
