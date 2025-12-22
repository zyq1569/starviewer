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

#ifndef UDGDRAWERPOLYLINE_H
#define UDGDRAWERPOLYLINE_H

#include "drawerprimitive.h"
#include <QVector>

class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPropAssembly;

namespace udg {

/**
    Polyline primitive for the Drawer
*/
class DrawerPolyline : public DrawerPrimitive {
    Q_OBJECT
public:
    DrawerPolyline(QObject *parent = 0);
    ~DrawerPolyline();

    /// We add a point to the polyline.
    /// @param point [] Point that defines the point
    void addPoint(double point[3]);

    /// Assign the value of the i-th point of the polyline. In this case it is equivalent to
    /// update the value of this indicated point. If 'i' is out of range,
    /// the given point is added at the end, as if we were doing addPoint (point)
    /// @param and index of the point we want to modify
    /// @param point [] point coordinates
    void setPoint(int i, double point[3]);

    /// Remove the i-th point from the polyline. If 'i' is out of range,
    /// no point in the polyline will be deleted.
    /// @param and index of the point we want to modify
    void removePoint(int i);

    /// Returns the polyline as the VTK representation object
    vtkProp* getAsVtkProp();

    /// Returns the number of points that the polyline has
    int getNumberOfPoints();

    /// Returns the i-th point of the polyline in case it exists.
    double* getPoint(int position);

    /// Delete all points in the polyline
    void deleteAllPoints();

    /// Calculates the distance it has from the past point per parameter
    double getDistanceToPoint(double *point3D, double closestPoint[3]);

    void getBounds(double bounds[6]);

    ///Returns the polyline dot list
    QList<QVector<double> > getPointsList();

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    /// Internal method for moving from the list of points to the relevant vtk structure
    void buildVtkPoints();

    /// Internal method for transforming the properties of the primitive to properties of vtk
    void updateVtkActorProperties();

private:
    ///List of polyline points
    QList<QVector<double> > m_pointsList;

    /// Vtk structures, to build the polyline
    vtkPolyData *m_vtkPolydata;
    vtkPoints *m_vtkPoints;
    vtkCellArray *m_vtkCellArray;
    vtkActor2D *m_vtkActor;
    vtkActor2D *m_vtkBackgroundActor;
    vtkPolyDataMapper2D *m_vtkMapper;

    /// vtkProp to group the actor and the background to simulate the outline
    vtkPropAssembly *m_vtkPropAssembly;
};

}

#endif
