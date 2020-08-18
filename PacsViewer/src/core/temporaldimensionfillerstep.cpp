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

#include "temporaldimensionfillerstep.h"
#include "logging.h"
#include "patientfillerinput.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include <QStringList>

namespace udg {

TemporalDimensionFillerStep::TemporalDimensionFillerStep()
: PatientFillerStep()
{
}

TemporalDimensionFillerStep::~TemporalDimensionFillerStep()
{
    VolumeInfo *volumeInfo;
    QHash<int, VolumeInfo*> *volumeHash;

    foreach (Series *key, TemporalDimensionInternalInfo.keys())
    {
        volumeHash = TemporalDimensionInternalInfo.take(key);

        foreach (int volumeNumber, volumeHash->keys())
        {
            volumeInfo = volumeHash->take(volumeNumber);
            delete volumeInfo;
        }

        delete volumeHash;
    }
}

bool TemporalDimensionFillerStep::fillIndividually()
{
    VolumeInfo *volumeInfo;
    bool volumeInfoInitialized = false;

    QString acquisitionNumber;
    if (!m_input->getCurrentImages().isEmpty())
    {
        acquisitionNumber = m_input->getCurrentImages().first()->getAcquisitionNumber();
    }

    // Obtenim el VolumeInfo. Si no existeix en generem un de nou i l'afegim a l'estructura.
    if (TemporalDimensionInternalInfo.contains(m_input->getCurrentSeries()))
    {
        QHash<int, VolumeInfo*> *volumeHash = TemporalDimensionInternalInfo.value(m_input->getCurrentSeries());

        if (volumeHash->contains(m_input->getCurrentVolumeNumber()))
        {
            // Ja tenim el un VolumeInfo existent pel número de volum que estem processant actualment
            volumeInfoInitialized = true;
            volumeInfo = volumeHash->value(m_input->getCurrentVolumeNumber());

            if (!volumeInfo->multipleAcquisitionNumber)
            {
                if (volumeInfo->firstAcquisitionNumber != acquisitionNumber)
                {
                    volumeInfo->multipleAcquisitionNumber = true;
                }
            }
        }
        else
        {
            // El número de volum actual és nou, per tant cal crear un nou VolumeInfo associat a aquest nou volum
            volumeInfo = new VolumeInfo;
            volumeHash->insert(m_input->getCurrentVolumeNumber(), volumeInfo);
        }
    }
    else
    {
        QHash<int, VolumeInfo*> *volumeHash = new QHash<int, VolumeInfo*>();
        volumeInfo = new VolumeInfo;
        volumeHash->insert(m_input->getCurrentVolumeNumber(), volumeInfo);
        TemporalDimensionInternalInfo.insert(m_input->getCurrentSeries(), volumeHash);
    }

    // Si el VolumeInfo és nou, l'inicialitzem.
    if (!volumeInfoInitialized)
    {
        volumeInfo->numberOfPhases = 1;
        volumeInfo->numberOfImages = 0;
        volumeInfo->isCTLocalizer = false;
        volumeInfo->firstImagePosition = "";
        volumeInfo->firstAcquisitionNumber = acquisitionNumber;
        volumeInfo->multipleAcquisitionNumber = false;

        // En el cas del CT ens interessa saber si és localizer
        if (m_input->getCurrentSeries()->isCTLocalizer())
        {
            DEBUG_LOG("La serie amb uid " + m_input->getCurrentSeries()->getInstanceUID() + " no és dinàmica (És un CT LOCALIZER)");
            volumeInfo->isCTLocalizer = true;
        }
    }

    // Si és CTLocalizer no cal recorre totes les imatges ja que només ens interessa saber quantes n'hem d'afegir al VolumeInfo.
    if (volumeInfo->isCTLocalizer)
    {
        volumeInfo->numberOfImages += m_input->getCurrentImages().count();
    }
    else
    {
        foreach (Image *image, m_input->getCurrentImages())
        {
            const double *imagePositionPatient = image->getImagePositionPatient();

            if (!(imagePositionPatient[0] == 0. && imagePositionPatient[1] == 0. && imagePositionPatient[2] == 0.))
            {
                QString imagePositionPatientString = QString("%1\\%2\\%3").arg(imagePositionPatient[0])
                                                                          .arg(imagePositionPatient[1])
                                                                          .arg(imagePositionPatient[2]);

                if (volumeInfo->firstImagePosition.isEmpty())
                {
                    volumeInfo->firstImagePosition = imagePositionPatientString;
                }
                else
                {
                    if (volumeInfo->firstImagePosition == imagePositionPatientString)
                    {
                        volumeInfo->numberOfPhases++;
                    }
                }

                if (volumeInfo->phasesPerPositionHash.contains(imagePositionPatientString))
                {
                    // Ja el tenim, augmentem el nombre de fases per aquella posició
                    volumeInfo->phasesPerPositionHash.insert(imagePositionPatientString, volumeInfo->phasesPerPositionHash.value(imagePositionPatientString) + 1);
                }
                else
                {
                    // Creem la nova entrada, inicialment seria la primera fase
                    volumeInfo->phasesPerPositionHash.insert(imagePositionPatientString, 1);
                }
            }

            volumeInfo->numberOfImages++;
        }
    }

    return true;
}

void TemporalDimensionFillerStep::postProcessing()
{
    int currentVolume = -1;
    int currentPhase;
    int numberOfPhases;

    foreach (Series *key, TemporalDimensionInternalInfo.keys())
    {
        QHash<int, VolumeInfo*> *volumeHash = TemporalDimensionInternalInfo.take(key);

        foreach (Image *image, key->getImages())
        {
            if (currentVolume != image->getVolumeNumberInSeries())
            {
                currentVolume = image->getVolumeNumberInSeries();
                if (volumeHash->contains(currentVolume))
                {
                    VolumeInfo *volumeInfo = volumeHash->take(currentVolume);

                    if (volumeInfo->multipleAcquisitionNumber)
                    {
                        numberOfPhases = 1;
                        DEBUG_LOG(QString("No totes les imatges tenen el mateix AcquisitionNumber. Considerem que el volume %1 de la sèrie %2 no és dinàmic.")
                                     .arg(currentVolume).arg(key->getInstanceUID()));
                        INFO_LOG(QString("No totes les imatges tenen el mateix AcquisitionNumber. Considerem que el volume %1 de la sèrie %2 no és dinàmic.")
                                    .arg(currentVolume).arg(key->getInstanceUID()));
                    }
                    else
                    {
                        // Inicialment donem per fet que el càlcul de fases és correcte
                        numberOfPhases = volumeInfo->numberOfPhases;
                        // Si passem la llista de valors a un QSet i aquest té mida 1, vol dir que totes les posicions tenen el mateix nombre de fases
                        // (Un QSet no admet duplicats). S'ha de tenir en compte que si només hi ha una posició amb diferents fases no cal fer res ja que serà correcte
                        QList<int> phasesList = volumeInfo->phasesPerPositionHash.values();
                        int listSize = phasesList.count();
                        if (listSize > 1 && phasesList.toSet().count() > 1)
                        {
                            numberOfPhases = 1;
                        }
                        // TODO Si el càlcul no ha sigut correcte, caldria dividir en volums?
                    }
                    // L'esborrem perquè ja no el necessitarem més
                    if (volumeInfo)
                    {
                        delete volumeInfo;
                    }
                }
                else
                {
                    numberOfPhases = 1;

                    ERROR_LOG(QString("El volume %1 de la sèrie %2 no ha estat processat! Considerem que no és dinàmic")
                                 .arg(currentVolume).arg(key->getInstanceUID()));
                    DEBUG_LOG(QString("El volume %1 de la sèrie %2 no ha estat processat! Considerem que no és dinàmic")
                                 .arg(currentVolume).arg(key->getInstanceUID()));
                }

                currentPhase = 0;

                if (numberOfPhases > 1)
                {
                    DEBUG_LOG(QString("El volume %1 de la serie %2 és dinamic").arg(currentVolume).arg(key->getInstanceUID()));
                }

            }

            image->setPhaseNumber(currentPhase);

            currentPhase++;
            if (currentPhase == numberOfPhases)
            {
                currentPhase = 0;
            }
        }

        currentVolume = -1;

        delete volumeHash;
    }
}

}
