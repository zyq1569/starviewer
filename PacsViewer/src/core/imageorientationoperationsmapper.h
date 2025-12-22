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

#include <QString>
#include <QMap>

#include "patientorientation.h"

namespace udg {

/**
This class is in charge of telling us which rotation and flip operations we need to apply
on a 2D image given the initial orientation and the desired orientation.

To do this we will indicate the initial and desired orientation through the setInitialOrientation () methods
and setDesiredOrientation (). Once the guidelines have been assigned, we will ask for the necessary operations
apply on the 2D image to get the desired orientation.

The order in which the indicated operations are applied * is unique * and must be strictly as follows:
1. Apply rotations
2. Apply flips

This class only contemplates possible operations within the same 2D plan, so it cannot give us
a formula that passes an acquired image in axial to sagittal, e.g.
In these cases, getNumberOfClockwiseTurnsToApply () and requiresHorizontalFlip () will return 0 and false respectively.
*/
class ImageOrientationOperationsMapper {
public:
    ImageOrientationOperationsMapper();
    ~ImageOrientationOperationsMapper();

    /// Indicates initial orientation
    void setInitialOrientation(const PatientOrientation &initialOrientation);

    /// Indicates the desired orientation
    void setDesiredOrientation(const PatientOrientation &desiredOrientation);

    /// Returns the number of turns to be applied clockwise
    /// Possible values are 0, 1, 2 or 3
    int getNumberOfClockwiseTurnsToApply();

    /// Tells us whether to apply a horizontal flip or not
    bool requiresHorizontalFlip();

private:
    /// Initialize the values of the table where we will map the operations
    /// to be performed according to the possible orientations
    void initializeOrientationTable();

    ///Update the operations to be performed according to the guidelines entered
    void updateOperations();

private:
    /// Initial and desired orientations
    PatientOrientation m_initialOrientation;
    PatientOrientation m_desiredOrientation;

    ///Name of girs to apply
    int m_clockwiseTurns;

    /// Indicates whether to flip or not
    bool m_horizontalFlip;

    /// It will tell us if we need to find the necessary rotations and flips for the current combination of orientations
    bool m_hasToUpdateOperations;

    /// QMap to save the corresponding rotation and flip operations according to the given orientations.
    /// The key is a QString in which we will save the initial and desired orientation in the following format:
    /// "initialRowDirectionLabel \\ initialColumnDirectionLabel-desiredRowDirectionLabel \\ desiredColumnDirectionLabel"
    /// The value of each key will be a QString with the number of rotations followed by a value that indicates
    /// whether to flip or not, separated by a comma, such as "3, false"
    QMap<QString, QString> m_orientationMappingTable;
};

} // End namespace udg
