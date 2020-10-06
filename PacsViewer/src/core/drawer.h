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

#ifndef UDGDRAWER_H
#define UDGDRAWER_H

#include <QObject>
#include <QMultiMap>
#include <QSet>

#include "q2dviewer.h"

namespace udg {

class DrawerPrimitive;

/**
    Class responsible for painting graphic primitive objects in the assigned viewer
*/
class Drawer : public QObject {
    Q_OBJECT
public:
    Drawer(Q2DViewer *viewer, QObject *parent = 0);
    ~Drawer();

    /// Draw the given primitive on the indicated plan and slice
    /// @param primitive Primitiva a pintar
    /// @param plane Plan on which we want to paint the primitive
    /// @param slice Slice to which we attach the primitive
    void draw(DrawerPrimitive *primitive, const OrthogonalPlane &plane, int slice);

    /// Always draw the primitive at the top no matter what plane or slice we are on.
    /// Visibility will depend only on the isVisible () property of the primitive
    void draw(DrawerPrimitive *primitive);

    /// Deletes all erasable primitives that are seen in the viewfinder, that is, in the current plan and slices.
    void clearViewer();

    /// We add a primitive to the indicated group.
    /// @param primitive Primitive that we want to add
    /// @param groupName name of the group where we want to include it
    void addToGroup(DrawerPrimitive *primitive, const QString &groupName);

    /// Disable primitives in a given group, making them never visible, under any conditions
    /// @param groupName Name of the group of primitives we want to disable
    void disableGroup(const QString &groupName);

    /// Enables primitives in a certain group, making them return to their usual behavior
    /// where the Drawer will decide whether or not they should be visible according to the assigned plan and slice
    /// @param groupName Name of the group of primitives we want to enable
    void enableGroup(const QString &groupName);

    /// Returns the erasable primitive closest to the given point, within the provided view and slice
    /// This method does not take into account any proximity threshold, that is, it returns the primitive that in terms
    /// absolute is closer to the given point. In case there is no primitive for that sight and slice, it will be returned null.
    DrawerPrimitive* getNearestErasablePrimitiveToPoint(double point[3], const OrthogonalPlane &view, int slice, double closestPoint[3]);

    ///It deletes the erasable primitives that are within the area bounded by the points passed by parameter.
    void erasePrimitivesInsideBounds(double bounds[6], const OrthogonalPlane &view, int slice);

    ///It tells us the total of primitives drawn in all views
    int getNumberOfDrawnPrimitives();

public slots:
    /// It stops keeping the primitive within its internal structure
    /// and removes it from the scene where it was being painted
    /// @param primitive Primitive that we want to stop controlling
    void erasePrimitive(DrawerPrimitive *primitive);

    ///Deletes all primitives registered in the drawer (includes non-erasable primitives).
    void removeAllPrimitives();

private:
    /// Shows / hides the primitives in a given plane and slice
    /// @param plane Plan on which we want to show / hide the primitives
    /// @param slice Slice inside that plane.
    void hide(const OrthogonalPlane &plane, int slice);
    void show(const OrthogonalPlane &plane, int slice);

    ///It tells us if the given primitive, which is in the view view, is within the indicated bounds
    bool isPrimitiveInside(DrawerPrimitive *primitive, const OrthogonalPlane &view, double bounds[6]);

    /// Deletes the given primitive from the specified primitive container.
    /// If you find it delete it. Returns true if found, false otherwise.
    bool erasePrimitiveFromContainer(DrawerPrimitive *primitive, QMultiMap<int, DrawerPrimitive*> &primitiveContainer);

    ///Makes the primitive visible in the associated viewer
    void renderPrimitive(DrawerPrimitive *primitive);

private slots:
    ///Refreshes the primitives to be viewed by the viewer according to their status
    void refresh();

private:
    ///Viewer on which we will paint the primitives
    Q2DViewer *m_2DViewer;

    /// Containers of primitives for each possible plan
    QMultiMap<int, DrawerPrimitive*> m_XYPlanePrimitives;
    QMultiMap<int, DrawerPrimitive*> m_YZPlanePrimitives;
    QMultiMap<int, DrawerPrimitive*> m_XZPlanePrimitives;
    QList<DrawerPrimitive*> m_top2DPlanePrimitives;

    /// Flat and slice in what is currently the 2D Viewer. It serves to control
    /// the slices and plan changes, to know which primitives we need to clean
    OrthogonalPlane m_currentPlane;
    int m_currentSlice;

    /// Groups of primitives. We group them by name
    QMultiMap<QString, DrawerPrimitive*> m_primitiveGroups;

    ///Set of primitives in disabled state
    QSet<DrawerPrimitive*> m_disabledPrimitives;
};

}

#endif
