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

#ifndef UDGQSCREENDISTRIBUTION_H
#define UDGQSCREENDISTRIBUTION_H

#include <QWidget>
#include <QList>
#include <QRect>

class QPaintEvent;
class QMouseEvent;

namespace udg {

/**
Class showing the screens (or monitors) available in a small window (QDialog)
     and lets you select which one you want to move the main window to.
     Returns the screen ID, and it is outside of it that the operations must be performed
     whether or not to move the screen.
  */
class QScreenDistribution : public QWidget {
    Q_OBJECT

public:
    /// Builder, window is the window that will move.
    QScreenDistribution(QWidget *parent = 0);
    /// Default destroyer
    ~QScreenDistribution();

signals:
    int screenClicked(int screenIndex);

protected:
    /// Retorna la mida adequada per a visualitzar correctament la finestra (400,200)
    QSize sizeHint() const;
    /// Capture mouse movement, check if the mouse is above any of the icons
    /// screens and repaints it by giving it relief.
    void mouseMoveEvent(QMouseEvent *event);
    /// Capture the position of the mouse click and check which screen icon has been clicked.
    /// Then call ScreenManager to move the main window to it.
    void mousePressEvent(QMouseEvent *event);
    ///Paint the distribution of the screens. If the mouse is over one, paint it with relief.
    void paintEvent(QPaintEvent *event);

private:
    /// Calculates the size of the window and calculates the positions of the window icons
    /// so that they are centered and scaled correctly.
    void computeSizesAndPositions();

private:
    /// List of rectangles representing monitors, with positions ready to draw.
    QList<QRect> m_screens;
    /// Indicates the monitor identifier on the icon where the mouse is located.
    int m_mouseInScreen;
    ///Font size used for the number on each screen.
    int m_screenNumberPixelSize;

    /// Margin around the window where to start painting.
    static const int WidgetMargin;
    /// Maximum size that the number of each screen can have.
    static const int MaximumScreenNumberPixelSize;
};

} // End namespace udg

#endif // UDGQSCREENDISTRIBUTION_H
