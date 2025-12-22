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

#ifndef UDGHANGINGPROTOCOLLAYOUT_H
#define UDGHANGINGPROTOCOLLAYOUT_H

#include <QStringList>

namespace udg {

class HangingProtocolLayout {

public:
    HangingProtocolLayout();

    ~HangingProtocolLayout();

    ///Enter the name of the screens
    void setNumberOfScreens(int numberOfScreens);
    /// Put the pixel list vertically
    void setVerticalPixelsList(const QList<int> &verticalPixelsList);
    /// Put the pixel list horizontally
    void setHorizontalPixelsList(const QList<int> &horizontalPixelsList);
    /// Put the list of positions of the viewers
    void setDisplayEnvironmentSpatialPositionList(const QStringList &displayEnvironmentSpatialPosition);
    /// Get the screen name
    int getNumberOfScreens() const;
    /// Get the vertical pixel list
    QList<int> getVerticalPixelsList() const;
    /// Get the horizontal pixel list
    QList<int> getHorizontalPixelsList() const;
    /// Get the list of viewer positions
    QStringList getDisplayEnvironmentSpatialPositionList() const;

private:
    /// Hanging Protocol Environment Module Attributes
    /// Number of screens
    int m_numberOfScreens;
    ///Vertical pixel list
    QList<int> m_verticalPixelsList;
    ///Horizontal pixel list
    QList<int> m_horizontalPixelsList;
    /// List of viewer positions
    QStringList m_displayEnvironmentSpatialPosition;
};

}

#endif
