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

    ///We get the VolumeInfo. If it does not exist, we generate a new one and add it to the structure.
    if (TemporalDimensionInternalInfo.contains(m_input->getCurrentSeries()))
    {
        QHash<int, VolumeInfo*> *volumeHash = TemporalDimensionInternalInfo.value(m_input->getCurrentSeries());

        if (volumeHash->contains(m_input->getCurrentVolumeNumber()))
        {
            ///We already have the existing VolumeInfo for the volume number we are currently processing
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
            /// The current volume number is new, so you need to
            ///  create a new VolumeInfo associated with this new volume
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

    //If the VolumeInfo is new, we initialize it.
    if (!volumeInfoInitialized)
    {
        volumeInfo->numberOfPhases = 1;
        volumeInfo->numberOfImages = 0;
        volumeInfo->isCTLocalizer = false;
        volumeInfo->firstImagePosition = "";
        volumeInfo->firstAcquisitionNumber = acquisitionNumber;
        volumeInfo->multipleAcquisitionNumber = false;

        // In the case of CT we are interested to know if it is localizer
        if (m_input->getCurrentSeries()->isCTLocalizer())
        {
            DEBUG_LOG("The series with uid" + m_input->getCurrentSeries()->getInstanceUID() + " it is not dynamic (It is a CT LOCALIZER)");
            volumeInfo->isCTLocalizer = true;
        }
    }

    ///If it is CTLocalizer it is not necessary to go through
    /// all the images as we are only interested in knowing how many we need to add to the VolumeInfo.
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
                    //We already have it, we increase the number of phases by that position
                    volumeInfo->phasesPerPositionHash.insert(imagePositionPatientString, volumeInfo->phasesPerPositionHash.value(imagePositionPatientString) + 1);
                }
                else
                {
                    // We create the new entry, initially it would be the first phase
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
                        DEBUG_LOG(QString("Not all images have the same AcquisitionNumber. We consider that volume %1 of series %2 is not dynamic.")
                                  .arg(currentVolume).arg(key->getInstanceUID()));
                        INFO_LOG(QString("Not all images have the same AcquisitionNumber. We consider that volume %1 of series %2 is not dynamic.")
                                 .arg(currentVolume).arg(key->getInstanceUID()));
                    }
                    else
                    {
                        ///Initially we assume that the phase calculation is correct
                        numberOfPhases = volumeInfo->numberOfPhases;
                        /// If we pass the list of values to a QSet and this one has size 1,
                        ///it means that all the positions have the same number of phases
                        /// (A QSet does not support duplicates). It should be noted that
                        /// if there is only one position with different phases there is no need to do anything as it will be correct
                        QList<int> phasesList = volumeInfo->phasesPerPositionHash.values();
                        int listSize = phasesList.count();
                        if (listSize > 1 && phasesList.toSet().count() > 1)
                        {
                            numberOfPhases = 1;
                        }
                        // TODO Si el càlcul no ha sigut correcte, caldria dividir en volums?
                    }
                    //We will delete it because we will no longer need it
                    if (volumeInfo)
                    {
                        delete volumeInfo;
                    }
                }
                else
                {
                    numberOfPhases = 1;

                    ERROR_LOG(QString("Volume %1 of series %2 has not been processed! We consider it not dynamic")
                              .arg(currentVolume).arg(key->getInstanceUID()));
                    DEBUG_LOG(QString("Volume %1 of series %2 has not been processed! We consider it not dynamic")
                              .arg(currentVolume).arg(key->getInstanceUID()));
                }

                currentPhase = 0;

                if (numberOfPhases > 1)
                {
                    DEBUG_LOG(QString("Volume %1 in series %2 is dynamic").arg(currentVolume).arg(key->getInstanceUID()));
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
