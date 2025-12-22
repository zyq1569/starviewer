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

#ifndef UDGDISTANCETOOL_H
#define UDGDISTANCETOOL_H

#include "genericdistancetool.h"

namespace udg {

/**
    Tool for measuring distances in a 2D viewfinder.
    It allows us to fix two points on the screen and calculate the distance between them.

    To mark each of the points will be done with a click or double-click with the left button
    of the mouse.
    Once the first point of the distance to be calculated is marked, a line will be displayed that will join
    the first point marked with the point where the mouse pointer is. While the user does not
    marked the second point, no distance information will be displayed at that time.

    Once the second point is marked, the measurement will be displayed in millimeters, if information exists
    of spacing, in pixels otherwise.

    Once the measurement is completed, as many measurements as desired can be recorded successively.

    The annotated measurements will only appear on the images on which they were taken,
    but they are not erased if we change image. If we change the image but show it again later
    the annotations made on this previously, will reappear.

    When the viewfinder input is changed, the entries made up to that point will be deleted.
    When the tool is deactivated, the annotations made up to that point will be maintained.
  */
class DistanceTool : public GenericDistanceTool {
    Q_OBJECT
public:
    DistanceTool(QViewer *viewer, QObject *parent = 0);
    ~DistanceTool();

protected:
    void handleLeftButtonPress();
    void handleMouseMove();
    void abortDrawing();

private:
    /// Manage what point of distance we are drawing. It will be called every time
    /// we have left clicked with the left mouse button.
    void handlePointAddition();

    /// Marks a new distance point. If the corresponding primitive
    /// not created is created before adding the new point.
    void annotateNewPoint();

    ///Simulate the line when the mouse moves and we have the first point marked.
    void simulateLine();

    ///Removes the temporary representation from the tool
    void deleteTemporalRepresentation();

    ///Equalizes the depth of the elements that make up the final distance.
    void equalizeDepth();

private slots:
    /// Initializes the status of the tool.
    void initialize();

private:
    /// Possible states of the drawn line.
    enum { NoPointFixed, FirstPointFixed };

    ///Line status.
    int m_lineState;
};

}

#endif
