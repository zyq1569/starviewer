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

#ifndef UDGPATIENTORIENTATION_H
#define UDGPATIENTORIENTATION_H

#include <QString>

class QVector3D;

namespace udg {

class ImageOrientation;

/**
    Classe que encapsularà l'atribut DICOM Patient Orientation (0020,0020). Per més informació consultar l'apartat C.7.6.1.1.1 (PS 3.3)
    En cas que aquest atribut s'assigni directament del contingut de l'arxiu DICOM constarà de dues cadenes de texte separades per \\
    que representen la direcció de les files i columnes de la imatge respecte al pacient. 
    En cas que l'atribut es calculi a partir de la classe ImageOrientation, a més a més li afegirim una tercera cadena que indicaria 
    la direcció en que s'apilen les imatges, útil per les reconstruccions.
  */
class PatientOrientation {
public:
    /// Etiquetes d'orientació
    /// Si Anatomical Orientation Type (0010,2210) és buit o té el valor de BIPED, 
    /// aquestes seran les abreviacions designades per les direccions anatòmiques
    /// Si Anatomical Orientation Type (0010,2210) té el valor de QUADRUPED, es farien servir altres abreviacions (animals)
    /// TODO De moment només tenim en compte les orientacions de tipus "BIPED". Consultar C.7.6.1.1.1 (PS 3.3) per a més informació
    static const QString LeftLabel;
    static const QString RightLabel;
    static const QString PosteriorLabel;
    static const QString AnteriorLabel;
    static const QString HeadLabel;
    static const QString FeetLabel;

    /// Assigna les etiquetes d'orientació per cada direcció: fila, columna, normal. L'etiqueta corresponent a la normal és opcional.
    /// En cas que les etiquetes proporcionades no siguin vàlides, retornarà fals, cert altrament.
    bool setLabels(const QString &rowDirectionLabel, const QString &columnDirectionLabel, const QString &normalDirectionLabel = QString());
    
    /// Assigna la cadena d'orientació en format DICOM. Si aquesta té alguna inconsistència, retornarà fals, cert altrament.
    /// S'accepten cadenes buides, amb 2 i 3 ítems separats per \\ i que continguin les etiquetes d'orientació estipulades pel DICOM
    bool setDICOMFormattedPatientOrientation(const QString &patientOrientation);
    
    /// Retornna la cadena de la orientació de pacient en format DICOM
    QString getDICOMFormattedPatientOrientation() const;
    
    /// Assigna la orientació de pacient calculada a partir de l'ImageOrientation
    void setPatientOrientationFromImageOrientation(const ImageOrientation &imageOrientation);
    
    /// Ens retorna l'etiqueta d'orientació de la direcció de la fila, columna o normal, en cas que aquesta existeixi.
    QString getRowDirectionLabel() const;
    QString getColumnDirectionLabel() const;
    QString getNormalDirectionLabel() const;
    
    /// Donada una etiqueta d'orientació, ens retorna aquesta etiqueta però amb els valors oposats.
    /// Per exemple, si l'etiqueta que ens donen és RPF (Right-Posterior,Feet), el valor retornat seria LAH (Left-Anterior-Head)
    /// Les etiquetes vàlides i els seus oposats són les següents:
    /// RightLabel:LeftLabel, AnteriorLabel:PosteriorLabel, HeadLabel:FeetLabel
    /// Si l'string donada no es correspon amb cap d'aquests valors, el valor transformat serà '?'
    static QString getOppositeOrientationLabel(const QString &label);

    /// Equality operator
    bool operator==(const PatientOrientation &orientation) const;

private:
    /// Ens valida una cadena d'orientació de pacient en format DICOM. Acceptarem cadenes amb 2 o 3 elements o buides.
    /// Si la cadena és correcta retorna cert, fals altrament
    bool validateDICOMFormattedPatientOrientationString(const QString &string);

    /// Donat un vector de direcció ens retorna l'etiqueta d'orientació corresponent
    QString getOrientationLabelFromDirectionVector(const QVector3D &vector);

    /// Retorna l'n-éssima etiqueta d'orientació (Fila = 0, Columna = 1, Normal = 2).
    /// Si donem un índex fora de rang pel nombre d'etiquetes que hi ha, retornarà un string buit.
    QString getNthDirectionLabel(int i) const;

private:
    /// La cadena d'orientació de pacient. Es guardarà com en el format DICOM, admetent que sigui buida o contingui 2 o 3 elements.
    QString m_patientOrientationString;
};

} // End namespace udg

#endif
