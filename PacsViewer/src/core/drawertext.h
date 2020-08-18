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

#ifndef UDGDRAWERTEXT_H
#define UDGDRAWERTEXT_H

#include "drawerprimitive.h"

class vtkTextActor;

namespace udg {

/**
    Primitiva de text per al Drawer
  */
class DrawerText : public DrawerPrimitive {
Q_OBJECT
public:
    DrawerText(QObject *parent = 0);
    ~DrawerText();

    vtkProp* getAsVtkProp();

    /// Permet assignar el text
    void setText(const QString &text);

    /// Ens retorna el text que conté
    QString getText();

    /// Permet canviar el punt d'ancoratge
    void setAttachmentPoint(double point[3]);

    /// Retorna el punt d'ancoratge
    double* getAttachmentPoint();

    /// Ens retorna el tipus de font
    QString getFontFamily();

    /// Ens permet assignar el tipus de font
    void setFontFamily(const QString &family);

    /// Assigna el tamany de la font
    void setFontSize(int size);

    /// Ens retona el tamany de la font
    int getFontSize();

    /// Activa la negreta del text
    void boldOn();

    /// Desactiva la negreta del text
    void boldOff();

    /// Activa / desactiva la negreta del text
    void bold(bool bold);

    /// Ens diu si està activada la negreta del text
    bool isBold();

    /// Activa la cursiva del text
    void italicOn();

    /// Desactiva la cursiva del text
    void italicOff();

    /// Activa / desactiva la cursiva del text
    void italic(bool italic);

    /// Ens diu si està activada la cursiva del text
    bool isItalic();

    /// Activa l'ombra del text
    void shadowOn();

    /// Desactiva l'ombra del text
    void shadowOff();

    /// Activa / desactiva l'ombra del text
    void setShadow(bool shadow);

    /// Ens diu si està activada l'ombra del text
    bool hasShadow();

    /// Ens permet assignar el tipus de justificació horitzontal
    void setHorizontalJustification(const QString &horizontalJustification);

    /// Ens retorna el tipus de justificació horitzontal
    QString getHorizontalJustification();

    /// Ens permet assignar el tipus de justificació vertical
    void setVerticalJustification(const QString &verticalJustification);

    /// Ens retorna el tipus de justificació vertical
    QString getVerticalJustification();

    /// Fa que el text s'escali
    void scaledTextOn();

    /// Fa que el text no s'escali
    void scaledTextOff();

    /// Ens permet determinar si el text s'escala o no
    void scaledText(bool scaled);

    /// Ens diu si el text és escalat o no
    bool isTextScaled();

    /// Calcula la distància que té respecte al punt passat per paràmetre
    double getDistanceToPoint(double *point3D, double closestPoint[3]);

    /// Ens retorna els límits de l'hexahedre que encapsula la primitiva
    /// en aquest ordre: minX, maxX, minY, maxY, minZ, maxZ
    virtual void getBounds(double bounds[6]);

    /// Assigna/Obté el color del fons de l'objecte
    void setBackgroundColor(QColor color);
    QColor getBackgroundColor() const;

    /// Assigna/Obté l'opacitat del fons de l'objecte
    void setBackgroundOpacity(double opacity);
    double getBackgroundOpacity() const;

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    /// Mètode intern per transformar les propietats de la primitiva a propietats de vtk
    void updateVtkActorProperties();

    /// Retorna cert si el punt està dins de la bounding box del text
    bool isInside(const double *point3D);

private:
    /// Text a mostrar
    QString m_text;

    /// Justificació horitzontal
    QString m_horizontalJustification;

    /// Justificació vertical
    QString m_verticalJustification;

    /// Família de la font
    QString m_fontFamily;

    /// Mida de la font
    int m_fontSize;

    /// Ombra
    bool m_shadow;

    /// Lletres en cursiva
    bool m_italic;

    /// Lletres en negreta
    bool m_bold;

    /// Alçada
    double m_height;

    /// Amplada
    double m_width;

    /// Text escalat
    bool m_scaled;

    /// Estructures de vtk, per construir el text
    vtkTextActor *m_vtkActor;

    /// Punt on es mostrarà el text
    double m_attachPoint[3];

    /// Color de fons de la imatge
    QColor m_backgroundColor;

    /// Opacitat del fons de l'objecte, per defecte 1.0, és a dir, completament opac
    double m_backgroundOpacity;
};

}

#endif
