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

#ifndef UDGORDERIMAGESFILLERSTEP_H
#define UDGORDERIMAGESFILLERSTEP_H

#include "patientfillerstep.h"
#include <QMap>
#include <QHash>
#include <QString>
#include <QVector3D>

namespace udg {

class Patient;
class Series;
class Image;

/**
Module that is responsible for correctly sorting the images of the series. One of its requirements is to have the DICOMClassified label,
the ImageFillerStep and the TemporalDimensionFillerStep.
*/
class OrderImagesFillerStep : public PatientFillerStep {
public:
    OrderImagesFillerStep();

    ~OrderImagesFillerStep();

    bool fillIndividually();

    void postProcessing();

private:
    /// Methods for processing series-specific information
    void processImage(Image *image);

    /// Method for calculating how many phases per position each image actually has within each series and subvolume.
    void processPhasesPerPositionEvaluation(Image *image);
    ///Method that transforms the structure of ordered images into a list and inserts it into the series
    void setOrderedImagesIntoSeries(Series *series);

    /// Method that transforms the structure of ordered images into a list and inserts it into the series.
    QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*> *m_orderedNormalsSet;

    // Angle NormalVector Distance InstanceNumber0FrameNumber
    QHash<Series*, QMap<int, QMap<double, QMap<QString, QMap<double, QMap<unsigned long, Image*>*>*>*>*>*> m_orderImagesInternalInfo;

    QHash<Series*, QHash<int, QPair<QString, bool>*> > m_acquisitionNumberEvaluation;

    QVector3D m_firstPlaneVector3D;
    QVector3D m_direction;

    /// Type to define a hash to count the phases corresponding to each position
    /// The hash key is a string with the position of the image (ImagePositionPatient)
    /// and the associated value counts the occurrences (phases) of this position.
    /// If we have the same number of phases in all positions, we can say that it is a volume with phases
    typedef QHash<QString, int> PhasesPerPositionHashType;

    /// Hash in which for each series, we map a hash that indicates for each volume number, how many phases by position it has
    /// This will help us in the processed post to decide which subvolumes should be sorted by instance number
    /// in case not all the images of a same subvolume do not have the same number of phases
    /// <Series, <VolumeNumber, <PhasesPerPositionHash>>>
    QHash<Series*, QHash<int, PhasesPerPositionHashType*>*> m_phasesPerPositionEvaluation;

    /// Hash with which for each series we map a hash where it is indicated by each number
    /// of subvolume, if all the positions have the same number of phases.
    /// If not, the corresponding subvolume must be sorted by instance number as in the case of m_acquisitionNumberEvaluation
    /// EVERYTHING Maybe with this evaluation it would be enough and the one done by Acquisition Number could be eliminated
    /// <Series, <VolumeNumber, SameNumberOfPhasesPerPosition?>>
    QHash<Series*, QHash<int, bool>*> m_sameNumberOfPhasesPerPositionPerVolumeInSeriesHash;
};

}

#endif
