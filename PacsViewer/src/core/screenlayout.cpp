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

#include "screenlayout.h"

#include <QDesktopWidget>
#include <QApplication>

namespace udg {

const int ScreenLayout::SamePositionThreshold = 5;

ScreenLayout::ScreenLayout()
{
}

ScreenLayout::~ScreenLayout()
{
}

bool ScreenLayout::addScreen(const Screen &screen)
{
    if (screen.getID() <= Screen::NullScreenID)
    {
        return false;
    }
    
    bool success = true;
    int numberOfScreens = getNumberOfScreens();
    int i = 0;
    while (i < numberOfScreens && success)
    {
        success = m_screens.at(i).getID() != screen.getID() && !(m_screens.at(i).isPrimary() && screen.isPrimary());
        ++i;
    }

    if (success)
    {
        m_screens << screen;
    }
    
    return success;
}

int ScreenLayout::getNumberOfScreens() const
{
    return m_screens.count();
}

void ScreenLayout::clear()
{
    m_screens.clear();
}

Screen ScreenLayout::getScreen(int screenID) const
{
    Screen screen;
    
    int index = getIndexOfScreen(screenID);
    if (index > -1)
    {
        screen = m_screens.at(index);
    }

    return screen;
}

int ScreenLayout::getPrimaryScreenID() const
{
    int id = -1;
    bool found = false;
    int numberOfScreens = getNumberOfScreens();
    int i = 0;
    while (i < numberOfScreens && !found)
    {
        if (m_screens.at(i).isPrimary())
        {
            id = m_screens.at(i).getID();
            found = true;
        }
        ++i;
    }

    return id;
}

int ScreenLayout::getScreenOnTheRightOf(int screenID) const
{
    int rightScreenID = Screen::NullScreenID;
    Screen screen = getScreen(screenID);
    Screen screenToCompare;
    // Find a screen on the right and at the same height + or -
    for (int i = 0; i < getNumberOfScreens(); ++i)
    {
        screenToCompare = getScreen(i);
        // If it is on the right, but it is not completely above or below
        if (screenToCompare.isOnRight(screen) && !screenToCompare.isOver(screen) && !screenToCompare.isUnder(screen))
        {
            // Si encara no hem trobat cap pantalla
            if (rightScreenID == Screen::NullScreenID)
            {
                rightScreenID = i;
            }
            // From the screens on the right, we want the closest one
            // If the screen we found is further to the left than the one we have
            else if (screenToCompare.isOnLeft(getScreen(rightScreenID)))
            {
                rightScreenID = i;
            }
        }
    }

    return rightScreenID;
}

int ScreenLayout::getScreenOnTheLeftOf(int screenID) const
{
    int leftScreenID = Screen::NullScreenID;
    Screen screen = getScreen(screenID);
    Screen screenToCompare;
    // Find a screen on the left and at the same height + or -
    for (int i = 0; i < getNumberOfScreens(); ++i)
    {
        screenToCompare = getScreen(i);
        // If it is on the left, but it is not completely above or below
        if (screenToCompare.isOnLeft(screen) && !screenToCompare.isOver(screen) && !screenToCompare.isUnder(screen))
        {
            // If we have not found any screen yet
            if (leftScreenID == Screen::NullScreenID)
            {
                leftScreenID = i;
            }
            // From the screens on the left, we want the closest one
            // If the screen we found is further to the right than the one we have
            else if (screenToCompare.isOnRight(getScreen(leftScreenID)))
            {
                leftScreenID = i;
            }
        }
    }

    return leftScreenID;
}

int ScreenLayout::getPreviousScreenOf(int screenID) const
{
    if (getIndexOfScreen(screenID) < 0)
    {
        return -1;
    }
    
    Screen currentScreen = getScreen(screenID);

    int previousScreenID = getScreenOnTheLeftOf(screenID);
    // If there is no screen on the left, then look for the one on the far right that is above it
    if (previousScreenID == -1)
    {
        Screen screen;
        for (int i = 0; i < getNumberOfScreens(); i++)
        {
            screen = getScreen(i);
            if (screen.isHigher(currentScreen))
            {
                //If we have not found any screen yet
                if (previousScreenID == -1)
                {
                    previousScreenID = i;
                }
                //From the screens above, we want the far right
                else if (screen.isOnRight(getScreen(previousScreenID)))
                {
                    previousScreenID = i;
                }
            }
        }
    }

    //If there is no screen above the current one, we will take the one on the bottom right
    if (previousScreenID == -1)
    {
        Screen screen;
        // This ensures that you will never get to the brave moveToDesktop -1
        previousScreenID = 0;
        for (int i = 1; i < getNumberOfScreens(); i++)
        {
            screen = getScreen(i);
            // Si està per sota de l'actual ens la quedem
            if (screen.isUnder(getScreen(previousScreenID)))
            {
                previousScreenID = i;
            }
            //If not, if it is not above, we take it if it is further to the right than the current one
            else
            {
                Screen previousScreen = getScreen(previousScreenID);
                if (!screen.isOver(previousScreen) && screen.isMoreToTheRight(previousScreen))
                {
                    previousScreenID = i;
                }
            }
        }
    }

    return previousScreenID;
}

int ScreenLayout::getNextScreenOf(int screenID) const
{
    if (getIndexOfScreen(screenID) < 0)
    {
        return -1;
    }
    
    Screen currentScreen = getScreen(screenID);

    // Find a screen on the right and at the same height + or -
    int nextScreenID = getScreenOnTheRightOf(screenID);
    
    // If there is no screen on the right, then look for the one on the far left that is below it
    if (nextScreenID == -1)
    {
        Screen screen;
        for (int i = 0; i < getNumberOfScreens(); i++)
        {
            screen = getScreen(i);
            if (screen.isLower(currentScreen))
            {
                // If we have not found any screen yet
                if (nextScreenID == -1)
                {
                    nextScreenID = i;
                }
                // From the screens below, we want the one on the left
                else if (screen.isOnLeft(getScreen(nextScreenID)))
                {
                    nextScreenID = i;
                }
            }
        }
    }

    // If there is no patalla below the current one, we will take the one on the top left
    if (nextScreenID == -1)
    {
        Screen screen;
        // This ensures that you will never get to the brave moveToDesktop -1
        nextScreenID = 0;
        for (int i = 1; i < getNumberOfScreens(); i++)
        {
            screen = getScreen(i);
            // If it is above the current one we will keep it
            if (screen.isOver(getScreen(nextScreenID)))
            {
                nextScreenID = i;
            }
            // If not, if it's not below, we'll take it if it's further to the left than the current one
            else
            {
                Screen nextScreen = getScreen(nextScreenID);
                if (!screen.isUnder(nextScreen) && screen.isMoreToTheLeft(nextScreen))
                {
                    nextScreenID = i;
                }
            }
        }
    }

    return nextScreenID;
}

int ScreenLayout::getIndexOfScreen(int screenID) const
{
    int i = 0;
    int numberOfScreens = getNumberOfScreens();
    while (i < numberOfScreens)
    {
        if (m_screens.at(i).getID() == screenID)
        {
            return i;
        }
        ++i;
    }

    return -1;
}

} // End namespace udg
