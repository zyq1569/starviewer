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

#include "screenmanager.h"
#include "dynamicmatrix.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>

namespace udg {

ScreenManager::ScreenManager()
{
    setupCurrentScreenLayout();
}

void ScreenManager::setupCurrentScreenLayout()
{
    m_screenLayout.clear();
    m_applicationDesktop = QApplication::desktop();

    Screen screen;
    int primaryScreen = m_applicationDesktop->primaryScreen();
    QRect temp;
    for (int i = 0; i < m_applicationDesktop->screenCount(); ++i)
    {
        screen.setID(i);
        temp = m_applicationDesktop->screenGeometry(i);
        //screen.setGeometry(m_applicationDesktop->screenGeometry(i));
        //screen.setGeometry(m_applicationDesktop->screenGeometry(i));
        screen.setGeometry(temp);
        screen.setGeometry(temp);
        screen.setAvailableGeometry(m_applicationDesktop->availableGeometry(i));
        screen.setAsPrimary(primaryScreen == i);
        m_screenLayout.addScreen(screen);
    }
}

ScreenLayout ScreenManager::getScreenLayout() const
{
    return m_screenLayout;
}

void ScreenManager::maximize(QWidget *window)
{
    QRect geometryToMaximizeToMultipleScreens = getGeometryToMaximizeToMulipleScreens(window);

    if (!geometryToMaximizeToMultipleScreens.isNull())
    {
        window->setGeometry(geometryToMaximizeToMultipleScreens);
    }
    else
    {
        window->showMaximized();
    }
}

bool ScreenManager::isMaximizedToMultipleScreens(QWidget *window)
{
    if (window->isMinimized() || window->isMaximized() || window->isFullScreen())
    {
        return false;
    }

    int left1, top1, right1, bottom1, left2, top2, right2, bottom2;
    window->geometry().getCoords(&left1, &top1, &right1, &bottom1);
    getGeometryToMaximizeToMulipleScreens(window).getCoords(&left2, &top2, &right2, &bottom2);

    const int Tolerance = 32;

    return qAbs(left1 - left2) <= Tolerance && qAbs(top1 - top2) <= Tolerance && qAbs(right1 - right2) <= Tolerance && qAbs(bottom1 - bottom2) <= Tolerance;
}

void ScreenManager::moveToDesktop(QWidget *window, int idDesktop)
{
    // Aquesta variable s'utilitza per saber si abans de fer el fit la finestra hi cabia.
    bool fits = doesItFitInto(window, idDesktop);

    if (window->isMaximized())
    {
        // The window is maximized and moves to another screen
        // therefore it is de-maximized, moved and maximized again
        // If you move to the same screen, nothing is done
        int desktopIAm = getScreenID(window);
        if (idDesktop != desktopIAm)
        {
            window->showNormal();
            fitInto(window, idDesktop);
            window->showMaximized();
        }
    }
    else
    {
        fitInto(window, idDesktop);
        if (!fits)
        {
            window->showMaximized();
        }
    }
}

void ScreenManager::moveToPreviousDesktop(QWidget *window)
{
    int desktopIAm = getScreenID(window);
    int desktopIllBe = m_screenLayout.getPreviousScreenOf(desktopIAm);

    moveToDesktop(window, desktopIllBe);
}

void ScreenManager::moveToNextDesktop(QWidget *window)
{
    int desktopIAm = getScreenID(window);
    int desktopIllBe = m_screenLayout.getNextScreenOf(desktopIAm);

    moveToDesktop(window, desktopIllBe);
}

void ScreenManager::restoreFromMinimized(QWidget *window)
{
    // WindowState accepts a combination of Enums Qt :: WindowState, what we do with this method is
    // remove the minimized state and tell it back to active
    // Suppose we had the window maximized, if we minimize it WindowState will have the value
    // Qt :: WindowMinimized | QtWindowMaximized, and applying this code we will remove the state Qt: WindowMinimized
    // and the WindowActive restores the window to its original state
    window->setWindowState(window->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
}

int ScreenManager::getScreenID(QWidget *window) const
{
    return m_applicationDesktop->screenNumber(window);
}

int ScreenManager::getScreenID(const QPoint &point) const
{
    return m_applicationDesktop->screenNumber(point);
}

DynamicMatrix ScreenManager::computeScreenMatrix(QWidget *window) const
{
    int desktopIAm = getScreenID(window);

    //First of all look for the screens in the same row
    DynamicMatrix dynamicMatrix;
    int indexLeft = 0;
    int indexRight = 0;
    int indexTop = 0;
    int indexBottom = 0;
    dynamicMatrix.setValue(0, 0, desktopIAm);
    int numberOfScreens = m_screenLayout.getNumberOfScreens();
    bool changes = true;
    Screen screen1;
    Screen screen2;
    while (changes)
    {
        changes = false;
        for (int i = 0; i < numberOfScreens; i++)
        {
            screen1 = m_screenLayout.getScreen(i);
            screen2 = m_screenLayout.getScreen(dynamicMatrix.getValue(0, indexLeft));
            if (screen1.isLeft(screen2))
            {
                dynamicMatrix.setValue(0, --indexLeft, i);
                changes = true;
            }
            else
            {
                screen2 = m_screenLayout.getScreen(dynamicMatrix.getValue(0, indexRight));
                if (screen1.isRight(screen2))
                {
                    dynamicMatrix.setValue(0, ++indexRight, i);
                    changes = true;
                }
            }
        }
    }
    // Now go see if the whole row has screens above or below
    changes = true;
    bool keepLookingUp = true;
    bool keepLookingDown = true;
    while (changes)
    {
        changes = false;
        // See if you can add the row above
        QList<int> topRow;
        int index = indexLeft;
        // If we have to look at the row above true otherwise false
        bool rowFound = keepLookingUp;
        while (keepLookingUp && index <= indexRight)
        {
            bool found = false;
            int j = 0;
            while (j < numberOfScreens)
            {
                screen1 = m_screenLayout.getScreen(j);
                screen2 = m_screenLayout.getScreen(dynamicMatrix.getValue(indexTop, index));
                if (screen1.isTop(screen2))
                {
                    topRow.append(j);
                    found = true;
                    j = numberOfScreens;
                }
                j++;
            }
            if (found)
            {
                index++;
            }
            else
            {
                rowFound = false;
                keepLookingUp = false;
                index = indexRight + 1;
            }
        }
        //If ALL of the row above could be added
        if (rowFound)
        {
            indexTop++;
            for (int i = indexLeft, j = 0; i <= indexRight; i++, j++)
            {
                dynamicMatrix.setValue(indexTop, i, topRow[j]);
            }
            changes = true;
        }

        //See if you can add the row below
        QList<int> bottomRow;
        index = indexLeft;
        //If I look at the row below true, otherwise false
        rowFound = keepLookingDown;
        while (keepLookingDown && index <= indexRight)
        {
            bool found = false;
            int j = 0;
            while (j < numberOfScreens)
            {
                screen1 = m_screenLayout.getScreen(j);
                screen2 = m_screenLayout.getScreen(dynamicMatrix.getValue(indexBottom, index));
                if (screen1.isBottom(screen2))
                {
                    bottomRow.append(j);
                    found = true;
                    j = numberOfScreens;
                }
                j++;
            }
            if (found)
            {
                index++;
            }
            else
            {
                rowFound = false;
                keepLookingDown = false;
                index = indexRight + 1;
            }
        }
        // If ALL row above could be added
        if (rowFound)
        {
            indexBottom--;
            for (int i = indexLeft, j = 0; i <= indexRight; i++, j++)
            {
                dynamicMatrix.setValue(indexBottom, i, bottomRow[j]);
            }
            changes = true;
        }
    }

    return dynamicMatrix;
}

bool ScreenManager::doesItFitInto(QWidget *window, int IdDesktop)
{
    QRect newDesktopAvailableGeometry = m_screenLayout.getScreen(IdDesktop).getAvailableGeometry();

    // If it’s too wide or too high, it doesn’t fit.
    return !(newDesktopAvailableGeometry.width() < window->minimumWidth() || newDesktopAvailableGeometry.height() < window->minimumHeight());
}

void ScreenManager::fitInto(QWidget *window, int IdDesktop)
{
    QRect newDesktopAvailableGeometry = m_screenLayout.getScreen(IdDesktop).getAvailableGeometry();
    int newDesktopWidth = newDesktopAvailableGeometry.width();
    int newDesktopHeight = newDesktopAvailableGeometry.height();

    int width = window->frameSize().width();
    int height = window->frameSize().height();
    int x = newDesktopAvailableGeometry.topLeft().x();
    int y = newDesktopAvailableGeometry.topLeft().y();

    //Find the size of the frame and window
    QRect frameSize = window->frameGeometry();
    QRect windowSize = window->geometry();

    //Find the actual size of the corners and the title bar
    int topBorder = windowSize.top() - frameSize.top();
    int bottomBorder = frameSize.bottom() - windowSize.bottom();
    int leftBorder = windowSize.left() - frameSize.left();
    int rightBorder = frameSize.right() - windowSize.right();

    //If the window is wider than the screen
    if (width > newDesktopWidth)
    {
        width = newDesktopWidth;
        x += leftBorder;
    }
    // Otherwise focus
    else
    {
        x = x + newDesktopWidth / 2 - width / 2 + leftBorder;
    }

    //If the window is higher than the screen
    if (height > newDesktopHeight)
    {
        height = newDesktopHeight;
        y += topBorder;
    }
    // Otherwise focus
    else
    {
        y = y + newDesktopHeight / 2 - height / 2 + topBorder;
    }

    // We took the size of the window from the frame, so we have to remove it
    // the corners so that the size is the same
    window->setGeometry(x,
                        y,
                        width - leftBorder - rightBorder,
                        height - topBorder - bottomBorder);
}

QPoint ScreenManager::getTopLeft(const DynamicMatrix &dynamicMatrix) const
{
    // First of all look for the left corner, from the list of monitors on the left,
    // take the maximum, in case the windows bar is on the left in any of them
    QList<int> screens = dynamicMatrix.getLeftColumn();
    int x = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().left();
    for (int i = 1; i < screens.count(); i++)
    {
        x = std::max(x, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().left());
    }
    //The same for the top
    screens = dynamicMatrix.getTopRow();
    int y = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().top();
    for (int i = 1; i < screens.count(); i++)
    {
        y = std::max(y, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().top());
    }

    return QPoint(x, y);
}

QPoint ScreenManager::getBottomRight(const DynamicMatrix &dynamicMatrix) const
{

    // First of all look for the right corner, from the list of monitors on the right,
    // take the minimum, in case the windows bar is on the right in any of them
    QList<int> screens = dynamicMatrix.getRightColumn();
    int x = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().right();
    for (int i = 1; i < screens.count(); i++)
    {
        x = std::min(x, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().right());
    }
    // The same for the bottom
    screens = dynamicMatrix.getBottomRow();
    int y = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().bottom();
    for (int i = 1; i < screens.count(); i++)
    {
        y = std::min(y, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().bottom());
    }

    // Add 1 to compensate for the fact that right() and bottom() don't return
    // the true right and the true bottom. From QRect documentation:
    // "Note that for historical reasons the values returned by the bottom()
    // and right() functions deviate from the true bottom-right corner of the rectangle:
    // The right() function returns left() + width() - 1 and the bottom() function returns top() + height() - 1."
    return QPoint(x + 1, y + 1);
}

QRect ScreenManager::getGeometryToMaximizeToMulipleScreens(QWidget *window)
{
    DynamicMatrix dynamicMatrix = computeScreenMatrix(window);

    if (dynamicMatrix.isMaximizable())
    {
        if (window->isMaximized())
        {
            window->showNormal();
        }

        //Find the size of the frame and window
        QRect frameSize = window->frameGeometry();
        QRect windowSize = window->geometry();

        //Find the actual size of the corners and the title bar
        int topBorder = windowSize.top() - frameSize.top();
        int bottomBorder = frameSize.bottom() - windowSize.bottom();
        int leftBorder = windowSize.left() - frameSize.left();
        int rightBorder = frameSize.right() - windowSize.right();

        // Grab the top left and bottomright to determine the dimensions of the window
        QPoint topLeft = getTopLeft(dynamicMatrix);
        QPoint bottomRight = getBottomRight(dynamicMatrix);

        // Calculate what size it should be.
        // The geometry of the window must be passed to it, without corners.
        int x = topLeft.x() + leftBorder;
        int y = topLeft.y() + topBorder;
        // x val x + leftBorder
        int width = bottomRight.x() - x - rightBorder;
        // y val y + topBorder
        int height = bottomRight.y() - y - bottomBorder;

        return QRect(x, y, width, height);
    }
    else
    {
        return QRect();
    }
}

} // End namespace udg
