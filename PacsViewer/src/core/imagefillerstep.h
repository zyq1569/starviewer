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

#ifndef UDGIMAGEFILLERSTEP_H
#define UDGIMAGEFILLERSTEP_H

#include "patientfillerstep.h"

#include "dicomtag.h"

namespace udg {

class DICOMSequenceItem;
class DICOMTagReader;
class Image;

/**
 * @brief The ImageFillerStep class creates and fills Image objects from DICOM files.
 */
class ImageFillerStep : public PatientFillerStep {

public:
    ImageFillerStep();
    virtual ~ImageFillerStep();

    virtual bool fillIndividually() override;

private:
    /// Method for processing patient-specific information, series and image
    void processImage(Image *image, const DICOMTagReader *dicomReader);

    /// Method responsible for processing the DICOM file to extract the set of images
    /// that make it up, filling in the necessary information
    QList<Image*> processDICOMFile(const DICOMTagReader *dicomReader);

    /// Specific method for processing files that are of the Enhanced type
    QList<Image*> processEnhancedDICOMFile(const DICOMTagReader *dicomReader);

    /// Fill in the information common to all images.
    /// Image and dicomReader must be valid objects.
    void fillCommonImageInformation(Image *image, const DICOMTagReader *dicomReader);

    /// Fill in the given image with the information of the functional groups contained in the item provided
    /// This method is intended to be used with the items obtained
    /// with both the Shared Functional Groups Sequence and the Per-Frame Functional Groups Sequence
    void fillFunctionalGroupsInformation(Image *image, DICOMSequenceItem *frameItem);

    ///Returns how many overlays are in the provided dataset
    unsigned short getNumberOfOverlays(const DICOMTagReader *dicomReader);
    
    /// Calculates the pixel spacing and assigns it to the given image in case it can be calculated
    /// @param image Image to which we will assign pixel spacing
    /// @param dicomReader Reader from DICOM that contains the data source of the associated Image
    void computePixelSpacing(Image *image, const DICOMTagReader *dicomReader);

    /// Checks and sets the Estimated Radiographic Magnification Factor tag for the corresponding modalities
    void checkAndSetEstimatedRadiographicMagnificationFactor(Image *image, const DICOMTagReader *dicomReader);

    ///It tells us if the SOP Class UID corresponds to that of an enhanced image
    bool isEnhancedImageSOPClass(const QString &sopClassUID);

    /// Validates the spacing string and sets it to the given image if it's well formatted.
    /// If no pixel spacing tag is specified, Pixel Spacing will be used by default
    /// PixelSpacing and ImagerPixelSpacing tags are the only tags supported currently
    /// Returns true on success, false otherwise
    void validateAndSetSpacingAttribute(Image *image, const QString &spacing, const DICOMTag &tag = DICOMTag(0x0028, 0x0030));

};

}

#endif
