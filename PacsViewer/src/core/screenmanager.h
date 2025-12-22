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

#ifndef UDGSCREENMANAGER_H
#define UDGSCREENMANAGER_H

#include "dynamicmatrix.h"
#include "screenlayout.h"

class QWidget;
class QDesktopWidget;
class QRect;

namespace udg {

/**
This class is used for two main functionalities, maximizing a window when there are multiple screens,
and move a window from one screen to another.
It is also responsible for calculating the screen layout (ScreenLayout) in the system.
Its basic operation is through the maximize and moveToDesktop methods, to which the window to be treated is passed.
  */
class ScreenManager {
public:
    /// Default constructor.
    ScreenManager();
    
    ///  Returns the current layout
    ScreenLayout getScreenLayout() const;
    
    /// Method that maximizes the window passed by parameters
    /// on as many screens as possible so that it looks good.
    void maximize(QWidget *window);
    /// Returns true if the given window is maximized to more than one screen
    /// and false otherwise (including when it's simply maximized or not maximized).
    bool isMaximizedToMultipleScreens(QWidget *window);
    ///  Method for changing the window specified by the first parameter on the screen with the identifier of the second parameter.
    void moveToDesktop(QWidget *window, int idDesktop);
    ///  Method to move the window to the previous screen according to the screen identifier
    void moveToPreviousDesktop(QWidget *window);
    ///  Method to move the window to the next screen according to the screen ID
    void moveToNextDesktop(QWidget *window);

    /// Restores the window to the state before it was minimized
    void restoreFromMinimized(QWidget *window);
    
    /// Returns to which screen the widget / coordinate is located
    int getScreenID(QWidget *window) const;
    int getScreenID(const QPoint &point) const;

    /// Calculate the distribution matrix of the screens.
    /// Use the DynamicMatrix class to create the data structure to represent the distribution of
    /// the screens in space.
    DynamicMatrix computeScreenMatrix(QWidget *window) const;

private:
    /// Prepare the screen layout according to the current configuration
    void setupCurrentScreenLayout();
    
    /// Returns if the window passed by parameter to inside the screen.
    bool doesItFitInto(QWidget *window, int IdDesktop);
    ///  Resize the window passed by parameter so that it fits on the screen with IdDesktop id.
    ///  /// Also, center the window.
    void fitInto(QWidget *window, int IdDesktop);

    /// Returns the point at the top left according to the available geometry.
    /// (considering the taskbar)
    QPoint getTopLeft(const DynamicMatrix &dynamicMatrix) const;
    /// Returns the lowest point on the right according to the available geometry.
    /// (considering the taskbar)
    QPoint getBottomRight(const DynamicMatrix &dynamicMatrix) const;

    /// Returns the geometry that should be applied to the given window to maximize it to multiple screens.
    /// If the window can't be maximized to multiple screens returns a null QRect.
    QRect getGeometryToMaximizeToMulipleScreens(QWidget *window);

private:
    ///  Desktop manager where calls are made regarding multiple desktops or screens
    QDesktopWidget *m_applicationDesktop;

    /// /// Distribution of screens
    ScreenLayout m_screenLayout;
};

} // End namespace udg

#endif // UDGSCREENMANAGER_H
