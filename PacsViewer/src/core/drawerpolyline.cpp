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

#include "drawerpolyline.h"
#include "logging.h"
#include "mathtools.h"
// Vtk
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkActor2D.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkPropAssembly.h>
// Qt
#include <QVector>

namespace udg {

DrawerPolyline::DrawerPolyline(QObject *parent)
    : DrawerPrimitive(parent), m_vtkPolydata(0), m_vtkPoints(0), m_vtkCellArray(0), m_vtkActor(0), m_vtkBackgroundActor(0), m_vtkMapper(0), m_vtkPropAssembly(0)
{
    m_vtkActor = NULL;
}

DrawerPolyline::~DrawerPolyline()
{
    emit dying(this);

    if (m_vtkPolydata)
    {
        m_vtkPolydata->Delete();
    }

    if (m_vtkPoints)
    {
        m_vtkPoints->Delete();
    }

    if (m_vtkCellArray)
    {
        m_vtkCellArray->Delete();
    }

    if (m_vtkActor)
    {
        m_vtkActor->Delete();
    }
    if (m_vtkBackgroundActor)
    {
        m_vtkBackgroundActor->Delete();
    }
    if (m_vtkMapper)
    {
        m_vtkMapper->Delete();
    }
}

void DrawerPolyline::addPoint(double point[3])
{
    QVector<double> array(3);

    array[0] = point[0];
    array[1] = point[1];
    array[2] = point[2];

    m_pointsList << array;
    emit changed();
}

void DrawerPolyline::setPoint(int i, double point[3])
{
    if (i >= m_pointsList.count() || i < 0)
    {
        addPoint(point);
    }
    else
    {
        QVector<double> array(3);
        array[0] = point[0];
        array[1] = point[1];
        array[2] = point[2];

        m_pointsList.removeAt(i);
        m_pointsList.insert(i, array);
        emit changed();
    }
}

double* DrawerPolyline::getPoint(int position)
{
    if (position >= m_pointsList.count())
    {
        double *array = new double[3];
        return array;
    }
    else
    {
        return m_pointsList[position].data();
    }
}

void DrawerPolyline::removePoint(int i)
{
    m_pointsList.removeAt(i);
    emit changed();
}

void DrawerPolyline::deleteAllPoints()
{
    m_pointsList.clear();
}

vtkProp* DrawerPolyline::getAsVtkProp()
{
    if (!m_vtkPropAssembly)
    {
        m_vtkPropAssembly = vtkPropAssembly::New();

        buildVtkPoints();
        // We create the m_vtkActor pipeline
        m_vtkActor = vtkActor2D::New();
        m_vtkBackgroundActor = vtkActor2D::New();
        m_vtkMapper = vtkPolyDataMapper2D::New();

        m_vtkActor->SetMapper(m_vtkMapper);
        m_vtkBackgroundActor->SetMapper(m_vtkMapper);
        m_vtkMapper->SetInputData(m_vtkPolydata);
        // We give it the attributes
        updateVtkActorProperties();

        m_vtkPropAssembly->AddPart(m_vtkBackgroundActor);
        m_vtkPropAssembly->AddPart(m_vtkActor);
    }
    return m_vtkPropAssembly;
}

void DrawerPolyline::update()
{
    switch (m_internalRepresentation)
    {
    case VTKRepresentation:
        updateVtkProp();
        break;

    case OpenGLRepresentation:
        break;
    }
}

void DrawerPolyline::updateVtkProp()
{
    if (m_vtkPropAssembly)
    {
        m_vtkPolydata->Reset();
        buildVtkPoints();
        updateVtkActorProperties();
        this->setModified(false);
    }
    else
    {
        DEBUG_LOG("Unable to update polyline as not yet created!");
    }
}

void DrawerPolyline::buildVtkPoints()
{
    if (!m_vtkPolydata)
    {
        m_vtkPolydata = vtkPolyData::New();
        m_vtkPoints = vtkPoints::New();
        m_vtkCellArray = vtkCellArray::New();
    }

    //We specify the number of vertices that the polyline has
    int numberOfVertices = m_pointsList.count();
    m_vtkCellArray->InsertNextCell(numberOfVertices);
    m_vtkPoints->SetNumberOfPoints(numberOfVertices);

    // Let's give the points
    int i = 0;
    foreach (const QVector<double> &vertix, m_pointsList)
    {
        m_vtkPoints->InsertPoint(i, vertix.data());
        m_vtkCellArray->InsertCellPoint(i);
        i++;
    }

    // We assign the points to the polydata
    m_vtkPolydata->SetPoints(m_vtkPoints);

    m_vtkPolydata->SetLines(m_vtkCellArray);
}

void DrawerPolyline::updateVtkActorProperties()
{
    // Coordinate system
    m_vtkMapper->SetTransformCoordinate(this->getVtkCoordinateObject());
    // Line style
    m_vtkActor->GetProperty()->SetLineStipplePattern(m_linePattern);
    m_vtkBackgroundActor->GetProperty()->SetLineStipplePattern(m_linePattern);
    // We assign line thickness
    m_vtkActor->GetProperty()->SetLineWidth(m_lineWidth);
    m_vtkBackgroundActor->GetProperty()->SetLineWidth(m_lineWidth + 2);
    // We assign opacity of the line
    m_vtkActor->GetProperty()->SetOpacity(m_opacity);
    m_vtkBackgroundActor->GetProperty()->SetOpacity(m_opacity);
    //Let's look at the visibility of the m_vtkActor
    m_vtkActor->SetVisibility(this->isVisible());
    m_vtkBackgroundActor->SetVisibility(this->isVisible());
    // Assignem color
    QColor color = this->getColor();
    m_vtkActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    m_vtkBackgroundActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
}

int DrawerPolyline::getNumberOfPoints()
{
    return m_pointsList.count();
}

double DrawerPolyline::getDistanceToPoint(double *point3D, double closestPoint[3])
{
    int closestEdge;
    return MathTools::getPointToClosestEdgeDistance(point3D, m_pointsList, false, closestPoint, closestEdge);
}

void DrawerPolyline::getBounds(double bounds[6])
{
    if (m_vtkPolydata)
    {
        m_vtkPolydata->GetBounds(bounds);
    }
    else
    {
        memset(bounds, 0.0, sizeof(double) * 6);
    }
}

QList<QVector<double> > DrawerPolyline::getPointsList()
{
    return m_pointsList;
}

}
