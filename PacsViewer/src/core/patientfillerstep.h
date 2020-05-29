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

#ifndef UDGPATIENTFILLERSTEP_H
#define UDGPATIENTFILLERSTEP_H

#include <QStringList>

namespace udg {

class PatientFillerInput;
class Series;

/**
    Classe pare dels mòduls que omplen parts específiques de l'estructura Patient
  */
class PatientFillerStep {
public:
    /// Flags que defineixen les diferents prioritats que poden tenir els Steps
    enum PriorityFlags{ LowPriority = 2, NormalPriority = 1, HighPriority = 0 };

    PatientFillerStep();

    virtual ~PatientFillerStep();

    /// Li assignem les dades d'entrada que li caldrà processar. Aquestes dades seran proporcionades per una classe externa.
    /// Es presuposa que aquest input mai serà NUL.
    void setInput(PatientFillerInput *input);

    /// Retorna la llista d'etiquetes que s'han de complir per poder processar aquest step.
    QStringList getRequiredLabels() const
    {
        return m_requiredLabelsList;
    }

    /// Retorna la prioritat que té assignada
    PriorityFlags getPriority() const
    {
        return m_priority;
    }

    /// Donat l'input, omple la part de l'estructura Patient que li pertoca a l'step. Si no és capaç de tractar el
    /// que li toca retorna fals, true altrament. S'ha d'utilitzar passant els steps individualment fitxer a fitxer.
    virtual bool fillIndividually() = 0;

    /// Al executar els steps individualment per cada fitxer, és possible que alguns steps necessitin fer quelcom
    /// un cop s'han processat tots els fitxers.
    virtual void postProcessing() = 0;

    /// Retorna el nom del Filler Step. Aquest no serà mai visible per l'usuari però ajuda a l'hora de debugar el sistema.
    virtual QString name() = 0;

    /// Operador que serveix per saber, de dos PatientFillerSteps, quin és el que té una prioritat més gran
    bool operator<(const PatientFillerStep &patientFillerStep) const;

protected:
    /// Mètodes de conveniència
    /// Ens diu si aquella sèrie és d'imatges, kin's o presentation states
    bool isImageSeries(Series *series);
    bool isKeyImageNoteSeries(Series *series);
    bool isPresentationStateSeries(Series *series);

protected:
    /// L'input a tractar
    PatientFillerInput *m_input;

    /// Llista d'etiquetes necessàries per poder processar aquest step.
    QStringList m_requiredLabelsList;

    /// Prioritat de l'step. Per defecte tindrem prioritat normal en tots els steps, excepte en aquells que es defineixi
    /// alguna prioritat diferent.
    PriorityFlags m_priority;

};

}

#endif
