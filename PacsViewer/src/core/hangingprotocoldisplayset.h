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

#ifndef UDGHANGINGPROTOCOLDISPLAYSET_H
#define UDGHANGINGPROTOCOLDISPLAYSET_H

#include "patientorientation.h"

class QRectF;

namespace udg {

class HangingProtocol;
class HangingProtocolImageSet;

class HangingProtocolDisplaySet {

public:
    HangingProtocolDisplaySet();

    ~HangingProtocolDisplaySet();

    /// Posar l'identificador
    void setIdentifier(int identifier);

    /// Posar la descripcio
    void setDescription(QString description);

    /// Assignar el seu image set
    void setImageSet(HangingProtocolImageSet *imageSet);

    /// Obtenir el seu image set
    HangingProtocolImageSet* getImageSet() const;

    /// Assignar el hanging protocol al que pertany
    void setHangingProtocol(HangingProtocol *hangingProtocol);

    /// Obtenir el hanging protocol al que pertany
    HangingProtocol* getHangingProtocol() const;

    /// Posar la posició del visualitzador
    void setPosition(QString position);

    /// Posar la posició del pacient
    void setPatientOrientation(const PatientOrientation &orientation);

    /// Posar la reconstruccio (axial, sagital, coronal)
    void setReconstruction(QString reconstruction);

    /// Posar la fase
    void setPhase(int phase);

    /// Obtenir l'identificador
    int getIdentifier() const;

    /// Obtenir la descripcio
    QString getDescription() const;

    /// Obtenir la posició del visualitzador
    QString getPosition() const;

    /// Return geometry of the display set as QRectF
    QRectF getGeometry() const;

    /// Obtenir la posició del pacient
    PatientOrientation getPatientOrientation() const;

    /// Obtenir la reconstruccio
    QString getReconstruction() const;

    /// Obtenir la fase
    int getPhase() const;

    /// Mètode per mostrar els valors
    void show();

    /// Posar la llesca a mostrar
    void setSlice(int sliceNumber);

    /// Obtenir la llesca a mostrar
    int getSlice();

    // Llesca per si tenim volums
    void setSliceModifiedForVolumes(int sliceNumber);

    // Obtenir la llesca per si tenim volums
    int getSliceModifiedForVolumes();

    /// Assigna el tipus d'icona per representar-lo
    void setIconType(QString iconType);

    /// Obté el tipus d'icona per representar-lo
    QString getIconType() const;

    /// Assigna cap a quin costat ha d'estar alineada la imatge
    void setAlignment(QString alignment);

    /// Obté el costat que s'ha d'alinear la imatge
    QString getAlignment() const;

    /// Assigna l'eina a activar al visualitzador
    void setToolActivation(QString toolActive);

    /// Obté l'eina a activar al visualitzador
    QString getToolActivation();

    // Posa el windowWidth
    void setWindowWidth(double windowWidth);

    // Obté el windowWidth
    double getWindowWidth();

    // Posa el windowCenter
    void setWindowCenter(double windowCenter);

    // Obté el windowCenter
    double getWindowCenter();

private:
    /// Identificador de la seqüència
    int m_identifier;

    /// Descripció
    QString m_description;

    /// Hanging Protocol al que pertany
    HangingProtocol *m_hangingProtocol;

    /// Punter al seu image set.
    HangingProtocolImageSet *m_imageSet;

    /// Posició a on es troba
    QString m_position;

    /// Orientacio del pacient
    PatientOrientation m_patientOrientation;

    /// Reconstruccio
    QString m_reconstruction;

    /// Fase
    int m_phase;

    /// Llesca
    int m_sliceNumber;

    /// Llesca per volums
    int m_sliceModifiedForVolumes;

    /// Indica el tipus d'icona per representar el hanging protocol
    QString m_iconType;

    /// Indica si la imatge ha d'estar alineada en algun costat
    QString m_alignment;

    /// Indica la tool a activar
    QString m_activateTool;

    // Indica el windowWidth
    double m_windowWidth;

    // Indica el windowCenter
    double m_windowCenter;
};

}

#endif
