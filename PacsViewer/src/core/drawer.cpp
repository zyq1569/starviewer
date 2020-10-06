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

#include "drawer.h"
#include "drawerprimitive.h"
#include "logging.h"
#include "mathtools.h"
// Vtk
#include <vtkRenderer.h>
#include <QColor>

namespace udg {

Drawer::Drawer(Q2DViewer *viewer, QObject *parent)
    : QObject(parent), m_currentPlane(OrthogonalPlane::YZPlane), m_currentSlice(0)
{
    m_2DViewer = viewer;
    connect(m_2DViewer, SIGNAL(sliceChanged(int)), SLOT(refresh()));
    connect(m_2DViewer, SIGNAL(viewChanged(int)), SLOT(refresh()));
}

Drawer::~Drawer()
{
}

void Drawer::draw(DrawerPrimitive *primitive, const OrthogonalPlane &plane, int slice)
{
    switch (plane)
    {
    case OrthogonalPlane::XYPlane:
        m_XYPlanePrimitives.insert(slice, primitive);
        break;

    case OrthogonalPlane::YZPlane:
        m_YZPlanePrimitives.insert(slice, primitive);
        break;

    case OrthogonalPlane::XZPlane:
        m_XZPlanePrimitives.insert(slice, primitive);
        break;

    default:
        DEBUG_LOG("Pla no definit!");
        return;
        break;
    }

    // Depending on the current plan, it will be necessary to check
    // the visibility of the primitive according to the slice
    if (m_2DViewer->getView() == plane)
    {
        if (slice < 0 || m_2DViewer->getCurrentSlice() == slice)
        {
            primitive->setVisibility(true);
        }
        else
        {
            primitive->setVisibility(false);
        }
    }

    // We proceed to "paint it"
    renderPrimitive(primitive);
}

void Drawer::draw(DrawerPrimitive *primitive)
{
    m_top2DPlanePrimitives << primitive;

    //We proceed to "paint it"
    renderPrimitive(primitive);
}

void Drawer::clearViewer()
{
    QMultiMap<int, DrawerPrimitive*> primitivesContainer;
    switch (m_currentPlane)
    {
    case OrthogonalPlane::XYPlane:
        primitivesContainer = m_XYPlanePrimitives;
        break;

    case OrthogonalPlane::YZPlane:
        primitivesContainer = m_YZPlanePrimitives;
        break;

    case OrthogonalPlane::XZPlane:
        primitivesContainer = m_XZPlanePrimitives;
        break;

    default:
        DEBUG_LOG("Pla no definit!");
        return;
        break;
    }

    //We get the primitives from the current view and slice
    QList<DrawerPrimitive*> list = primitivesContainer.values(m_currentSlice);
    // We remove all primitives that are in the list, that do not have "owners" and that are erasable
    // Deleting will call the erasePrimitive () method that is already in charge of doing the "dirty work"
    foreach (DrawerPrimitive *primitive, list)
    {
        if (!primitive->hasOwners() && primitive->isErasable())
        {
            delete primitive;
        }
    }
    m_2DViewer->render();
}

void Drawer::addToGroup(DrawerPrimitive *primitive, const QString &groupName)
{
    // We do not check if it already exists or if it is in any other of the lists, it is not necessary.
    m_primitiveGroups.insert(groupName, primitive);
}

void Drawer::refresh()
{
    if (m_currentPlane == m_2DViewer->getView())
    {
        if (m_currentSlice != m_2DViewer->getCurrentSlice())
        {
            ///It is necessary to make invisible what was seen in
            ///this plan and slice and to make visible what is in the new slice
            hide(m_currentPlane, m_currentSlice);
            m_currentSlice = m_2DViewer->getCurrentSlice();
            show(m_currentPlane, m_currentSlice);
        }
    }
    else
    {
        /// It is necessary to make invisible what was seen in this plan and
        /// slice and to make visible what is in the new plan and slice
        hide(m_currentPlane, m_currentSlice);
        m_currentSlice = m_2DViewer->getCurrentSlice();
        m_currentPlane = m_2DViewer->getView();
        show(m_currentPlane, m_currentSlice);
    }
}

void Drawer::removeAllPrimitives()
{
    QList <DrawerPrimitive*> list = m_XYPlanePrimitives.values();
    QList <DrawerPrimitive*> sagitalList = m_YZPlanePrimitives.values();
    QList <DrawerPrimitive*> coronalList = m_XZPlanePrimitives.values();

    list += sagitalList;
    list += coronalList;
    list += m_top2DPlanePrimitives;

    foreach (DrawerPrimitive *primitive, list)
    {
        // TODOAttention with this treatment for the smart pointer substitute.
        // We'll only delete if no one owns it, but we won't check if they are "erasable" or not
        if (!primitive->hasOwners())
        {
            m_2DViewer->getRenderer()->RemoveViewProp(primitive->getAsVtkProp());
            delete primitive;
        }
    }
}

void Drawer::erasePrimitive(DrawerPrimitive *primitive)
{
    if (!primitive)
    {
        return;
    }
    // TODO Attention with this treatment for the smart pointer substitute.
    // HACK We will only delete if no one owns it
    if (primitive->hasOwners())
    {
        DEBUG_LOG("We do not erase the primitive. We have owners");
        return;
    }

    //Let’s see if he’s in any group
    QMutableMapIterator<QString, DrawerPrimitive*> groupsIterator(m_primitiveGroups);
    while (groupsIterator.hasNext())
    {
        groupsIterator.next();
        if (primitive == groupsIterator.value())
        {
            groupsIterator.remove();
        }
    }

    // We look in the axial plane
    if (erasePrimitiveFromContainer(primitive, m_XYPlanePrimitives))
    {
        // In principle the same primitive will only be in one of the lists
        return;
    }

    //We look in the sagittal plane
    if (erasePrimitiveFromContainer(primitive, m_YZPlanePrimitives))
    {
        return;
    }

    // We look at the coronal plane
    if (erasePrimitiveFromContainer(primitive, m_XZPlanePrimitives))
    {
        return;
    }

    // We look in the top layer
    if (m_top2DPlanePrimitives.contains(primitive))
    {
        m_top2DPlanePrimitives.removeAt(m_top2DPlanePrimitives.indexOf(primitive));
        m_2DViewer->getRenderer()->RemoveViewProp(primitive->getAsVtkProp());
        m_2DViewer->render();
    }
}

void Drawer::hide(const OrthogonalPlane &plane, int slice)
{
    QList<DrawerPrimitive*> primitivesList;
    switch (plane)
    {
    case OrthogonalPlane::XYPlane:
        primitivesList = m_XYPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::YZPlane:
        primitivesList = m_YZPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::XZPlane:
        primitivesList = m_XZPlanePrimitives.values(slice);
        break;
    }
    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        if (primitive->isVisible())
        {
            primitive->visibilityOff();
            primitive->update();
        }
    }
}

void Drawer::show(const OrthogonalPlane &plane, int slice)
{
    QList<DrawerPrimitive*> primitivesList;
    switch (plane)
    {
    case OrthogonalPlane::XYPlane:
        primitivesList = m_XYPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::YZPlane:
        primitivesList = m_YZPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::XZPlane:
        primitivesList = m_XZPlanePrimitives.values(slice);
        break;
    }

    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        if (!m_disabledPrimitives.contains(primitive) && (primitive->isModified() || !primitive->isVisible()))
        {
            primitive->visibilityOn();
            primitive->update();
        }
    }
}

int Drawer::getNumberOfDrawnPrimitives()
{
    return (m_XYPlanePrimitives.size() + m_YZPlanePrimitives.size() + m_XZPlanePrimitives.size());
}

void Drawer::disableGroup(const QString &groupName)
{
    bool hasToRender = false;
    QList<DrawerPrimitive*> primitiveList = m_primitiveGroups.values(groupName);
    foreach (DrawerPrimitive *primitive, primitiveList)
    {
        if (primitive->isModified() || primitive->isVisible())
        {
            primitive->visibilityOff();
            primitive->update();
            hasToRender = true;
        }

        m_disabledPrimitives.insert(primitive);
    }

    if (hasToRender)
    {
        m_2DViewer->render();
    }
}

void Drawer::enableGroup(const QString &groupName)
{
    QList<DrawerPrimitive*> currentVisiblePrimitives;
    int currentSlice = m_2DViewer->getCurrentSlice();
    switch (m_2DViewer->getView())
    {
    case OrthogonalPlane::XYPlane:
        currentVisiblePrimitives = m_XYPlanePrimitives.values(currentSlice);
        break;

    case OrthogonalPlane::YZPlane:
        currentVisiblePrimitives = m_YZPlanePrimitives.values(currentSlice);
        break;

    case OrthogonalPlane::XZPlane:
        currentVisiblePrimitives = m_XZPlanePrimitives.values(currentSlice);
        break;
    }
    currentVisiblePrimitives << m_top2DPlanePrimitives;

    bool hasToRender = false;
    QList<DrawerPrimitive*> groupPrimitives = m_primitiveGroups.values(groupName);
    foreach (DrawerPrimitive *primitive, groupPrimitives)
    {
        /// If the primitive meets the visibility conditions
        /// by enable status we will make it visible
        if (currentVisiblePrimitives.contains(primitive))
        {
            if (primitive->isModified() || !primitive->isVisible())
            {
                primitive->visibilityOn();
                primitive->update();
                hasToRender = true;
            }
        }

        //We remove it from the list of disabled primitives
        m_disabledPrimitives.remove(primitive);
    }

    if (hasToRender)
    {
        m_2DViewer->render();
    }
}

DrawerPrimitive* Drawer::getNearestErasablePrimitiveToPoint(double point[3], const OrthogonalPlane &view, int slice, double closestPoint[3])
{
    double distance;
    double minimumDistance = MathTools::DoubleMaximumValue;
    QList<DrawerPrimitive*> primitivesList;

    DrawerPrimitive *nearestPrimitive = 0;

    switch (view)
    {
    case OrthogonalPlane::XYPlane:
        primitivesList = m_XYPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::YZPlane:
        primitivesList = m_YZPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::XZPlane:
        primitivesList = m_XZPlanePrimitives.values(slice);
        break;

    default:
        break;
    }

    double localClosestPoint[3];
    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        if (primitive->isErasable())
        {
            distance = primitive->getDistanceToPoint(point, localClosestPoint);
            if (distance <= minimumDistance)
            {
                minimumDistance = distance;
                nearestPrimitive = primitive;
                closestPoint[0] = localClosestPoint[0];
                closestPoint[1] = localClosestPoint[1];
                closestPoint[2] = localClosestPoint[2];
            }
        }
    }
    return nearestPrimitive;
}

void Drawer::erasePrimitivesInsideBounds(double bounds[6], const OrthogonalPlane &view, int slice)
{
    QList<DrawerPrimitive*> primitivesList;

    switch (view)
    {
    case OrthogonalPlane::XYPlane:
        primitivesList = m_XYPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::YZPlane:
        primitivesList = m_YZPlanePrimitives.values(slice);
        break;

    case OrthogonalPlane::XZPlane:
        primitivesList = m_XZPlanePrimitives.values(slice);
        break;

    default:
        break;
    }

    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        if (primitive->isErasable())
        {
            if (isPrimitiveInside(primitive, view, bounds))
            {
                erasePrimitive(primitive);
            }
        }
    }
}

bool Drawer::isPrimitiveInside(DrawerPrimitive *primitive, const OrthogonalPlane &view, double bounds[6])
{
    // We check that the bounds of the primitive are contained
    // within those that have passed us by parameter
    double primitiveBounds[6];
    primitive->getBounds(primitiveBounds);

    int xIndex = view.getXIndex();
    int yIndex = view.getYIndex();

    bool inside = false;
    if (bounds[xIndex * 2] <= primitiveBounds[xIndex * 2] && bounds[xIndex * 2 + 1] >= primitiveBounds[xIndex * 2 + 1] &&
            bounds[yIndex * 2] <= primitiveBounds[yIndex * 2] && bounds[yIndex * 2 + 1] >= primitiveBounds[yIndex * 2 + 1])
    {
        inside = true;
    }

    return inside;
}

bool Drawer::erasePrimitiveFromContainer(DrawerPrimitive *primitive, QMultiMap<int, DrawerPrimitive*> &primitiveContainer)
{
    bool found = false;
    QMutableMapIterator<int, DrawerPrimitive*> containerIterator(primitiveContainer);
    while (containerIterator.hasNext() && !found)
    {
        containerIterator.next();
        if (primitive == containerIterator.value())
        {
            found = true;
            containerIterator.remove();
            m_2DViewer->getRenderer()->RemoveViewProp(primitive->getAsVtkProp());
        }
    }

    return found;
}

void Drawer::renderPrimitive(DrawerPrimitive *primitive)
{
    vtkProp *prop = primitive->getAsVtkProp();
    if (prop)
    {
        connect(primitive, SIGNAL(dying(DrawerPrimitive*)), SLOT(erasePrimitive(DrawerPrimitive*)));
        m_2DViewer->getRenderer()->AddViewProp(prop);
        if (primitive->isVisible())
        {
            m_2DViewer->render();
        }
    }
}

}
