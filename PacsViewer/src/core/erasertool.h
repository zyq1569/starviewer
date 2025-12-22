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

#ifndef UDGERASERTOOL_H
#define UDGERASERTOOL_H

#include "tool.h"
#include "orthogonalplane.h"

namespace udg {

class Q2DViewer;
class DrawerPolygon;
class DrawerPrimitive;

/**
Tool for deleting primitives and screen annotations.
If you click, it looks for the primitive closest to the clicked point and if it is within a threshold it clears it.
You can also click on a point and drag it to draw a box, deleting all the primitives that
are within the area delimiting the drawn box.
*/
class EraserTool : public Tool {
    Q_OBJECT
public:
    ///Possible tool states
    enum { StartClick, None };

    EraserTool(QViewer *viewer, QObject *parent = 0);
    ~EraserTool();

    void handleEvent(unsigned long eventID);

private:
    ///Start the process of determining the deletion zone
    void startEraserAction();

    ///Draw the erase area
    void drawAreaOfErasure();

    /// It will look for the closest primitive, and if it is close enough it will delete it.
    void erasePrimitive();

    /// Determine which primitive can be deleted with the given point, view, and slice.
    /// @param point Coordinate of world from which we want to determine if there is a near primitive
    /// @param view Current model view (Axial, Sagittal, Coronal)
    /// @param slice Slice where we want to get the primitive
    /// @return The primitive that is close to the given point, within a certain threshold.
    /// If the point is not close enough according to the threshold or there are no primitives in that slice,
    /// the returned primitive will be null.
    DrawerPrimitive* getErasablePrimitive(double point[3], const OrthogonalPlane &view, int slice);

private slots:
    /// Slot that returns the tool to its initial state
    void reset();

private:
    ///2D viewer we are working on
    Q2DViewer *m_2DViewer;

    ///It allows us to determine the status of the tool
    int m_state;

    ///First point of the erasure area
    double m_startPoint[3];

    /// Last point of the erasure area
    double m_endPoint[3];

    ///Polygon that marks the erasure area.
    DrawerPolygon *m_polygon;
};

}

#endif
