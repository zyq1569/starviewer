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
    Primitiva de línia per al Drawer
  */
class DrawerLine : public DrawerPrimitive {
Q_OBJECT
public:
    DrawerLine(QObject *parent = 0);
    ~DrawerLine();

    /// Afegim el primer punt de la línia
    void setFirstPoint(double point[3]);
    void setFirstPoint(double x, double y, double z);

    /// Afegim el segon punt de la línia
    void setSecondPoint(double point[3]);
    void setSecondPoint(double x, double y, double z);

    vtkProp* getAsVtkProp();

    /// Retorna el primer punt de la línia
    double* getFirstPoint();

    /// Retorna el segon punt de la línia
    double* getSecondPoint();

    /// Calcula la distància que té respecte al punt passat per paràmetre
    double getDistanceToPoint(double *point3D, double closestPoint[3]);

    void getBounds(double bounds[6]);

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    /// Mètode intern per transformar les propietats de la primitiva a propietats de vtk
    void updateVtkActorProperties();

private:
    /// Primer punt de la línia
    double m_firstPoint[3];

    /// Segon punt de la línia
    double m_secondPoint[3];

    /// Estructures de vtk, per construir la línia
    vtkLineSource *m_vtkLineSource;
    vtkActor2D *m_vtkActor;
    vtkActor2D *m_vtkBackgroundActor;
    vtkPolyDataMapper2D *m_vtkMapper;

    /// vtkProp per agrupar l'actor i el background per simular el contorn
    vtkPropAssembly *m_vtkPropAssembly;
};

}

#endif
