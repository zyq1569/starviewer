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

#ifndef UDGSCREENLAYOUT_H
#define UDGSCREENLAYOUT_H

#include <QList>
#include <QRect>

#include "screen.h"

namespace udg {

/**
Class to represent the distribution of screens
 */
class ScreenLayout {
public:
    ScreenLayout();
    ~ScreenLayout();

    /// Add a screen to the layout. If the screen is not valid to add to the current layout,
    /// it will not be added and returned false, otherwise true.
    /// Screens with invalid IDs (<0) or with the same ID as an existing screen in the layout will not be added.
    /// A screen will not be added either if it is main and one already exists in the layout.
    bool addScreen(const Screen &screen);

    /// Returns the screen name to the layout
    int getNumberOfScreens() const;

    /// Reset the layout by removing all the screens it contained, leaving it blank
    void clear();

    /// Returns the screen with the requested ID. If the screen does not exist, it will return an empty Screen
    Screen getScreen(int screenID) const;

    /// Returns the id of the main screen. In case there is no definite one, it will return -1
    int getPrimaryScreenID() const;

    /// Tells us which screen is on the right / left of the screen with ID indicated.
    /// Returns -1 if there is no screen on the indicated side
    int getScreenOnTheRightOf(int screenID) const;
    int getScreenOnTheLeftOf(int screenID) const;

    /// Indicates which screen is considered anterior /
    /// posterior in the layout with respect to the screen passed by parameter
    /// Returns the same ID if we have no more screens than ours and -1 if the last screen does not exist.
    int getPreviousScreenOf(int screenID) const;
    int getNextScreenOf(int screenID) const;

private:
    /// Returns the screen index with ID screenID. If it does not exist, the index will be -1
    int getIndexOfScreen(int screenID) const;

private:
    /// Constant to determine the proximity threshold between windows
    static const int SamePositionThreshold;

    /// List of screens
    QList<Screen> m_screens;
};

} // End namespace udg

#endif
