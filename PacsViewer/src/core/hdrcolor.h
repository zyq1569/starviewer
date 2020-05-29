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

#ifndef UDGHDRCOLOR_H
#define UDGHDRCOLOR_H

#include <QColor>

namespace udg {

/**
    Color amb les components representades en coma flotant.
  */
class HdrColor {

public:
    /// Crea un color (0, 0, 0, 0).
    HdrColor();
    /// Crea un color amb els paràmetres desitjats.
    HdrColor(float r, float g, float b, float a = 1.0);
    /// Crea un color a partir d'un QColor.
    HdrColor(const QColor &color);
    /// Destructor.
    ~HdrColor();

    /// Cert si red = green = blue = 0.
    bool isBlack() const;
    /// Cert si alpha = 0.
    bool isTransparent() const;
    /// Multiplica les components de color per \a f i retorna el color.
    HdrColor& multiplyColorBy(float f);
    /// Posa a 1 les components més grans que 1 i a 0 les més petites que 0.
    HdrColor& clamp();

    /// Suma component a component.
    HdrColor operator +(const HdrColor &c) const;
    /// Suma component a component.
    HdrColor& operator +=(const HdrColor &c);
    /// Producte component a component.
    HdrColor operator *(const HdrColor &c) const;
    /// Producte component a component.
    HdrColor& operator *=(const HdrColor &c);
    /// Producte de totes les components per un real.
    HdrColor operator *(float f) const;
    /// Producte de totes les components per un real.
    HdrColor& operator *=(float f);

    /// Producte de totes les components per un real.
    friend HdrColor operator *(float f, const HdrColor &c);

    /// Retorna una string representativa del color.
    QString toString() const;

    float red, green, blue, alpha;

};

inline HdrColor::HdrColor()
{
    red = green = blue = alpha = 0.0;
}

inline HdrColor::HdrColor(float r, float g, float b, float a)
{
    red = r;
    green = g;
    blue = b;
    alpha = a;
}

inline HdrColor::HdrColor(const QColor &color)
{
    red = color.redF();
    green = color.greenF();
    blue = color.blueF();
    alpha = color.alphaF();
}

inline HdrColor::~HdrColor()
{
}

inline bool HdrColor::isBlack() const
{
    return red == 0.0 && green == 0.0 && blue == 0.0;
}

inline bool HdrColor::isTransparent() const
{
    return alpha == 0.0;
}

inline HdrColor& HdrColor::multiplyColorBy(float f)
{
    red *= f;
    green *= f;
    blue *= f;
    return *this;
}

inline HdrColor& HdrColor::clamp()
{
    red = qBound(0.0f, red, 1.0f);
    green = qBound(0.0f, green, 1.0f);
    blue = qBound(0.0f, blue, 1.0f);
    alpha = qBound(0.0f, alpha, 1.0f);
    return *this;
}

inline HdrColor HdrColor::operator +(const HdrColor &c) const
{
    return HdrColor(red + c.red, green + c.green, blue + c.blue, alpha + c.alpha);
}

inline HdrColor& HdrColor::operator +=(const HdrColor &c)
{
    red += c.red;
    green += c.green;
    blue += c.blue;
    alpha += c.alpha;
    return *this;
}

inline HdrColor HdrColor::operator *(const HdrColor &c) const
{
    return HdrColor(red * c.red, green * c.green, blue * c.blue, alpha * c.alpha);
}

inline HdrColor& HdrColor::operator *=(const HdrColor &c)
{
    red *= c.red;
    green *= c.green;
    blue *= c.blue;
    alpha *= c.alpha;
    return *this;
}

inline HdrColor HdrColor::operator *(float f) const
{
    return HdrColor(red * f, green * f, blue * f, alpha * f);
}

inline HdrColor& HdrColor::operator *=(float f)
{
    red *= f;
    green *= f;
    blue *= f;
    alpha *= f;
    return *this;
}

inline HdrColor operator *(float f, const HdrColor &c)
{
    return c * f;
}

inline QString HdrColor::toString() const
{
    return QString("(%1, %2, %3, %4)").arg(red).arg(green).arg(blue).arg(alpha);
}

}

#endif
