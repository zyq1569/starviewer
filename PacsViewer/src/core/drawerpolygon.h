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

#ifndef UDGDRAWERPOLYGON_H
#define UDGDRAWERPOLYGON_H

#include "drawerprimitive.h"
#include "q2dviewer.h"
#include "line3d.h"

class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPropAssembly;

namespace udg {

/**
   Polygon primitive for the Drawer
*/
class DrawerPolygon : public DrawerPrimitive {
    Q_OBJECT
public:
    DrawerPolygon(QObject *parent = 0);
    ~DrawerPolygon();

    /// We add a vertex to the polygon.
    /// @param point [] Point that defines the vertex of the polygon
    void addVertix(double point[3]);
    void addVertix(double x, double y, double z);

    /// Empty the list of vertices
    void removeVertices();

    /// Assigns the value of the i-th vertex of the polygon. In this case it is equivalent to
    /// update the value of this indicated point. If 'i' is out of range,
    /// add the given vertex at the end, as if we were addVertix (point)
    /// @param and index of the vertex we want to modify
    /// @param point [] vertex coordinates
    void setVertix(int i, double point[3]);
    void setVertix(int i, double x, double y, double z);

    /// Returns the i-th vertex of the polygon.
    /// If the index is out of range it will return an uninitialized array
    const double* getVertix(int i) const;

    /// Returns the segments of the polygon
    QList<Line3D> getSegments();
    
    vtkProp* getAsVtkProp();

    /// It returns the number of points that the polygon has
    int getNumberOfPoints() const;

    /// Calculates the distance it has from the past point per parameter
    double getDistanceToPoint(double *point3D, double closestPoint[3]);

    void getBounds(double [6]);

    /// Returns the 2D coordinate indices corresponding to the plane where the polygon is lying on.
    /// If the plane could not be determined, -1, -1 will be returned
    void get2DPlaneIndices(int &x, int &y) const;

    /// Returns the vtkPolyData that represents this polygon.
    vtkPolyData* getVtkPolyData() const;

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:

    /// Internal method to build the VTK pipeline.
    void buildVtkPipeline();
    /// Internal method for moving from the list of points to the relevant vtk structure
    void buildVtkPoints();
    /// Internal method for transforming the properties of the primitive to properties of vtk
    void updateVtkActorProperties();

private:
    /// List of points of the polygon
    QList<QVector<double> > m_pointsList;
    /// Indicates whether points have changed since
    /// since the last time the VTK rendering was updated.
    bool m_pointsChanged;

    /// Vtk structures, to build the polygon
    vtkPolyData *m_vtkPolyData;
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
