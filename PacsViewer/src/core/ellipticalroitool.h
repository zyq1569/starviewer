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

#ifndef UDGELLIPTICALROITOOL_H
#define UDGELLIPTICALROITOOL_H

#include "roitool.h"

namespace udg {

class Q2DViewer;
class DrawerText;

/**
    Tool for calculating the area and average of grays of an oval
*/
class EllipticalROITool : public ROITool {
    Q_OBJECT
public:
    EllipticalROITool(QViewer *viewer, QObject *parent = 0);
    ~EllipticalROITool();

    void handleEvent(long unsigned eventID);

protected:
    virtual void setTextPosition(DrawerText *text);

private:
    /// Tool states
    enum { Ready, FirstPointFixed };

    /// Manage actions to be performed when the mouse is clicked
    void handlePointAddition();

    ///It simulates the shape of the ellipse when we have the first point and move the mouse
    void simulateEllipse();

    /// Calculates the center of the ellipse from the points entered by user interaction
    void computeEllipseCentre(double centre[3]);

    ///Update the points of the polygon to make the drawing of the oval
    void updatePolygonPoints();

    /// Give the drawing of the oval finished
    void closeForm();

    /// Removes the temporary representation from the tool
    void deleteTemporalRepresentation();

    /// Equalizes the depth of the elements that make up the final ellipse.
    void equalizeDepth();

private slots:
    ///Initialize the tool
    void initialize();

private:
    /// Points we write down from user interaction to create the oval
    double m_firstPoint[3];
    double m_secondPoint[3];

    ///Tool status
    int m_state;
};

}

#endif
