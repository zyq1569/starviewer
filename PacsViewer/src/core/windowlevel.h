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

#ifndef UDGWINDOWLEVEL_H
#define UDGWINDOWLEVEL_H

#include <QString>

namespace udg {

/**
    Classe que encapsula el concepte de Window Level.
  */
class WindowLevel {

public:
    WindowLevel();
    WindowLevel(double width, double center, const QString &name = QString());
    ~WindowLevel();

    /// Define / get the window level description
    void setName(const QString &name);
    const QString& getName() const;
    
    /// Defines / obtains the width of the window level
    void setWidth(double width);
    double getWidth() const;
    
    /// Defines / obtains the center of the window level
    void setCenter(double center);
    double getCenter() const;

    /// Determines whether the width and level values are valid.
    /// A WindowLevel will always be valid unless the width is 0
    bool isValid() const;

    /// Compare if the window width and window level values are the same as the WindowLevel passed by parameter.
    /// The description is not taken into account in this comparison
    bool valuesAreEqual(const WindowLevel &windowLevel) const;

    /// Compare that all members of the class are equal
    bool operator==(const WindowLevel &windowLevelToCompare) const;

private:
    QString m_name;
    double m_width;
    double m_center;
};

}

#endif
