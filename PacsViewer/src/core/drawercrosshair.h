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

#ifndef UDGDRAWERCROSSHAIR_H
#define UDGDRAWERCROSSHAIR_H

#include "drawerprimitive.h"
#include "drawerline.h"

// Forward declarations
class vtkPropAssembly;

namespace udg {

/**
   Primitive that draws a "Crosshair", that is, a cross to place a point.
*/
class DrawerCrossHair : public DrawerPrimitive {
    Q_OBJECT
public:
    DrawerCrossHair(QObject *parent = 0);
    ~DrawerCrossHair();

    /// We add the first point of the line
    void setCentrePoint(double x, double y, double z);

    vtkPropAssembly* getAsVtkPropAssembly();

    vtkProp* getAsVtkProp();

    double getDistanceToPoint(double *point3D, double closestPoint[3]);

    void getBounds(double bounds[6]);

    void setVisibility(bool visible);

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    /// Internal method for transforming the properties of the primitive to properties of vtk
    void updateVtkActorProperties();

private:
    /// Center of the crosshair.
    double m_centrePoint[3];

    ///Lines to build the crosshair
    DrawerLine *m_lineUp;
    DrawerLine *m_lineDown;
    DrawerLine *m_lineLeft;
    DrawerLine *m_lineRight;
    DrawerLine *m_lineFront;
    DrawerLine *m_lineBack;

    vtkPropAssembly *m_vtkPropAssembly;
};

}

#endif
