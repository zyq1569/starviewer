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

#include "imagefillerstep.h"

#include "dicomformattedvaluesconverter.h"
#include "dicomsequenceattribute.h"
#include "dicomsequenceitem.h"
#include "dicomtagreader.h"
#include "dicomvalueattribute.h"
#include "image.h"
#include "patientfillerinput.h"
#include "series.h"

namespace udg {

namespace {

// Returns the number of frames in the given DICOM file.
int getNumberOfFrames(const DICOMTagReader *dicomReader)
{
    if (dicomReader->tagExists(DICOMNumberOfFrames))
    {
        return dicomReader->getValueAttributeAsQString(DICOMNumberOfFrames).toInt();
    }
    else
    {
        return 1;
    }
}

// Fills the Patient Orientation of the image from the given reader.
// Depends on image having Image Orientation Patient.
void fillPatientOrientation(Image *image, const DICOMTagReader *dicomReader)
{
    /// Patient guidance
    /// We get it from the DICOM tag if it exists,
    ///  otherwise we calculate it from ImageOrientation
    PatientOrientation patientOrientation;
    QString value = dicomReader->getValueAttributeAsQString(DICOMPatientOrientation);
    if (!value.isEmpty())
    {
        patientOrientation.setDICOMFormattedPatientOrientation(value);
    }
    else
    {
        patientOrientation.setPatientOrientationFromImageOrientation(image->getImageOrientationPatient());
    }
    image->setPatientOrientation(patientOrientation);
}

// Fills the VOI LUTs of the image from the given container.
template <class C>
void fillVoiLuts(Image *image, const C *voiLutsContainer)
{
    QList<VoiLut> voiLutList;

    /// Normal: Llegim els valors de window level, tipus 1C i les respectives descripcions de ww/wl si n'hi ha (tipus 3)
    ///         Aquests es troben a VOI LUT Module (C.11.2)
    ///         El mòdul és opcional a CR, CT, MR, NM, US,
    /// US MF, SC, XA, RF, RF IM i PET i és condicional a SC MF GB, SC MF GW, DX, MG i IO
    /// Enhanced:  Frame VOI LUT Macro (C.7.6.16.2.10)

    // Obtenim Window Center (1C normal, 1 enhanced)
    QString windowCenterString = voiLutsContainer->getValueAttributeAsQString(DICOMWindowCenter);
    // Obtenim Window Width (1C normal, 1 enhanced)
    QString windowWidthString = voiLutsContainer->getValueAttributeAsQString(DICOMWindowWidth);
    // Obtenim Window Explanations (3)
    QString windowLevelExplanationString = voiLutsContainer->getValueAttributeAsQString(DICOMWindowCenterWidthExplanation);
    QList<WindowLevel> windowLevelList =
            DICOMFormattedValuesConverter::parseWindowLevelValues(windowWidthString, windowCenterString, windowLevelExplanationString);
    foreach (const WindowLevel &windowLevel, windowLevelList)
    {
        voiLutList.append(windowLevel);
    }

    // Normal: Read VOI LUTs from the VOI LUT Sequence
    // Enhanced: Frame VOI LUT With LUT Macro (C.7.6.16.2.10b)
    if (voiLutsContainer->hasAttribute(DICOMVOILUTSequence))
    {
        // Type 1C
        QList<DICOMSequenceItem*> items = voiLutsContainer->getSequenceAttribute(DICOMVOILUTSequence)->getItems();

        foreach (DICOMSequenceItem *item, items)
        {
            if (!item->hasAttribute(DICOMLUTDescriptor) || !item->hasAttribute(DICOMLUTData))
            {
                DEBUG_LOG("Missing type 1 attributes in a VOI LUT sequence item");
                continue;
            }

            // Type 1
            QString lutDescriptor = item->getValueAttributeAsQString(DICOMLUTDescriptor);
            // Type 3
            QString lutExplanation = item->getValueAttributeAsQString(DICOMLUTExplanation);
            // Type 1
            QString lutData = item->getValueAttributeAsQString(DICOMLUTData);
            voiLutList.append(DICOMFormattedValuesConverter::parseVoiLut(lutDescriptor, lutExplanation, lutData));
        }
    }

    image->setVoiLutList(voiLutList);
}

// Fills the display shutters of the image from the given container.
template <class C>
void fillDisplayShutterInformation(Image *image, const C *displayShutterContainer)
{
    // Obtenim Shutter Shape (1)
    DICOMValueAttribute *dicomValue = displayShutterContainer->getValueAttribute(DICOMShutterShape);
    QString shutterShape;
    if (dicomValue)
    {
        shutterShape = dicomValue->getValueAsQString();
    }
    else
    {
        /// If there is no shutter shape value then it is
        /// we don't have shutters, no need to look at any more tags
        return;
    }

    // We get the presentation value (3)
    unsigned short presentationValue = 0;
    dicomValue = displayShutterContainer->getValueAttribute(DICOMShutterPresentationValue);
    if (dicomValue)
    {
        presentationValue = dicomValue->getValueAsQString().toUShort();
    }

    // According to the values of shutterShape, we will fill in the information of each shape of the shutter
    if (shutterShape.contains("RECTANGULAR"))
    {
        int minX, maxX, minY, maxY;
        //We get Shutter Left Vertical Edge, required when shutter shape contains RECTANGULAR
        dicomValue = displayShutterContainer->getValueAttribute(DICOMShutterLeftVerticalEdge);
        if (dicomValue)
        {
            minX = dicomValue->getValueAsInt();
        }
        else
        {
            ERROR_LOG("The tag Shutter Left Vertical Edge was not found in a file that is supposed to have it!");
        }

        // We get Shutter Right Vertical Edge, required when shutter shape contains RECTANGULAR
        dicomValue = displayShutterContainer->getValueAttribute(DICOMShutterRightVerticalEdge);
        if (dicomValue)
        {
            maxX = dicomValue->getValueAsInt();
        }
        else
        {
            ERROR_LOG("No s'ha trobat el tag Shutter Right Vertical Edge en un arxiu que se suposa que l'ha de tenir!");
        }

        //We get Shutter Upper Horizontal Edge, required when shutter shape contains RECTANGULAR
        dicomValue = displayShutterContainer->getValueAttribute(DICOMShutterUpperHorizontalEdge);
        if (dicomValue)
        {
            minY = dicomValue->getValueAsInt();
        }
        else
        {
            ERROR_LOG("No s'ha trobat el tag Shutter Upper Horizontal Edge en un arxiu que se suposa que l'ha de tenir!");
        }

        // We get Shutter Lower Horizontal Edge, required when shutter shape contains RECTANGULAR
        dicomValue = displayShutterContainer->getValueAttribute(DICOMShutterLowerHorizontalEdge);
        if (dicomValue)
        {
            maxY = dicomValue->getValueAsInt();
        }
        else
        {
            ERROR_LOG("Shutter Lower Horizontal Edge tag not found in file supposed to have it!");
        }

        DisplayShutter rectangularShutter;
        rectangularShutter.setShape(DisplayShutter::RectangularShape);
        rectangularShutter.setShutterValue(presentationValue);
        rectangularShutter.setPoints(QPoint(minX, minY), QPoint(maxX, maxY));

        image->addDisplayShutter(rectangularShutter);
    }

    if (shutterShape.contains("CIRCULAR"))
    {
        //We obtain Radius of Circular Shutter, required when shutter shape contains CIRCULAR
        int radius = 0;
        dicomValue = displayShutterContainer->getValueAttribute(DICOMRadiusOfCircularShutter);
        if (dicomValue)
        {
            radius = dicomValue->getValueAsInt();
        }
        else
        {
            ERROR_LOG("The Radius of Circular Shutter tag was not found in a file that is supposed to have it!");
        }

        // We get Center of Circular Shutter, required when shutter shape contains CIRCULAR
        QPoint centre;
        dicomValue = displayShutterContainer->getValueAttribute(DICOMCenterOfCircularShutter);
        if (dicomValue)
        {
            QStringList centreStringList = dicomValue->getValueAsQString().split("\\");
            if (centreStringList.count() != 2)
            {
                ERROR_LOG("The Center of Circular Shutter attribute is in an unexpected format: " + dicomValue->getValueAsQString());
            }
            else
            {
                // given as row and column
                centre.setX(centreStringList.at(1).toInt());
                centre.setY(centreStringList.at(0).toInt());
            }
        }
        else
        {
            ERROR_LOG("The Radius of Circular Shutter tag was not found in a file that is supposed to have it!");
        }

        DisplayShutter circularShutter;
        circularShutter.setShape(DisplayShutter::CircularShape);
        circularShutter.setShutterValue(presentationValue);
        circularShutter.setPoints(centre, radius);

        image->addDisplayShutter(circularShutter);
    }

    if (shutterShape.contains("POLYGONAL"))
    {
        QVector<QPoint> polygonVertices;

        // We obtain Vertices of the Polygonal Shutter, required when shutter shape contains POLYGONAL
        dicomValue = displayShutterContainer->getValueAttribute(DICOMVerticesOfThePolygonalShutter);
        if (dicomValue)
        {
            QStringList vertices = dicomValue->getValueAsQString().split("\\");
            if (MathTools::isOdd(vertices.count()))
            {
                ERROR_LOG("The Vertices of the Polygonal Shutter attribute is in an unexpected format: " + dicomValue->getValueAsQString());
            }
            else
            {
                for (int i = 0; i < vertices.count() - 1; i += 2 )
                {
                    // Pairs are defined as row and column
                    polygonVertices << QPoint(vertices.at(i + 1).toInt(), vertices.at(i).toInt());
                }
            }
        }
        else
        {
            ERROR_LOG("The tag Vertices of the Polygonal Shutter was not found in a file that is supposed to have it!");
        }

        DisplayShutter polygonalShutter;
        polygonalShutter.setShape(DisplayShutter::PolygonalShape);
        polygonalShutter.setShutterValue(presentationValue);
        polygonalShutter.setPoints(polygonVertices);

        image->addDisplayShutter(polygonalShutter);
    }
}

}

ImageFillerStep::ImageFillerStep()
{
}

ImageFillerStep::~ImageFillerStep()
{
}

bool ImageFillerStep::fillIndividually()
{
    Q_ASSERT(m_input);

    bool ok = false;

    const DICOMTagReader *dicomReader = m_input->getDICOMFile();

    if (dicomReader)
    {
        ok = true;
        QList<Image*> generatedImages = processDICOMFile(dicomReader);
        if (!generatedImages.isEmpty())
        {
            m_input->setCurrentImages(generatedImages);
        }
    }

    return ok;
}

QList<Image*> ImageFillerStep::processDICOMFile(const DICOMTagReader *dicomReader)
{
    QList<Image*> generatedImages;
    bool ok = dicomReader->tagExists(DICOMPixelData);
    if (ok)
    {
        // We check if the image is enhanced or not in order to call the most appropriate specific method
        if (isEnhancedImageSOPClass(dicomReader->getValueAttributeAsQString(DICOMSOPClassUID)))
        {
            generatedImages = processEnhancedDICOMFile(dicomReader);
        }
        else
        {
            int numberOfFrames = getNumberOfFrames(dicomReader);

            for (int frameNumber = 0; frameNumber < numberOfFrames; frameNumber++)
            {
                Image *image = new Image();
                image->setFrameNumber(frameNumber);
                processImage(image, dicomReader);

                // We will add the image to the list if it could be added to the corresponding series
                if (m_input->getCurrentSeries()->addImage(image))
                {
                    generatedImages << image;
                }
                else
                {
                    delete image;
                }
            }
        }
    }
    return generatedImages;
}

void ImageFillerStep::fillCommonImageInformation(Image *image, const DICOMTagReader *dicomReader)
{
    // The path where the disk image is located
    image->setPath(dicomReader->getFileName());

    //DICOMSource from which the image comes
    image->setDICOMSource(m_input->getDICOMSource());

    // C.12.1 SOP Common Module
    image->setSOPInstanceUID(dicomReader->getValueAttributeAsQString(DICOMSOPInstanceUID));
    image->setInstanceNumber(dicomReader->getValueAttributeAsQString(DICOMInstanceNumber));

    // C.7.6.3 Image Pixel Module
    image->setSamplesPerPixel(dicomReader->getValueAttributeAsQString(DICOMSamplesPerPixel).toInt());
    image->setPhotometricInterpretation(dicomReader->getValueAttributeAsQString(DICOMPhotometricInterpretation));
    image->setRows(dicomReader->getValueAttributeAsQString(DICOMRows).toInt());
    image->setColumns(dicomReader->getValueAttributeAsQString(DICOMColumns).toInt());
    image->setBitsAllocated(dicomReader->getValueAttributeAsQString(DICOMBitsAllocated).toInt());
    image->setBitsStored(dicomReader->getValueAttributeAsQString(DICOMBitsStored).toInt());
    image->setPixelRepresentation(dicomReader->getValueAttributeAsQString(DICOMPixelRepresentation).toInt());

    // C.7.6.1 General Image Module (present a totes les modalitats no enhanced, excepte 3D XA, 3D CF i OPT)
    // C.8.13.1 Enhanced MR Image Module
    // C.8.15.2 Enhanced CT Image Module
    // C.8.19.2 Enhanced XA/XRF Image Module
    image->setImageType(dicomReader->getValueAttributeAsQString(DICOMImageType));

    // We get the time the pixel was created / obtained
    // C.7.6.1 General Image Module (present in all non-enhanced modes except 3D XA, 3D CF and OPT)
    // C.7.6.16 Multi-Frame Functional Groups Module
    image->setImageTime(dicomReader->getValueAttributeAsQString(DICOMContentTime));

    image->setAcquisitionNumber(std::move(dicomReader->getValueAttributeAsQString(DICOMAcquisitionNumber)));

    // C.9 Overlays
    image->setNumberOfOverlays(getNumberOfOverlays(dicomReader));

    // Transfer Syntax UID
    image->setTransferSyntaxUID(dicomReader->getValueAttributeAsQString(DICOMTransferSyntaxUID));
}

void ImageFillerStep::processImage(Image *image, const DICOMTagReader *dicomReader)
{
    QString value;

    //We fill in the common information
    fillCommonImageInformation(image, dicomReader);

    //We calculate the pixel spacing
    computePixelSpacing(image, dicomReader);
    // Fill other information
    checkAndSetEstimatedRadiographicMagnificationFactor(image, dicomReader);

    // We calculate properties of the image plane

    // Image Plane Module Properties (C.7.6.2) (Required by CT, MR and PET)

    // We get Slice Thickness, type 2
    value = dicomReader->getValueAttributeAsQString(DICOMSliceThickness);
    if (!value.isEmpty())
    {
        image->setSliceThickness(value.toDouble());
    }

    // Obtenim Slice Location, tipus 3
    if (dicomReader->tagExists(DICOMSliceLocation))
    {
        image->setSliceLocation(dicomReader->getValueAttributeAsQString(DICOMSliceLocation));
    }

    // Nuclear medicine modality has to be treated separately because Image Orientation (Patient) and
    // Image Position (Patient) tags are located in the Detector Infomration Sequence.
    // This process is only performed when the Image Type(0008,0008), Value 3, is RECON TOMO or RECON GATED TOMO.
    if (dicomReader->getValueAttributeAsQString(DICOMModality) == "NM" && image->getImageType().contains("PRIMARY\\RECON")
            && dicomReader->tagExists(DICOMDetectorInformationSequence))
    {
        // When Image Type(0008,0008), Value 3, is RECON TOMO or RECON GATED TOMO,
        // then the Number of Detectors (0054,0021) shall be 1.
        DICOMSequenceItem *firstDetectorItem = dicomReader->getFirstSequenceItem(DICOMDetectorInformationSequence);

        if (firstDetectorItem)
        {
            QString imageOrientationString = firstDetectorItem->getValueAttribute(DICOMImageOrientationPatient)->getValueAsQString();

            if (!imageOrientationString.isEmpty())
            {
                ImageOrientation imageOrientation;
                imageOrientation.setDICOMFormattedImageOrientation(imageOrientationString);
                image->setImageOrientationPatient(imageOrientation);

                fillPatientOrientation(image, dicomReader);

                // Image Position (Patient) of each image has to be computed because only
                // the Image Position (Patient) of the first image is provided
                QString imagePositionPatientString = firstDetectorItem->getValueAttribute(DICOMImagePositionPatient)->getValueAsQString();
                double spacingBetweenSlices = dicomReader->getValueAttributeAsQString(DICOMSpacingBetweenSlices).toDouble();

                if (!imagePositionPatientString.isEmpty() && spacingBetweenSlices != 0.0)
                {
                    QStringList values = imagePositionPatientString.split("\\");
                    QVector3D firstFrameImagePositionPatient(values.at(0).toDouble(), values.at(1).toDouble(), values.at(2).toDouble());
                    QVector3D imagePositionPatient = firstFrameImagePositionPatient + image->getFrameNumber() * spacingBetweenSlices * imageOrientation.getNormalVector();
                    double position[3] = { imagePositionPatient.x(), imagePositionPatient.y(), imagePositionPatient.z() };
                    image->setImagePositionPatient(position);
                }
            }
        }
    }
    else
    {
        // Obtenim Image Position (Patient), tipus 1
        value = dicomReader->getValueAttributeAsQString(DICOMImagePositionPatient);
        if (!value.isEmpty())
        {
            QStringList list = value.split("\\");
            if (list.size() == 3)
            {
                double position[3] = { list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble() };
                image->setImagePositionPatient(position);
            }
        }

        // Obtenim Image Orientation (Patient), tipus 1
        if (dicomReader->tagExists(DICOMImageOrientationPatient))
        {
            value = dicomReader->getValueAttributeAsQString(DICOMImageOrientationPatient);
            ImageOrientation imageOrientation;
            imageOrientation.setDICOMFormattedImageOrientation(value);
            image->setImageOrientationPatient(imageOrientation);

            fillPatientOrientation(image, dicomReader);
        }
        else
        {
            // General Image Module (C.7.6.1)
            // Required in virtually all non-enhanced modes, it contains the Patient Orientation tag

            // Because we do not have ImageOrientationPatient we cannot generate Patient Orientation information
            // So let's look for the value of the PatientOrientation tag, type 2C
            value = dicomReader->getValueAttributeAsQString(DICOMPatientOrientation);

            PatientOrientation patientOrientation;
            patientOrientation.setDICOMFormattedPatientOrientation(value);
            image->setPatientOrientation(patientOrientation);
        }
    }
    // We get data from the Grayscale Pipeline

    // We get Rescale Slope and Rescale Intercept, type 1 / 1C according to the module
    value = dicomReader->getValueAttributeAsQString(DICOMRescaleSlope);
    if (value.toDouble() == 0)
    {
        image->setRescaleSlope(1.);
    }
    else
    {
        image->setRescaleSlope(value.toDouble());
    }

    image->setRescaleIntercept(dicomReader->getValueAttributeAsQString(DICOMRescaleIntercept).toDouble());

    fillVoiLuts(image, dicomReader);

    // Useful properties for hanging protocols
    value = dicomReader->getValueAttributeAsQString(DICOMImageLaterality);
    if (!value.isEmpty())
    {
        image->setImageLaterality(value.at(0));
    }
    // For now we will only use it for mammography, but it may be valid for other modalities
    // By definition, we should only have one item
    DICOMSequenceItem *viewCodeSequenceItem = dicomReader->getFirstSequenceItem(DICOMViewCodeSequence);
    if (viewCodeSequenceItem)
    {
        image->setViewCodeMeaning(viewCodeSequenceItem->getValueAttribute(DICOMCodeMeaning)->getValueAsQString());
    }

    // Just in case it is DX we will have this image level attribute
    image->setViewPosition(dicomReader->getValueAttributeAsQString(DICOMViewPosition));

    /// Display Shutter Module (C.7.6.11)
    /// Fill in the information regarding the Display Shutters
    /// which can be found in CR, XA, RF, DX, MG and IO mode images
    fillDisplayShutterInformation(image, dicomReader);

	//add 20141204
	value = dicomReader->getValueAttributeAsQString(DICOMKVP);
	if (!value.isEmpty())
	{
		image->setDICOMKVP(value);
	}
	value = dicomReader->getValueAttributeAsQString(DICOMXRayTubeCurrent);
	if (!value.isEmpty())
	{
		image->setXRayTubeCurrent(value);
	}
}

QList<Image*> ImageFillerStep::processEnhancedDICOMFile(const DICOMTagReader *dicomReader)
{
    QList<Image*> generatedImages;
    int numberOfFrames = getNumberOfFrames(dicomReader);

    for (int frameNumber = 0; frameNumber < numberOfFrames; frameNumber++)
    {
        Image *image = new Image();
        fillCommonImageInformation(image, dicomReader);
        //We assign the frame node and the volume node to which it belongs
        image->setFrameNumber(frameNumber);

        // Afegirem la imatge a la llista si aquesta s'ha pogut afegir a la corresponent sèrie
        if (m_input->getCurrentSeries()->addImage(image))
        {
            generatedImages << image;
        }
    }

    //We deal with the Shared Functional Groups Sequence
    DICOMSequenceAttribute *sharedFunctionalGroupsSequence = dicomReader->getSequenceAttribute(DICOMSharedFunctionalGroupsSequence);
    if (sharedFunctionalGroupsSequence)
    {
        // This sequence may contain an item or head
        QList<DICOMSequenceItem*> sharedItems = sharedFunctionalGroupsSequence->getItems();
        if (!sharedItems.isEmpty())
        {
            foreach (Image *image, generatedImages)
            {
                fillFunctionalGroupsInformation(image, sharedItems.first());
            }
        }
    }
    else
    {
        ERROR_LOG("We did not find the Shared Functional Groups Sequence in a DICOM file that is assumed to be Enhanced");
    }
    //We deal with the Per-Frame Functional Groups Sequence
    DICOMSequenceAttribute *perFrameFunctionalGroupsSequence = dicomReader->getSequenceAttribute(DICOMPerFrameFunctionalGroupsSequence);
    if (perFrameFunctionalGroupsSequence)
    {
        QList<DICOMSequenceItem*> perFrameItems = perFrameFunctionalGroupsSequence->getItems();
        int frameNumber = 0;
        foreach (DICOMSequenceItem *item, perFrameItems)
        {
            fillFunctionalGroupsInformation(generatedImages.at(frameNumber), item);
            frameNumber++;
        }
    }
    else
    {
        ERROR_LOG("We did not find the per-frame Functional Groups Sequence in a DICOM file that is assumed to be Enhanced");
    }

    return generatedImages;
}

void ImageFillerStep::fillFunctionalGroupsInformation(Image *image, DICOMSequenceItem *frameItem)
{
    // There are some attributes that we will have to look for in different places depending on the modality
    QString sopClassUID = m_input->getDICOMFile()->getValueAttributeAsQString(DICOMSOPClassUID);

    // Attributes of CT and MRi MG Breast Tomosynthesis
    if (sopClassUID == UIDEnhancedCTImageStorage || sopClassUID == UIDEnhancedMRImageStorage || sopClassUID == UIDBreastTomosynthesisImageStorage)
    {
        //To obtain the Frame Type, we must select the appropriate sequence, depending on the mode
        DICOMSequenceAttribute *imageFrameTypeSequence = 0;
        if (sopClassUID == UIDEnhancedCTImageStorage)
        {
            // CT Image Frame Type (C.8.15.3.1)
            imageFrameTypeSequence = frameItem->getSequenceAttribute(DICOMCTImageFrameTypeSequence);
        }
        else if (sopClassUID == UIDEnhancedMRImageStorage)
        {
            // MR Image Frame Type (C.8.13.5.1)
            imageFrameTypeSequence = frameItem->getSequenceAttribute(DICOMMRImageFrameTypeSequence);
        }
        else
        {
            // X-Ray 3D Frame Type Macro (C.8.21.5.1)
            imageFrameTypeSequence = frameItem->getSequenceAttribute(DICOMXRay3DFrameTypeSequence);
        }

        // Once the appropriate sequence is selected, we obtain the values
        if (imageFrameTypeSequence)
        {
            // According to DICOM it is only allowed to contain a single item
            QList<DICOMSequenceItem*> imageFrameTypeItems = imageFrameTypeSequence->getItems();
            if (!imageFrameTypeItems.empty())
            {
                DICOMSequenceItem *item = imageFrameTypeItems.at(0);
                // We get the Frame Type (1)
                DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMFrameType);
                if (dicomValue)
                {
                    image->setImageType(dicomValue->getValueAsQString());
                }
                else
                {
                    ERROR_LOG("Falta el tag FrameType que hauria d'estar present!");
                }
            }
        }

        // Pixel Measures Module - C.7.6.16.2.1
        // According to DICOM it is only allowed to contain a single item
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMPixelMeasuresSequence))
        {
            // We get the Pixel Spacing (1C)
            DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMPixelSpacing);
            if (dicomValue)
            {
                validateAndSetSpacingAttribute(image, dicomValue->getValueAsQString());
            }

            //We get the Slice Thickness (1C)
            dicomValue = item->getValueAttribute(DICOMSliceThickness);
            if (dicomValue)
            {
                image->setSliceThickness(dicomValue->getValueAsDouble());
            }
        }

        //Plane Orientation Module - C.7.6.16.2.4
        // According to DICOM it is only allowed to contain a single item
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMPlaneOrientationSequence))
        {
            //We get Image Orientation (Patient) (1C) + "Patient Orientation" assignment
            DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMImageOrientationPatient);
            if (dicomValue)
            {
                ImageOrientation imageOrientation;
                imageOrientation.setDICOMFormattedImageOrientation(dicomValue->getValueAsQString());
                image->setImageOrientationPatient(imageOrientation);
                // We pass the ImageOrientation obtained to create the orientation tags
                PatientOrientation patientOrientation;
                patientOrientation.setPatientOrientationFromImageOrientation(image->getImageOrientationPatient());
                image->setPatientOrientation(patientOrientation);
            }
        }

        // Plane Position Module - C.7.6.16.2.3
        // According to DICOM it is only allowed to contain a single item
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMPlanePositionSequence))
        {
            // Obtenim Image Position (Patient) (1C)
            DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMImagePositionPatient);
            if (dicomValue)
            {
                QString imagePositionPatientString = dicomValue->getValueAsQString();
                if (!imagePositionPatientString.isEmpty())
                {
                    QStringList list = imagePositionPatientString.split("\\");
                    if (list.size() == 3)
                    {
                        double position[3] = { list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble() };
                        image->setImagePositionPatient(position);
                    }
                }
                else
                {
                    DEBUG_LOG("Value is empty when it should contain some value!");
                }
            }
        }

        // CT Pixel Value Transformation Module - C.8.15.3.10 - Enhanced CT
        // Pixel Value Transformation Module - C.7.6.16.2.9 - Enhanced MR
        // They contain the same information. The first is simply specialization for CTs
        // According to DICOM it is only allowed to contain a single item
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMPixelValueTransformationSequence))
        {
            // Obtenim Rescale Intercept (1)
            DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMRescaleIntercept);
            if (dicomValue)
            {
                image->setRescaleIntercept(dicomValue->getValueAsDouble());
            }
            else
            {
                ERROR_LOG("Falta el tag RescaleIntercept que hauria d'estar present!");
            }
            // Obtenim Rescale Slope (1)
            dicomValue = item->getValueAttribute(DICOMRescaleSlope);
            if (dicomValue)
            {
                image->setRescaleSlope(dicomValue->getValueAsDouble());
            }
            else
            {
                ERROR_LOG("Falta el tag RescaleSlope que hauria d'estar present!");
            }
        }
    }
    // XA and XRF attributes
    else if (sopClassUID == UIDEnhancedXAImageStorage || sopClassUID == UIDEnhancedXRFImageStorage)
    {
        // XA/XRF Frame Pixel Data Properties Macro - C.8.19.6.4
        // According to DICOM only a single Item shall be included in this sequence
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMFramePixelDataPropertiesSequence))
        {
            // Imager Pixel Spacing (1C)
            // Required if ImageType equals ORIGINAL. May be present otherwise.
            DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMImagerPixelSpacing);
            if (dicomValue)
            {
                validateAndSetSpacingAttribute(image, dicomValue->getValueAsQString(), DICOMImagerPixelSpacing);
            }
            else
            {
                ERROR_LOG("Imager Pixel Spacing not found in a sequence where is suposed to be present");
            }
        }
        // X-Ray Object Thickness Macro - C.8.19.6.7
        // According to DICOM it is only allowed to contain a single item
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMObjectThicknessSequence))
        {
            // Obtenim Calculated Anatomy Thickness (1)
            DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMCalculatedAnatomyThickness);
            if (dicomValue)
            {
                image->setSliceThickness(dicomValue->getValueAsDouble());
            }
            else
            {
                ERROR_LOG("No s'ha trobat el tag Calculated Anatomy Thickness en una seqüència que se suposa que l'ha de tenir!");
            }
        }

        // Patient Orientation in Frame Macro - C.7.6.16.2.15
        // Required if C-arm Positioner Tabletop Relationship is present and equal to YES
        // It could be present even if the previous condition is not met
        // According to DICOM it is only allowed to contain a single item
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMPatientOrientationInFrameSequence))
        {
            // Obtenim Patient Orientation (1)
            DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMPatientOrientation);
            if (dicomValue)
            {
                PatientOrientation patientOrientation;
                patientOrientation.setDICOMFormattedPatientOrientation(dicomValue->getValueAsQString());
                image->setPatientOrientation(patientOrientation);
            }
            else
            {
                ERROR_LOG("The Patient Orientation tag was not found in a sequence that is supposed to have it!");
            }
        }
    }

    // Frame Display Shutter Macro (C.7.6.16.2.16)
    // It can be found in the Enhanced modes: XA, XRF and US Volume
    if (sopClassUID == UIDEnhancedXAImageStorage || sopClassUID == UIDEnhancedXRFImageStorage || sopClassUID == UIDEnhancedUSVolumeStorage)
    {
        // According to DICOM it is only allowed to contain a single item
        if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMFrameDisplayShutterSequence))
        {
            fillDisplayShutterInformation(image, item);
        }
    }
    
    // Below we read the tags / modules found in all enhanced modes (MR / CT / XA / XRF)

    // Frame YOU LUT Macro (C.7.6.16.2.10) or Frame YOU LUT With LUT Macro (C.7.6.16.2.10b)
    // According to DICOM it is only allowed to contain a single item
    if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMFrameVOILUTSequence))
    {
        fillVoiLuts(image, item);
    }

    // Attributes we use for hanging protocols

    // Frame Anatomy Module (C.7.6.16.2.8)
    // According to DICOM it is only allowed to contain a single item
    if (DICOMSequenceItem *item = frameItem->getFirstSequenceItem(DICOMFrameAnatomySequence))
    {
        // Obtenim Frame Laterality (1)
        DICOMValueAttribute *dicomValue = item->getValueAttribute(DICOMFrameLaterality);
        if (dicomValue)
        {
            image->setImageLaterality(dicomValue->getValueAsQString().at(0));
        }
        else
        {
            ERROR_LOG("The Frame Laterality tag was not found in a sequence that is supposed to have it!");
        }
    }
}

unsigned short ImageFillerStep::getNumberOfOverlays(const DICOMTagReader *dicomReader)
{
    DICOMTag overlayTag(DICOMOverlayRows);

    unsigned short numberOfOverlays = 0;

    bool stop = false;
    while (!stop)
    {
        if (dicomReader->tagExists(overlayTag))
        {
            ++numberOfOverlays;
            overlayTag.setGroup(overlayTag.getGroup() + 2);
        }
        else
        {
            stop = true;
        }
    }

    return numberOfOverlays;
}

void ImageFillerStep::computePixelSpacing(Image *image, const DICOMTagReader *dicomReader)
{
    // We get the pixel spacing according to the modality we are dealing with
    QString pixelSpacing;
    QString imagerPixelSpacing;
    QString modality = dicomReader->getValueAttributeAsQString(DICOMModality);

    // For CT, MR and PET modalities we find pixel spacing
    // a Image Plane Module (C.7.6.2), in the Pixel Spacing tag, type 1
    if (modality == "CT" || modality == "MR" || modality == "PT")
    {
        pixelSpacing = dicomReader->getValueAttributeAsQString(DICOMPixelSpacing);
    }
    else if (modality == "US")
    {
        //In the case of the US mode, we need to do some extra calculations in order to get an approximate pixel spacing
        DICOMSequenceAttribute *ultraSoundsRegionsSequence = dicomReader->getSequenceAttribute(DICOMSequenceOfUltrasoundRegions);
        // We have to check it because it is optional.
        if (ultraSoundsRegionsSequence)
        {
            // This sequence can have more than one item. EVERYTHING We only try the first one, but we should do it for everyone,
            // since it defines more than one region and we could be getting the wrong information
            QList<DICOMSequenceItem*> items = ultraSoundsRegionsSequence->getItems();
            if (!items.isEmpty())
            {
                int physicalUnitsX = items.at(0)->getValueAttribute(DICOMPhysicalUnitsXDirection)->getValueAsInt();
                int physicalUnitsY = items.at(0)->getValueAttribute(DICOMPhysicalUnitsYDirection)->getValueAsInt();

                // 3 means that the units are cm
                if (physicalUnitsX == 3 && physicalUnitsY == 3)
                {
                    double physicalDeltaX = items.at(0)->getValueAttribute(DICOMPhysicalDeltaX)->getValueAsDouble();
                    double physicalDeltaY = items.at(0)->getValueAttribute(DICOMPhysicalDeltaY)->getValueAsDouble();

                    physicalDeltaX = qAbs(physicalDeltaX) * 10.;
                    physicalDeltaY = qAbs(physicalDeltaY) * 10.;

                    // Pixel spacing is rowSpacing\columnSpacing -> ySpacing\xSpacing
                    pixelSpacing = QString("%1\\%2").arg(physicalDeltaY).arg(physicalDeltaX);
                }
            }
        }
    }
    else if (modality == "CR" || modality == "DX" || modality == "RF" || modality == "XA" || modality == "MG" || modality == "IO")
    {
        // These modalities can have both Pixel Spacing and Imager Pixel Spacing tags
        pixelSpacing = dicomReader->getValueAttributeAsQString(DICOMPixelSpacing);
        imagerPixelSpacing = dicomReader->getValueAttributeAsQString(DICOMImagerPixelSpacing);

        if (dicomReader->getValueAttributeAsQString(DICOMSOPClassUID) == UIDXRay3DAngiographicImageStorage)
        {
            // In case it's a 3D XA, we should look for Imager Pixel Spacing in
            // X-Ray 3D Angiographic Image Contributing Sources Module (C.8.21.2.1), Contributing Sources Sequence
            DICOMSequenceAttribute *contributingSourcesSequence = dicomReader->getSequenceAttribute(DICOMContributingSourcesSequence);
            if (contributingSourcesSequence)
            {
                QList<DICOMSequenceItem*> items = contributingSourcesSequence->getItems();
                if (items.count() == 0)
                {
                    ERROR_LOG("Error: Contributing Sources Sequence should have one ore more items. Sequence is empty.");
                }
                else
                {
                    // TODO What to do if we have more than one item? Meanwhile we only take into account the first item only.
                    DICOMValueAttribute *value = items.first()->getValueAttribute(DICOMImagerPixelSpacing);
                    if (value)
                    {
                        imagerPixelSpacing = value->getValueAsQString();
                    }
                    else
                    {
                        ERROR_LOG("Error: Imager Pixel Spacing not found when it should be present in Contributing Sources Sequence (1C).");
                    }
                }
            }
        }
    }
    else
    {
        // By the default, pixel spacing would be the only one checked for the rest of modalities
        pixelSpacing = dicomReader->getValueAttributeAsQString(DICOMPixelSpacing);
    }

    // Put the computed values accordingly
    validateAndSetSpacingAttribute(image, pixelSpacing, DICOMPixelSpacing);
    validateAndSetSpacingAttribute(image, imagerPixelSpacing, DICOMImagerPixelSpacing);
}

void ImageFillerStep::checkAndSetEstimatedRadiographicMagnificationFactor(Image *image, const DICOMTagReader *dicomReader)
{
    QString factor;
    QString modality = dicomReader->getValueAttributeAsQString(DICOMModality);
    // XA Positioner Module C.8.7.5 (3)
    // XRF Positioner Module C.8.7.6 (3)
    // DX Positioning Module C.8.11.5 (3) (U: MG, DX, IO)
    // Breast Tomosynthesis Acquisition Module C.8.21.3.4, X-Ray 3D Acquisition Sequence, (1) (Br To)
    if (modality == "XA" || modality == "RF" || modality == "DX" || modality == "MG" || modality == "IO")
    {
        factor = dicomReader->getValueAttributeAsQString(DICOMEstimatedRadiographicMagnificationFactor);
    }
    else
    {
        DICOMSequenceAttribute *xRay3DAcquisitionSequence = dicomReader->getSequenceAttribute(DICOMXRay3DAcquisitionSequence);
        if (xRay3DAcquisitionSequence)
        {
            QList<DICOMSequenceItem*> items = xRay3DAcquisitionSequence->getItems();
            if (items.count() == 0)
            {
                ERROR_LOG("Error: X-Ray 3D Acquisition Sequence should have one ore more items. Sequence is empty.");
            }
            else
            {
                // TODO What to do if we have more than one item? Meanwhile we only take into account the first item only.
                DICOMValueAttribute *value = items.first()->getValueAttribute(DICOMEstimatedRadiographicMagnificationFactor);
                if (value)
                {
                    factor = value->getValueAsQString();
                }
                else
                {
                    ERROR_LOG("Error: Estimated Pixel Spacing not found when it is mandatory in X-Ray 3D Acquisition Sequence (1).");
                }
            }
        }
    }

    if (!factor.isEmpty())
    {
        image->setEstimatedRadiographicMagnificationFactor(factor.toDouble());
    }
}

bool ImageFillerStep::isEnhancedImageSOPClass(const QString &sopClassUID)
{
    return (sopClassUID == UIDEnhancedCTImageStorage || sopClassUID == UIDEnhancedMRImageStorage || sopClassUID == UIDEnhancedXAImageStorage ||
            sopClassUID == UIDEnhancedXRFImageStorage || sopClassUID == UIDEnhancedUSVolumeStorage || sopClassUID == UIDEnhancedMRColorImageStorage ||
            sopClassUID == UIDEnhancedPETImageStorage || sopClassUID == UIDBreastTomosynthesisImageStorage);
}

void ImageFillerStep::validateAndSetSpacingAttribute(Image *image, const QString &spacing, const DICOMTag &tag)
{
    if (spacing.isEmpty())
    {
        return;
    }

    PixelSpacing2D pixelSpacing = DICOMFormattedValuesConverter::parsePixelSpacing(spacing);

    if (pixelSpacing.isValid())
    {
        if (tag == DICOMPixelSpacing)
        {
            image->setPixelSpacing(pixelSpacing);
        }
        else if (tag == DICOMImagerPixelSpacing)
        {
            image->setImagerPixelSpacing(pixelSpacing);
        }
        else
        {
            DEBUG_LOG("Wrong pixel spacing tag provided");
        }
    }
    else
    {
        DEBUG_LOG("No standard defined pixel spacing value found: " + spacing);
    }
}

}
