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

#ifndef UDGSCREEN_H
#define UDGSCREEN_H

#include <QRect>

namespace udg {

/**
Class to define a screen. It allows us to define its geometry, its ID and whether it is principal or not.
     It also incorporates methods for determining the relative positions between its geometry
 */
class Screen {
public:
    Screen ();
    Screen (const QRect & geometry, const QRect & availableGeometry);
    ~ Screen ();

    /// Assign / Get the geometry of the screen
    void setGeometry (const QRect & geometry);
    QRect getGeometry () const;

    /// Assign / Get the available geometry of the screen for applications, taking into account, for example,
    /// system menu bars and other items in the desktop environment
    void setAvailableGeometry (const QRect & geometry);
    QRect getAvailableGeometry () const;

    /// Indicates / Gets if the screen is main
    void setAsPrimary (bool isPrimary);
    bool isPrimary () const;

    /// Assign / get the screen ID
    void setID (int ID);
    int getID () const;

    /// Returns the properties of the object to us in a string
    QString toString () const;

    /// Tells us if this screen is higher / lower than the other. Just check that your top is above / below the top of the screen to compare,
    /// therefore will return true whether it is complete or partially above / below.
    bool isHigher (const Screen & screen);
    bool isLower (const Screen & screen);

    /// Tells us if this screen is more to the left / right than the other. Just check that everyone's left / right is more or less,
    /// will therefore return true whether it is complete or partially further left / right.
    bool isMoreToTheLeft (const Screen & screen);
    bool isMoreToTheRight (const Screen & screen);

    /// Whether this screen is completely above the other. Check that the bottom of this one is above the top of the other.
    bool isOver (const Screen & screen) const;

    /// Indicates whether this screen is completely below the other. Check that the top of this one is below the bottom of the other.
    bool isUnder (const Screen & screen) const;

    /// Indicates whether this screen is completely to the left of the other. Check that the right of this one is less than the left of the other.
    bool isOnLeft (const Screen & screen) const;

    /// Whether this screen is completely to the right of the other. Check that the left of this one is greater than the right of the other.
    bool isOnRight (const Screen & screen) const;

    /// Whether this screen is on top of the other. For it to be true they must be of the same width
    /// and the top of this must be equal to the bottom of the other
    bool isTop (const Screen & screen) const;

    /// Indicates whether this screen is below the other. For it to be true they must be of the same width
    /// and the bottom of this must be equal to the top of the other
    bool isBottom (const Screen & screen) const;

    /// Whether this screen is to the left of the other. For it to be true they must be of the same height
    /// and the right of this one must be equal to the left of the other
    bool isLeft (const Screen & screen) const;

    /// Whether this screen is to the right of the other. For it to be true they must be of the same height
    /// and the left of this must be equal to the right of the other
    bool isRight (const Screen & screen) const;

    /// Indicates whether this screen is diagonally above the left of the other. To be true
    /// the bottomRight point of this must be equal to the topLeft point of the other
    bool isTopLeft (const Screen & screen) const;

    /// Indicates whether this screen is diagonally above to the right of the other. To be true
    /// the bottomLeft point of this one must be equal to the topRight point of the other one
    bool isTopRight (const Screen & screen) const;

    /// Indicates whether this screen is diagonally below the left of the other. To be true
    /// the topRight point of this must be equal to the bottomLeft point of the other
    bool isBottomLeft (const Screen & screen) const;

    /// Indicates whether this screen is diagonally below to the right of the other. To be true
    /// the topLeft point of this one must be equal to the bottomRight point of the other one
    bool isBottomRight (const Screen & screen) const;

    /// Equality operator
    bool operator == (const Screen & screen) const;

    /// Constant to set the non-existent screen ID
    static const int NullScreenID;

private:
    /// Threshold with which we define the maximum distance between windows to determine that they are of corner
    static const int MaximumDistanceInBetween;

    /// Initialize default values
    void initializeValues();

private:
    /// Indicates whether it is a main screen
    bool m_isPrimary;

    /// Total and available geometry of the screen
    QRect m_geometry;
    QRect m_availableGeometry;

    /// Screen ID
    int m_ID;
};
} // End namespace udg

#endif
