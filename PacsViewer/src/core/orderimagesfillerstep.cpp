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

#include "orderimagesfillerstep.h"
#include "logging.h"
#include "patientfillerinput.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "mathtools.h"

namespace udg {

OrderImagesFillerStep::OrderImagesFillerStep()
    : PatientFillerStep()
{
}

OrderImagesFillerStep::~OrderImagesFillerStep()
{
    QMap<unsigned long, Image*> *instanceNumberSet;
    QMap<double, QMap<unsigned long, Image*>*> *imagePositionSet;
    QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*> *orderedImageSet;
    QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*> *normalVectorImageSet;
    QMap<int, QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*>*> *volumesInSeries;

    foreach (Series *key, m_orderImagesInternalInfo.keys())
    {
        volumesInSeries = m_orderImagesInternalInfo.take(key);
        foreach (int volumeNumber, volumesInSeries->keys())
        {
            normalVectorImageSet = volumesInSeries->take(volumeNumber);
            foreach (double normalVectorKey, normalVectorImageSet->keys())
            {
                orderedImageSet = normalVectorImageSet->take(normalVectorKey);
                foreach (const QString &key, orderedImageSet->keys())
                {
                    imagePositionSet = orderedImageSet->take(key);
                    foreach (double distanceKey, imagePositionSet->keys())
                    {
                        instanceNumberSet = imagePositionSet->take(distanceKey);
                        delete instanceNumberSet;
                    }
                    delete imagePositionSet;
                }
                delete orderedImageSet;
            }
            delete normalVectorImageSet;
        }
        delete volumesInSeries;
    }

    foreach (Series *series, m_phasesPerPositionEvaluation.keys())
    {
        QHash<int, PhasesPerPositionHashType*> *volumeHash = m_phasesPerPositionEvaluation.take(series);
        foreach (int volumeNumber, volumeHash->keys())
        {
            PhasesPerPositionHashType *phasesPerPositionHash = volumeHash->take(volumeNumber);
            delete phasesPerPositionHash;
        }
        delete volumeHash;
    }

    foreach (Series *series, m_sameNumberOfPhasesPerPositionPerVolumeInSeriesHash.keys())
    {
        QHash<int, bool> *volumeHash = m_sameNumberOfPhasesPerPositionPerVolumeInSeriesHash.take(series);
        delete volumeHash;
    }
    foreach (Series *series, m_acquisitionNumberEvaluation.keys())
    {
        QHash<int, QPair<QString, bool>*> volumeHash = m_acquisitionNumberEvaluation.take(series);
        foreach (int volumeNumber, volumeHash.keys())
        {
            QPair<QString, bool> *pair = volumeHash.take(volumeNumber);
            delete pair;
        }
    }
}

bool OrderImagesFillerStep::fillIndividually()
{
    QMap<int, QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*>*> *volumesInSeries;

    if (m_orderImagesInternalInfo.contains(m_input->getCurrentSeries()))
    {
        volumesInSeries = m_orderImagesInternalInfo.value(m_input->getCurrentSeries());
        if (volumesInSeries->contains(m_input->getCurrentVolumeNumber()))
        {
            m_orderedNormalsSet = volumesInSeries->value(m_input->getCurrentVolumeNumber());
        }
        else
        {
            DEBUG_LOG(QString("Null list for volume %1 of series %2. We create a new one.").arg(m_input->getCurrentVolumeNumber()).arg(
                          m_input->getCurrentSeries()->getInstanceUID()));
            m_orderedNormalsSet = new QMap<double,QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*>();
            volumesInSeries->insert(m_input->getCurrentVolumeNumber(), m_orderedNormalsSet);
        }
    }
    else
    {
        DEBUG_LOG(QString("Null list We create a new one for series%1.").arg(m_input->getCurrentSeries()->getInstanceUID()));
        volumesInSeries = new QMap<int, QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*>*>();
        m_orderedNormalsSet = new QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*>();

        volumesInSeries->insert(m_input->getCurrentVolumeNumber(), m_orderedNormalsSet);
        m_orderImagesInternalInfo.insert(m_input->getCurrentSeries(), volumesInSeries);
    }

    foreach (Image * image, m_input->getCurrentImages())
    {
        processImage(image);
        // Evaluation of the number of phases per position
        processPhasesPerPositionEvaluation(image);
    }

    // Evaluation of AcquisitionNumbers
    QString acquisitionNumber;
    if (!m_input->getCurrentImages().isEmpty())
    {
        acquisitionNumber = m_input->getCurrentImages().first()->getAcquisitionNumber();
    }
    if (m_acquisitionNumberEvaluation.contains(m_input->getCurrentSeries()))
    {
        if (m_acquisitionNumberEvaluation.value(m_input->getCurrentSeries()).contains(m_input->getCurrentVolumeNumber()))
        {
            // We compare Acquisition Numbers
            QPair<QString, bool> *pair = m_acquisitionNumberEvaluation[m_input->getCurrentSeries()][m_input->getCurrentVolumeNumber()];
            if (!pair->second)
            {
                if (pair->first != acquisitionNumber)
                {
                    pair->second = true;
                }
            }
        }
        else
        {
            QPair<QString, bool> *acquisitionPair = new QPair<QString, bool>(acquisitionNumber, false);
            m_acquisitionNumberEvaluation[m_input->getCurrentSeries()].insert(m_input->getCurrentVolumeNumber(), acquisitionPair);
        }
    }
    else
    {
        QHash<int, QPair<QString, bool>*> volumeHash;
        QPair<QString, bool> *acquisitionPair = new QPair<QString, bool>(acquisitionNumber, false);
        volumeHash.insert(m_input->getCurrentVolumeNumber(), acquisitionPair);

        m_acquisitionNumberEvaluation.insert(m_input->getCurrentSeries(), volumeHash);
    }

    return true;
}

void OrderImagesFillerStep::postProcessing()
{
    // For each subvolume of each series, we check whether these have the same number of phases per position
    foreach (Series *currentSeries, m_phasesPerPositionEvaluation.keys())
    {
        // <VolumeNumber, SameNumberOfPhasesPerPosition?>
        QHash<int, bool> *volumeNumberPhaseCountHash = new QHash<int, bool>();
        foreach (int currentVolumeNumber, m_phasesPerPositionEvaluation.value(currentSeries)->keys())
        {
            PhasesPerPositionHashType *phasesPerPositionHash = m_phasesPerPositionEvaluation.value(currentSeries)->value(currentVolumeNumber);
            /// If we pass the list of values to a QSet and this one has size 1,
            /// it means that all the positions have the same number of phases
            /// (A QSet does not support duplicates). It should be noted that if there
            /// is only one position with different phases there is no need to do anything as it will be correct
            QList<int> phasesList = phasesPerPositionHash->values();
            int listSize = phasesList.count();
            bool sameNumberOfPhases = true;
            if (listSize > 1 && phasesList.toSet().count() > 1)
            {
                sameNumberOfPhases = false;
            }
            volumeNumberPhaseCountHash->insert(currentVolumeNumber, sameNumberOfPhases);
        }
        m_sameNumberOfPhasesPerPositionPerVolumeInSeriesHash.insert(currentSeries, volumeNumberPhaseCountHash);
    }
    
    foreach (Series *key, m_orderImagesInternalInfo.keys())
    {
        setOrderedImagesIntoSeries(key);
    }
}

void OrderImagesFillerStep::processImage(Image *image)
{
    /// We obtain the normal vector of the plane, which also determines to which "stack" the image belongs
    QVector3D planeNormalVector3D = image->getImageOrientationPatient().getNormalVector();
    /// We pass it to string which will be easier for us to compare, because this is how it is saved in the sort structure
    QString planeNormalString = QString("%1\\%2\\%3").arg(planeNormalVector3D.x(), 0, 'f', 5).arg(planeNormalVector3D.y(), 0, 'f', 5)
            .arg(planeNormalVector3D.z(), 0, 'f', 5);

    QMap<double, QMap<unsigned long, Image*>*> *imagePositionSet;
    QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*> *orderedImageSet;
    QMap<unsigned long, Image*> *instanceNumberSet;

    double distance = Image::distance(image);

    /// First we look for the most similar key (normal of the plan) of all
    /// Each key is the normal of each plane saved as a string.
    /// In case we have different normals, I would indicate that we have for example, different stacks in the same volume

    /// TODO WARN BUG: We are doing insertMulti () in another place, so there may be keys with multiple values,
    /// but we are getting only the last value for each key, so some plane normals can be missed.
    /// We should iterate over the values instead of the unique keys and use a QSet for planeNormals.
    QStringList planeNormals;
    foreach (double key, m_orderedNormalsSet->uniqueKeys())
    {
        planeNormals << m_orderedNormalsSet->value(key)->uniqueKeys();
    }

    // This loop is used to find if the normal of the new image
    // matches any normal of the already processed images
    QString keyPlaneNormal;
    foreach (const QString &normal, planeNormals)
    {
        if (normal == planeNormalString)
        {
            /// The normal of this plan already exists (most typical case)
            keyPlaneNormal = normal;
            break;
        }
        /// The normal ones are different, check if they are completely or not
        else
        {
            if (normal.isEmpty())
            {
                keyPlaneNormal = planeNormalString;
                break;
            }
            else
            {
                /// Although they are different, they may be almost the same
                /// then you have to check that in fact they are quite different
                /// since sometimes there are just small inaccuracies simply
                QStringList normalSplitted = normal.split("\\");
                QVector3D normalVector(normalSplitted.at(0).toDouble(), normalSplitted.at(1).toDouble(), normalSplitted.at(2).toDouble());

                double angle = MathTools::angleInDegrees(normalVector, planeNormalVector3D);

                if (angle < 1.0)
                {
                    /// If the angle between the normals
                    /// is within a threshold,
                    /// we can consider them equal
                    /// EVERYTHING better define this threshold
                    keyPlaneNormal = normal;
                    break;
                }
            }
        }
    }
    /// Now you need to insert the image in the sorted list
    /// If we didn't put any value, it means that the normal one is new and didn't exist until now
    if (keyPlaneNormal.isEmpty())
    {
        ///We assign the key
        keyPlaneNormal = planeNormalString;
        ///Now you need to insert the new key
        instanceNumberSet = new QMap<unsigned long, Image*>();
        instanceNumberSet->insert(QString("%1%2%3").arg(image->getInstanceNumber()).arg("0").arg(image->getFrameNumber()).toULong(), image);

        imagePositionSet = new QMap<double, QMap<unsigned long, Image*>*>();
        imagePositionSet->insert(distance, instanceNumberSet);
        
        orderedImageSet = new QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>();
        orderedImageSet->insert(keyPlaneNormal, imagePositionSet);

        double angle = 0;

        if (m_orderedNormalsSet->isEmpty())
        {
            m_firstPlaneVector3D = planeNormalVector3D;
        }
        else
        {
            if (m_orderedNormalsSet->size() == 1) /// We look for the normal to know the direction by which to order
            {
                m_direction = QVector3D::crossProduct(m_firstPlaneVector3D, planeNormalVector3D);
                m_direction = QVector3D::crossProduct(m_direction, m_firstPlaneVector3D);
            }
            
            angle = MathTools::angleInRadians(m_firstPlaneVector3D, planeNormalVector3D);

            if (QVector3D::dotProduct(planeNormalVector3D, m_direction) <= 0) // Direcció d'ordenació
            {
                angle = 2 * MathTools::PiNumber - angle;
            }
        }
        m_orderedNormalsSet->insertMulti(angle, orderedImageSet);

    }
    ///Normal already existed [m_orderedImageSet-> contains (keyPlaneNormal) == true],
    /// so just update the structure La normal ja existia [m_orderedImageSet->contains(keyPlaneNormal) == true], per tant només cal actualitzar l'estructura
    else
    {
        /// TODO WARN BUG: We are doing insertMulti() in another place, so there may be keys with multiple values,
        ///                but we are getting only the last value for each key, so some plane normals can be missed.
        ///                We should iterate over the values instead of the unique keys.
        foreach (double key, m_orderedNormalsSet->keys())
        {
            if (m_orderedNormalsSet->value(key)->contains(keyPlaneNormal))
            {
                orderedImageSet = m_orderedNormalsSet->value(key);
            }
        }

        imagePositionSet = orderedImageSet->value(keyPlaneNormal);
        if (imagePositionSet->contains(distance))
        {
            /// There are series where the images share the same instance number.
            /// To avoid the problem an insertMulti is made.
            imagePositionSet->value(distance)->insertMulti(QString("%1%2%3").arg(image->getInstanceNumber()).arg("0")
                                                           .arg(image->getFrameNumber()).toULong(), image);
        }
        else
        {
            instanceNumberSet = new QMap<unsigned long, Image*>();
            instanceNumberSet->insert(QString("%1%2%3").arg(image->getInstanceNumber()).arg("0").arg(image->getFrameNumber()).toULong(), image);
            imagePositionSet->insert(distance, instanceNumberSet);
        }
    }
}

void OrderImagesFillerStep::processPhasesPerPositionEvaluation(Image *image)
{
    Series *currentSeries = m_input->getCurrentSeries();
    
    /// We check if we have hash for the current series and assign it if necessary
    QHash<int, PhasesPerPositionHashType*> *volumeNumberPhasesPerPositionHash = 0;
    if (m_phasesPerPositionEvaluation.contains(currentSeries))
    {
        volumeNumberPhasesPerPositionHash = m_phasesPerPositionEvaluation.value(currentSeries);
    }
    else
    {
        volumeNumberPhasesPerPositionHash = new QHash<int, PhasesPerPositionHashType*>();
        m_phasesPerPositionEvaluation.insert(currentSeries, volumeNumberPhasesPerPositionHash);
    }

    /// We now check if the second hash has the current volume number and assign the values to it if necessary
    int currentVolumeNumber = m_input->getCurrentVolumeNumber();
    PhasesPerPositionHashType *phasesPerPositionHash = 0;
    if (volumeNumberPhasesPerPositionHash->contains(currentVolumeNumber))
    {
        phasesPerPositionHash = volumeNumberPhasesPerPositionHash->value(currentVolumeNumber);
    }
    else
    {
        phasesPerPositionHash = new PhasesPerPositionHashType;
        volumeNumberPhasesPerPositionHash->insert(currentVolumeNumber, phasesPerPositionHash);
    }

    /// Now we have the hash of the phases by position corresponding to the current series and volume number
    /// We proceed to insert the corresponding information
    const double *imagePositionPatient = image->getImagePositionPatient();

    if (!(imagePositionPatient[0] == 0. && imagePositionPatient[1] == 0. && imagePositionPatient[2] == 0.))
    {
        QString imagePositionPatientString = QString("%1\\%2\\%3").arg(imagePositionPatient[0])
                .arg(imagePositionPatient[1])
                .arg(imagePositionPatient[2]);

        if (phasesPerPositionHash->contains(imagePositionPatientString))
        {
            /// We already have it, we increase the number of phases by that position
            phasesPerPositionHash->insert(imagePositionPatientString, phasesPerPositionHash->value(imagePositionPatientString) + 1);
        }
        else
        {
            /// We create the new entry, initially it would be the first phase
            phasesPerPositionHash->insert(imagePositionPatientString, 1);
        }
        /// TODO The above if in principle could be summarized in this single line
        /// m_phasesPerPositionHash.insert (imagePositionPatientString, m_phasesPerPositionHash.value (imagePositionPatientString, 0) + 1);
    }
}

void OrderImagesFillerStep::setOrderedImagesIntoSeries(Series *series)
{
    QList<Image*> imageSet;
    QMap<unsigned long, Image*> *instanceNumberSet;
    QMap<double, QMap<unsigned long, Image*>*> *imagePositionSet;
    QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*> *orderedImageSet;
    QMap<double, QMap<double, QMap<double, QMap<unsigned long, Image*>*>*>*> lastOrderedImageSetDistanceAngle;
    QMap<double, QMap<double, QMap<double, QMap<unsigned long, Image*>*>*>*> lastOrderedImageSetDistanceStack;
    QMap<double, QMap<double, QMap<unsigned long, Image*>*>*> *lastOrderedImageSetDistance;
    QMap<int, QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*>*> *volumesInSeries;

    Image *currentImage;
    int orderNumberInVolume;

    volumesInSeries = m_orderImagesInternalInfo.take(series);

    foreach (int currentVolumeNumber, volumesInSeries->keys())
    {
        bool orderByInstanceNumber = false;
        /// Different number of images per phase
        if (!m_sameNumberOfPhasesPerPositionPerVolumeInSeriesHash.value(series)->value(currentVolumeNumber))
        {
            orderByInstanceNumber = true;
            DEBUG_LOG(QString("Not all images have the same number of phases. We sort volume %1 of series %2 by Instance Number").arg(
                          currentVolumeNumber).arg(series->getInstanceUID()));
            INFO_LOG(QString("Not all images have the same number of phases. We sort volume %1 of series %2 by Instance Number").arg(
                         currentVolumeNumber).arg(series->getInstanceUID()));
        }
        /// Multiple acquisition number
        if (m_acquisitionNumberEvaluation[series][currentVolumeNumber]->second)
        {
            orderByInstanceNumber = true;
            DEBUG_LOG(QString("Not all images have the same AcquisitionNumber. We sort volume %1 of series %2 by Instance Number").arg(
                          currentVolumeNumber).arg(series->getInstanceUID()));
            INFO_LOG(QString("Not all images have the same AcquisitionNumber. We sort volume %1 of series %2 by Instance Number").arg(
                         currentVolumeNumber).arg(series->getInstanceUID()));
        }

        if (orderByInstanceNumber)
        {
            m_orderedNormalsSet = volumesInSeries->take(currentVolumeNumber);
            QMap<unsigned long, Image*> sortedImagesByInstanceNumber;

            foreach (double key, m_orderedNormalsSet->keys())
            {
                orderedImageSet = m_orderedNormalsSet->take(key);
                foreach (const QString &key, orderedImageSet->keys())
                {
                    imagePositionSet = orderedImageSet->take(key);
                    foreach (double key2, imagePositionSet->keys())
                    {
                        instanceNumberSet = imagePositionSet->take(key2);
                        foreach (unsigned long key3, instanceNumberSet->keys())
                        {
                            currentImage = instanceNumberSet->take(key3);
                            sortedImagesByInstanceNumber.insertMulti(key3, currentImage);
                        }
                    }
                }
            }

            orderNumberInVolume = 0;

            foreach (Image *image, sortedImagesByInstanceNumber.values())
            {
                image->setOrderNumberInVolume(orderNumberInVolume);
                image->setVolumeNumberInSeries(currentVolumeNumber);
                orderNumberInVolume++;

                imageSet += image;
            }
        }
        else
        {
            m_orderedNormalsSet = volumesInSeries->take(currentVolumeNumber);

            ///Image groupings need to be sorted
            foreach (double angle, m_orderedNormalsSet->keys())
            {
                orderedImageSet = m_orderedNormalsSet->take(angle);
                lastOrderedImageSetDistance = new QMap<double, QMap<double, QMap<unsigned long, Image*>*>*>();
                bool isRotational = true;

                foreach (const QString &normal, orderedImageSet->keys())
                {
                    imagePositionSet = orderedImageSet->take(normal);
                    
                    Image *firstImage = (*(*imagePositionSet->begin())->begin());
                    double distance = Image::distance(firstImage);
                    lastOrderedImageSetDistance->insertMulti(distance, imagePositionSet);

                    if (imagePositionSet->size() > 1)
                    {
                        int pos = (imagePositionSet->size())-1;
                        double key = imagePositionSet->keys().at(pos);
                        Image *lastImage = (*(imagePositionSet->value(key))->begin());
                        double distanceToFirstImage = qAbs(Image::distance(lastImage)-distance);
                        if (distanceToFirstImage > 1.0)
                        {
                            isRotational = false;
                        }
                    }
                }

                if (isRotational)//Rotacionals
                {
                    lastOrderedImageSetDistanceAngle.insertMulti(angle, lastOrderedImageSetDistance);
                }
                else //Stacks
                {
                    Image *image = (*(*imagePositionSet->begin())->begin());
                    lastOrderedImageSetDistanceStack.insertMulti(Image::distance(image), lastOrderedImageSetDistance);
                }
            }
            
            QList<QMap<double, QMap<double, QMap<unsigned long, Image*>*>*>*> orderedSet = lastOrderedImageSetDistanceStack.values();
            orderedSet.append(lastOrderedImageSetDistanceAngle.values());
            
            orderNumberInVolume = 0;

            for (int position = 0; position < orderedSet.size(); position++)
            {
                lastOrderedImageSetDistance = orderedSet.value(position);
                foreach (double key2, lastOrderedImageSetDistance->keys())
                {
                    imagePositionSet = lastOrderedImageSetDistance->take(key2);
                    foreach (double key3, imagePositionSet->keys())
                    {
                        instanceNumberSet = imagePositionSet->take(key3);
                        foreach (unsigned long key4, instanceNumberSet->keys())
                        {
                            currentImage = instanceNumberSet->take(key4);
                            currentImage->setOrderNumberInVolume(orderNumberInVolume);
                            currentImage->setVolumeNumberInSeries(currentVolumeNumber);
                            orderNumberInVolume++;

                            imageSet += currentImage;
                        }
                    }
                }
            }
        }
    }
    series->setImages(imageSet);
}

}
