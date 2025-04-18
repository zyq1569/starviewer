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

#ifndef UDGIMAGE_H
#define UDGIMAGE_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QPair>
#include <QStringList>
#include <QPixmap>

#include "dicomsource.h"
#include "imageorientation.h"
#include "patientorientation.h"
#include "photometricinterpretation.h"
#include "imageoverlay.h"
#include "displayshutter.h"
#include "voilut.h"
#include "pixelspacing2d.h"

namespace udg {

class Series;

/**
  Class that encapsulates the properties of an image of a series of the Series class
*/
class Image : public QObject {
    Q_OBJECT
public:
    Image(QObject *parent = 0);
    ~Image();

    /// Assigns / obtains the SOP Instance UID of the image
    void setSOPInstanceUID(const QString &uid);
    QString getSOPInstanceUID() const;

    ///Assigns / obtains instance number
    void setInstanceNumber(const QString &number);
    QString getInstanceNumber() const;

    /// Assign / Obtain the orientation of the image plane, also called direction cousins.
    /// Values are the vectors that make up the image plane.
    /// From these two vectors, the normal of the image plane is calculated
    /// @param orientation [] The values of the vectors that define the image plane.
    void setImageOrientationPatient(const ImageOrientation &imageOrientation);
    ImageOrientation getImageOrientationPatient() const;

    /// Assign / Obtain patient guidance
    void setPatientOrientation(const PatientOrientation &orientation);
    PatientOrientation getPatientOrientation() const;

    /// Assign / Get pixel spacing
    void setPixelSpacing(double x, double y);
    void setPixelSpacing(const PixelSpacing2D &spacing);
    PixelSpacing2D getPixelSpacing() const;

    /// Set/Get Imager Pixel Spacing
    void setImagerPixelSpacing(double x, double y);
    void setImagerPixelSpacing(const PixelSpacing2D &spacing);
    PixelSpacing2D getImagerPixelSpacing() const;

    /// Set/Get Estimated Radiographic Magnification Factor
    void setEstimatedRadiographicMagnificationFactor(double x);
    double getEstimatedRadiographicMagnificationFactor() const;

    /// Returns the preferred pixel spacing to use with this image.
    /// This method serves to deal with the cases
    /// where, depending on the modality and the present pixel spacing attributes,
    ///  choose the pixel spacing value that should be used by default
    /// For instance, if we have an image with MG modality,
    /// imager pixel spacing and estimated radiographic factor, the value returned should be
    /// ImagerPixelSpacing corrected by EstimatedRadiographicFactor
    PixelSpacing2D getPreferredPixelSpacing() const;

    ///Assign / Obtain slice thickness, aka Z spacing
    void setSliceThickness(double z);
    double getSliceThickness() const;

    /// Assign / Obtain the position of the image.
    void setImagePositionPatient(double position[3]);
    const double* getImagePositionPatient() const;

    ///Assign / Obtain samples per pixel
    void setSamplesPerPixel(int samples);
    int getSamplesPerPixel() const;

    /// Assign / Obtain the photometric interpretation
    void setPhotometricInterpretation(const QString &value);
    PhotometricInterpretation getPhotometricInterpretation() const;

    /// Assignar/Obtenir files/columnes
    void setRows(int rows);
    int getRows() const;
    void setColumns(int columns);
    int getColumns() const;

    ///Assign / Get hosted bits
    void setBitsAllocated(int bits);
    int getBitsAllocated() const;

    /// Assign / Obtain stored bits
    void setBitsStored(int bits);
    int getBitsStored() const;

    ///Assign / Get the highest bit
    void setHighBit(int highBit);
    int getHighBit() const;

    ///Assign / Obtain pixel representation
    void setPixelRepresentation(int representation);
    int getPixelRepresentation() const;

    /// Assign / Obtain MODALITY LUT scaling values that are applied to the image
    /// the formula is f (x) = a * x + b, where 'x' is the pixel value of the image, 'a' the Slope and 'b' the Intercept
    void setRescaleSlope(double slope);
    double getRescaleSlope() const;
    void setRescaleIntercept(double intercept);
    double getRescaleIntercept() const;

    ///Assign / Obtain the VOI LUT embossing values that are applied to the image
    void addVoiLut(const VoiLut &voiLut);
    VoiLut getVoiLut(int index = 0) const;

    /// Assign the WindowLevels list of the image. If the listing contains an invalid WW / WL, it is not added
    /// Any previous window level list will always be deleted
    void setVoiLutList(const QList<VoiLut> &voiLutList);
    
    ///It returns the number of window levels we have
    int getNumberOfVoiLuts() const;

    ///We tell you which parent series it belongs to
    void setParentSeries(Series *series);
    Series* getParentSeries() const;

    /// Assigns / returns the absolute path of the image
    void setPath(const QString &path);
    QString getPath() const;

    /// Assigns / returns the slice location of the image
    void setSliceLocation(const QString &sliceLocation);
    QString getSliceLocation() const;

    ///Assign / Obtain the date and time the series was downloaded to the Local database
    void setRetrievedDate(QDate date);
    void setRetrievedTime(QTime time);
    QDate getRetrievedDate() const;
    QTime getRetrievedTime() const;

    /// Returns the Acquisition Number (0020,0012).
    const QString& getAcquisitionNumber() const;
    /// Sets the Acquisition Number (0020,0012).
    void setAcquisitionNumber(QString acquisitionNumber);

    /// Assign / Obtain the description of the image type
    void setImageType(const QString &imageType);
    QString getImageType() const;

    ///Assign / Get viewPosition
    void setViewPosition(const QString &viewPosition);
    QString getViewPosition() const;

    /// Assign / Obtain the laterity of the image
    void setImageLaterality(const QChar &imageLaterality);
    QChar getImageLaterality() const;

    /// Assign / Get the View Code description. For now it will only be applied for mammography images.
    void setViewCodeMeaning(const QString &viewCodeMeaning);
    QString getViewCodeMeaning() const;

    ///Assign / Obtain the frame number
    void setFrameNumber(int frameNumber);
    int getFrameNumber() const;

    /// Assign / Obtain the phase number
    void setPhaseNumber (int phaseNumber);
    int getPhaseNumber() const;

    /// Assign / Obtain the volume number to which the image in the series belongs
    void setVolumeNumberInSeries (int volumeNumberInSeries);
    int getVolumeNumberInSeries() const;

    ///Assign / Obtain the number occupied by the image within volume
    void setOrderNumberInVolume(int orderNumberInVolume);
    int getOrderNumberInVolume() const;

    /// Assign / Obtain Content Time (time of data creation)
    void setImageTime(const QString &imageTime);
    QString getImageTime() const;

    ///It returns the time in hh: mm: ss format in which the creation of the image began
    QString getFormattedImageTime() const;

    /// Sets the transfer syntax UID.
    void setTransferSyntaxUID(const QString &transferSyntaxUID);
    /// Returns the transfer syntax UID.
    const QString& getTransferSyntaxUID() const;

    /// Returns the distance of the origin of the image passed by parameter
    ///  with respect to an origin 0, 0, 0, according to the normal of the plane
    /// EVERYTHING Give it a more understandable name
    static double distance(Image *image);
    
    /// Methods for obtaining / assigning the number of overlays that the image has
    bool hasOverlays() const;
    unsigned short getNumberOfOverlays() const;
    void setNumberOfOverlays(unsigned short overlays);

    /// Get the list of overlays
    QList<ImageOverlay> getOverlays();

    /// Get a list with the division into regions of all overlays. All original overlays are merged into one
    /// and then the optimal partition of the different parts that make it up is done
    QList<ImageOverlay> getOverlaysSplit();

    /// He tells us if he has shutters or not
    bool hasDisplayShutters() const;
    
    ///Adds a Display Shutter to the image
    void addDisplayShutter(const DisplayShutter &shutter);
    
    /// Assigns a list of Display Shutters to the image. This overwrites the above.
    void setDisplayShutters(const QList<DisplayShutter> &shuttersList);
    
    ///Gets the Display Shutters list
    QList<DisplayShutter> getDisplayShutters() const;

    /// Returns the appropriate DisplayShutters composition for display
    /// If there are no shutters or no valid layout, an empty DisplayShutter will return
    DisplayShutter getDisplayShutterForDisplay();

    /// Returns display shutter for display in vtkImageData format, with the same dimensions as the image.
    /// The vtkImageData object is created only the first time, subsequent calls return the same object.
    vtkImageData* getDisplayShutterForDisplayAsVtkImageData();

    ///Assign / Obtain the DICOMSource of the image. Indicates the source of the DICOM files contained in the image
    void setDICOMSource(const DICOMSource &imageDICOMSource);
    DICOMSource getDICOMSource() const;

    ///It returns the key that identifies the image
    QString getKeyIdentifier() const;

    /// The method returns the thumbnail of the image. It will be created the first time it is requested
    /// @param getFromCache If true it will try to load the thumbnail if it is created in the cache.
    /// Otherwise, it will simply check that it is not created in memory and enough
    /// @param resolution The resolution with which we want the thumbnail
    /// @return A QPixmap with the thumbnail
    QPixmap getThumbnail(bool getFromCache = false, int resolution = 100);

    ///Returns a list of the modes we support as Image
    static QStringList getSupportedModalities();

//add:20241204
public:
	void setDICOMKVP(QString KVP);
	QString getDICOMKVP();

	void setXRayTubeCurrent(QString XRayTubeCurrent);
	QString getXRayTubeCurrent();

private:
    /// Read the overlays. If splitOverlays is true, save them by dividing
    /// the optimal regions in the m_overlaysSplit list
    /// Otherwise read them separately and save them to the m_overlaysList
    bool readOverlays(bool splitOverlays = true);

private:
    /// Atributs DICOM

    /// Image / file identifier. (0008,0018)
    QString m_SOPInstanceUID;

    /// General image information. C.7.6 General Image Module - PS 3.3.

    /// Number that identifies the image. (0020,0013) Type 2
    QString m_instanceNumber;

    /// Anatomical orientation of the rows and columns of the image (LR / AP / HF).
    ///  Required if image does not require Image Orientation (Patient) (0020,0037) i
    /// Image Position (Patient) (0020,0032). See C.6.7.1.1.1. (0020,0020) Type 2C.
    PatientOrientation m_patientOrientation;

    /// TODO Referenced Image Sequence (0008,1140) Type 3. Sequence that references
    //other images significantly related to these,
    /// as a post-localizer for CT.

    /// TODO Icon Image Sequence (0088,0200) Type 3. The following icon image
    //is representative of this image. see C.7.6.1.1.6

    // Image Plane Module C.6.7.2
    /// Physical distance between the center of each pixel (row, column) in mm. See 10.7.1.3. (0028,0030) Type 1
    PixelSpacing2D m_pixelSpacing;
    
    /// Imager Pixel spacing (0018,1164)
    /// Physical distance measured at the front plane of the Image Receptor housing between the center of each pixel.
    /// Present in CR (3), DX, MG, IO (1), Enhanced XA/XRF (1C), 3D XA (1C)
    PixelSpacing2D m_imagerPixelSpacing;

    /// Estimated Radiographic Magnification Factor (0018,1114)
    /// Ratio of Source Image Receptor Distance (SID) over Source Object Distance (SOD).
    /// May be present (3) in DX Positioning Module (C.8.11.5) (MG, DX, IO),
    /// XA Positioner Module C.8.7.5 (XA),
    /// XRF Positioner Module C.8.7.6 (RF), and must be present (1) in Breast
    ///  Tomosynthesis Acquisition Module C.8.21.3.4 (Br To)
    double m_estimatedRadiographicMagnificationFactor;
    /// Image orientation vectors with respect to the patient.
    /// See C.6.7.2.1.1. (020,0037) Type 1.
    ImageOrientation m_imageOrientationPatient;

    /// Image position. The x, y, z coordinates the upper left corner
    ///  (first transmitted pixel) of the image, in mm.
    /// See C.6.7.2.1.1. (0020,0032) Type 1. \ EVERYTHING aka origin ?.
    double m_imagePositionPatient[3];

    ///Slice thickness in mm. (0018,0050) Type 2.
    double m_sliceThickness;

    // Image Pixel Module C.6.7.3
    /// Number of samples per pixel in the image. See C.6.7.3.1.1. (0028,0002) Type 1.
    int m_samplesPerPixel;

    /// Photometric interpretation (monochrome, color ...). See C.6.7.3.1.2. (0028,0004) Type 1.
    PhotometricInterpretation m_photometricInterpretation;

    /// Rows and columns of the image. (0028,0010), (0028,0011) Type 1
    int m_rows;
    int m_columns;

    ///Bits hosted by each pixel. Each sample must have the
    ///  same number of pixels hosted. See PS 3.5 (0028,0100)
    int m_bitsAllocated;

    ///Bits stored for each pixel. Each sample must have the
    ///  same number of pixels stored. See PS 3.5 (0028,0101)
    int m_bitsStored;

    ///Most significant bit. See PS 3.5. (0028,0102) Type 1
    int m_highBit;

    ///Representation of each sample. Listed values
    /// 0000H = unsigned integer, 0001H = complement to 2. (0028,0103) Type 1
    int m_pixelRepresentation;

    ///MODALITY LUT rescaling values. (0028,1053), (0028,1054). Type 1
    double m_rescaleSlope, m_rescaleIntercept;

    /// VOI LUT rescaling values. (0028,1050), (0028,1051) Type 1C,
    /// present if there is no VOI LUT Sequence
    /// May include "Explanation" of window levels if any,
    /// descriptive text. (0028,1055) Type 3.
    /// Since we can have more than one we will have a list
    QList<VoiLut> m_voiLutList;
    // ALL improve definition
    /// Special situation of the slice in mm. (0020,1041)
    /// SC-> type 3
    /// NM-> type 3
    /// CT-> The documentation says this field is not included
    ///  but philips uses it as a Table Position
    QString m_sliceLocation;
    /// Image type. You can define us if it is a localizer,
    /// for example. Contains values separated by '\\'
    /// Found in the General Image module C.7.6.1 and in the Enhanced
    /// MR / CT / XA / XRF Image modules (C.8.13.1 / C.8.15.2 / C.8.19.2)
    /// In the case of Enhanced CT / MR images we will fill it with
    /// the FrameType value contained in the functional group CT / MR Image Frame Type
    QString m_imageType;

    /// Radiographic view associated with Patient Position.
    /// We find it in the CR Series (C.8.1.1) and DX Positioning (C.8.11.5) modules
    /// Defined values:
    /// AP = Previous / Back
    /// PA = Previous / Previous
    /// LL = Left Lateral
    /// RL = Right Lateral
    /// RLD = Right Lateral Decubitus
    /// LLD = Left Lateral Decubitus
    /// RLO = Right Lateral Oblique
    /// LLO = Left Lateral Oblique
    QString m_viewPosition;

    /// Laterality of the possibly matched part of the body examined.
    /// We find it in the modules DX Anatomy (C.8.11.2),
    ///  Mammography Image (C.8.11.7), Intra-oral Image (C.8.11.9) and Ocular Region Imaged (C.8.17.5)
    /// We also find it in the Frame Anatomy module (C.7.6.16.2.8)
    /// common to all enhanced, but the tag is called Frame Laterality instead of Image Laterality.
    /// Defined values:
    /// R = right
    /// L = left
    /// U = unpaired
    /// B = both left and right
    QChar m_imageLaterality;

    /// Description of the image view type. Its use will
    ///  be applied basically for mammography cases defined in
    /// PS 3.16 - Context ID 4014 (cranio-caudal, medio-lateral
    ///  oblique, etc ...) but we could extend its use to other types of image
    /// which also make use of this tag to store this type
    /// of information with other possible specific values.
    QString m_viewCodeMeaning;

    ///Frame number
    int m_frameNumber;

    /// Image phase number
    int m_phaseNumber;

    ///Volume number to which the image in the series belongs
    int m_volumeNumberInSeries;

    /// Image order number within vo
    int m_orderNumberInVolume;

    ///Time the pixel data was created
    QString m_imageTime;

    // TODO C.7.6.5 CINE MODULE: Multi-frame Cine Image

    /// Transfer Syntax UID (0002,0010)
    /// Transfer syntax defines how DICOM objects are serialized.
    QString m_transferSyntaxUID;

    /// Atributs NO-DICOM

    /// El path absolut de la imatge
    QString m_path;

    /// Date the image was downloaded to the local database
    QDate m_retrievedDate;
    QTime m_retrieveTime;

    /// Acquisition Number (0020,0012). Type 3 in C.7.6.1
    /// General Image Module (type 1 or 2 in other modules).
    /// A number identifying the single continuous gathering
    ///  of data over a period of time that resulted in this image.
    QString m_acquisitionNumber;

    /// Attribute that will tell us how many overlays the image has
    unsigned short m_numberOfOverlays;

    /// List of loaded overlays
    QList<ImageOverlay> m_overlaysList;

    ///List containing the partition in optimal fusion regions of all overlays
    QList<ImageOverlay> m_overlaysSplit;
    /// Remembers if m_overlaysSplit has been calculated.
    bool m_overlaysSplitComputed;

    ///List of display shutters
    QList<DisplayShutter> m_shuttersList;

    ///DisplayShutter for display that we will save once created
    DisplayShutter m_displayShutterForDisplay;

    /// Tells us if we need to create the display shutter by display
    bool m_haveToBuildDisplayShutterForDisplay;

    /// Display shutter for display in vtkImageData format.
    vtkSmartPointer<vtkImageData> m_displayShutterForDisplayVtkImageData;

    /// The parent series
    Series *m_parentSeries;

    /// Preview image cache
    QPixmap m_thumbnail;

    //Indicates the origin of DICOM images
    DICOMSource m_imageDICOMSource;

	//add 20241204
	QString m_dicomKVP;
	QString m_dicomXRayTubeCurrent;
};

}

#endif
