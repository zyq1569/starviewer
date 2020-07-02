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

    /// Assign / get the SOPInstanceUID of the image
    void setSOPInstanceUID (const QString & uid);
    QString getSOPInstanceUID () const;

    /// Assign / get instance number
    void setInstanceNumber (const QString & number);
    QString getInstanceNumber () const;

    /// Assign / Obtain the orientation of the image plane, also called direction cousins.
    /// Values are the vectors that make up the image plane.
    /// From these two vectors, the normal of the image plane is calculated
    /// @param orientation [] The values of the vectors that define the image plane.
    void setImageOrientationPatient (const ImageOrientation & imageOrientation);
    ImageOrientation getImageOrientationPatient () const;

    /// Assign / Obtain patient orientation
    void setPatientOrientation (const PatientOrientation & orientation);
    PatientOrientation getPatientOrientation () const;

    /// Assign / Get pixel spacing
    void setPixelSpacing (double x, double y);
    void setPixelSpacing (const PixelSpacing2D & spacing);
    PixelSpacing2D getPixelSpacing () const;

    /// Set/Get Imager Pixel Spacing
    void setImagerPixelSpacing(double x, double y);
    void setImagerPixelSpacing(const PixelSpacing2D &spacing);
    PixelSpacing2D getImagerPixelSpacing() const;

    /// Set/Get Estimated Radiographic Magnification Factor
    void setEstimatedRadiographicMagnificationFactor(double x);
    double getEstimatedRadiographicMagnificationFactor() const;

    /// Returns the preferred pixel spacing to use with this image. This method serves to deal with the cases
    /// where, depending on the modality and the present pixel spacing attributes, choose the pixel spacing value that should be used by default
    /// For instance, if we have an image with MG modality, imager pixel spacing and estimated radiographic factor, the value returned should be
    /// ImagerPixelSpacing corrected by EstimatedRadiographicFactor
    PixelSpacing2D getPreferredPixelSpacing() const;

    ///Assign / Obtain slice thickness, aka Z spacing
    void setSliceThickness(double z);
    double getSliceThickness() const;

    ///Assign / Obtain the position of the image.
    void setImagePositionPatient(double position[3]);
    const double* getImagePositionPatient() const;

    /// Assign / Obtain samples per pixel
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

    /// Assignar/Obtenir els bits allotjats
    void setBitsAllocated(int bits);
    int getBitsAllocated() const;

    /// Assignar/Obtenir els bits emmagatzemats
    void setBitsStored(int bits);
    int getBitsStored() const;

    /// Assign / Get the highest bit
    void setHighBit (int highBit);
    int getHighBit () const;

    /// Assign / Obtain pixel representation
    void setPixelRepresentation (int representation);
    int getPixelRepresentation () const;

    /// Assign / Obtain MODALITY LUT embedding values that are applied to the image
    /// the formula is f (x) = a * x + b, where 'x' is the pixel value of the image, 'a' the Slope and 'b' the Intercept
    void setRescaleSlope(double slope);
    double getRescaleSlope() const;
    void setRescaleIntercept(double intercept);
    double getRescaleIntercept() const;

    /// Assign / Obtain the values of the embossing of the VOI LUT that are applied on the image
    void addVoiLut (const VoiLut & voiLut);
    VoiLut getVoiLut (int index = 0) const;

    /// Assign the WindowLevels list of the image. If the listing contains an invalid WW / WL, it is not added
    /// Any previous window level list will always be deleted
    void setVoiLutList (const QList <VoiLut> & voiLutList);
    
    /// It returns the number of window levels we have
    int getNumberOfVoiLuts ();

    /// We tell you which parent series it belongs to
    void setParentSeries (Series * series);
    Series * getParentSeries () const;

    /// Assigns / returns the absolute path of the image
    void setPath (const QString & path);
    QString getPath () const;

    /// Assigns / returns the slice location of the image
    void setSliceLocation (const QString & sliceLocation);
    QString getSliceLocation () const;

    /// Assign / Obtain the date and time the series was downloaded to the Local database
    void setRetrievedDate (QDate data);
    void setRetrievedTime (QTime time);
    QDate getRetrievedDate ();
    QTime getRetrievedTime ();

    /// Assign / Get the description of the image type
    void setImageType (const QString & imageType);
    QString getImageType () const;

    /// Assign / Get the viewPosition
    void setViewPosition (const QString & viewPosition);
    QString getViewPosition () const;

    /// Assign / Obtain the laterity of the image
    void setImageLaterality (const QChar & imageLaterality);
    QChar getImageLaterality () const;

    /// Assign / Get the description of the View Code. For now it will only be applied for mammography images.
    void setViewCodeMeaning (const QString & viewCodeMeaning);
    QString getViewCodeMeaning () const;

    /// Assign / Get the frame number
    void setFrameNumber (int frameNumber);
    int getFrameNumber () const;

    /// Assign / Obtain the phase number
    void setPhaseNumber (int phaseNumber);
    int getPhaseNumber () const;

    /// Assign / Obtain the volume number to which the image in the series belongs
    void setVolumeNumberInSeries (int volumeNumberInSeries);
    int getVolumeNumberInSeries () const;

    /// Assign / Obtain the number occupied by the image within volume
    void setOrderNumberInVolume (int orderNumberInVolume);
    int getOrderNumberInVolume () const;

    /// Assign / Obtain Content Time (time of data creation)
    void setImageTime (const QString & imageTime);
    QString getImageTime () const;

    /// Returns the time in hh: mm: ss format in which the image creation began
    QString getFormattedImageTime () const;
    /// Sets the transfer syntax UID.
    void setTransferSyntaxUID(const QString &transferSyntaxUID);
    /// Returns the transfer syntax UID.
    const QString& getTransferSyntaxUID() const;

    /// Returns the distance of the origin of the image passed by parameter with respect to an origin 0, 0, 0, according to the normal of the plane
    /// EVERYTHING Give it a more understandable name
    static double distance (Image * image);

    /// Methods for obtaining / assigning the number of overlays that the image has
    bool hasOverlays () const;
    unsigned short getNumberOfOverlays () const;
    void setNumberOfOverlays (unsigned short overlays);

    /// Gets the list of overlays
    QList <ImageOverlay> getOverlays ();

    /// Gets a list with the division into regions of all overlays. All original overlays are merged into one
    /// and then the optimal partition of the different parts that make it up is done
    QList <ImageOverlay> getOverlaysSplit ();

    /// It tells us if it has shutters or not
    bool hasDisplayShutters () const;

    /// Adds a Display Shutter to the image
    void addDisplayShutter (const DisplayShutter & shutter);

    /// Assigns a list of Display Shutters to the image. This overwrites the above.
    void setDisplayShutters (const QList <DisplayShutter> & shuttersList);

    /// Gets the Display Shutters list
    QList <DisplayShutter> getDisplayShutters () const;

    /// Returns the appropriate DisplayShutters composition for display
    /// If there are no shutters or no valid layouts an empty DisplayShutter will return
    DisplayShutter getDisplayShutterForDisplay ();

    /// Returns display shutter for display in vtkImageData format, with the same dimensions as the image.
    /// The z value of the extent can be specified with zSlice.
    /// The vtkImageData object is created only the first time, subsequent calls return the same object.
    vtkImageData * getDisplayShutterForDisplayAsVtkImageData (int zSlice = 0);

    /// Assign / Obtain the DICOMSource from the image. Indicates the source of the DICOM files contained in the image
    void setDICOMSource (const DICOMSource & imageDICOMSource);
    DICOMSource getDICOMSource () const;

    /// Returns the key that identifies the image
    QString getKeyIdentifier () const;

    /// The method returns the thumbnail of the image. It will be created the first time it is requested
    /// @param getFromCache If true it will try to load the thumbnail if it is created in the cache.
    /// Otherwise, it will simply check that it is not created in memory and enough
    /// @param resolution The resolution with which we want the thumbnail
    /// @return A QPixmap with the thumbnail
    QPixmap getThumbnail (bool getFromCache = false, int resolution = 100);

    /// Returns a list of the modes we support as Image
    static QStringList getSupportedModalities ();

private:
    /// Read the overlays. If splitOverlays is true, save them by dividing the optimal regions in the m_overlaysSplit list
    /// Otherwise read them separately and save them in the m_overlaysList
    bool readOverlays(bool splitOverlays = true);

private:
    /// Atributs DICOM

    /// Image / file identifier. (0008,0018)
    QString m_SOPInstanceUID;

    /// General image information. C.7.6 General Image Module - PS 3.3.

    /// Number that identifies the image. (0020,0013) Type 2
    QString m_instanceNumber;

    /// Anatomical orientation of the rows and columns of the image (LR / AP / HF). Required if image does not require Image Orientation (Patient) (0020,0037) i
    /// Image Position (Patient) (0020,0032). See C.6.7.1.1.1. (0020,0020) Type 2C.
    PatientOrientation m_patientOrientation;

    // TODO Referenced Image Sequence (0008,1140) Tipus 3. Seqüència que referència altres imatges significativament relacionades amb aquestes,
    // com un post-localizer per CT.

    // TODO Icon Image Sequence (0088,0200) Tipus 3. La següent imatge d'icona és representativa d'aquesta imatge. veure C.7.6.1.1.6

    // Image Plane Module C.6.7.2
    /// Distància física entre el centre de cada píxel (row,column) en mm. Veure 10.7.1.3. (0028,0030) Tipus 1
    PixelSpacing2D m_pixelSpacing;

    /// Imager Pixel spacing (0018,1164)
    /// Physical distance measured at the front plane of the Image Receptor housing between the center of each pixel.
    /// Present in CR (3), DX, MG, IO (1), Enhanced XA/XRF (1C), 3D XA (1C)
    PixelSpacing2D m_imagerPixelSpacing;

    /// Estimated Radiographic Magnification Factor (0018,1114)
    /// Ratio of Source Image Receptor Distance (SID) over Source Object Distance (SOD).
    /// May be present (3) in DX Positioning Module (C.8.11.5) (MG, DX, IO), XA Positioner Module C.8.7.5 (XA),
    /// XRF Positioner Module C.8.7.6 (RF), and must be present (1) in Breast Tomosynthesis Acquisition Module C.8.21.3.4 (Br To)
    double m_estimatedRadiographicMagnificationFactor;

    /// Vectors d'orientació de la imatge respecte al pacient.
    /// Veure C.6.7.2.1.1. (020,0037) Tipus 1.
    ImageOrientation m_imageOrientationPatient;

    /// Posició de la imatge. Les coordenades x,y,z la cantonada superior esquerre (primer pixel transmés) de la imatge, en mm.
    /// Veure C.6.7.2.1.1. (0020,0032) Tipus 1. \TODO aka origen?.
    double m_imagePositionPatient[3];

    /// Gruix de llesca en mm. (0018,0050) Tipus 2.
    double m_sliceThickness;

    // Image Pixel Module C.6.7.3
    /// Nombre de mostres per pixel en la imatge. Veure C.6.7.3.1.1. (0028,0002) Tipus 1.
    int m_samplesPerPixel;

    /// Interpretació fotomètrica (monocrom,color...). Veure C.6.7.3.1.2. (0028,0004) Tipus 1.
    PhotometricInterpretation m_photometricInterpretation;

    /// Files i columnes de la imatge. (0028,0010),(0028,0011) Tipus 1
    int m_rows;
    int m_columns;

    /// Bits allotjats per cada pixel. Cada mostra ha de tenir el mateix nombre de pixels allotjats. Veure PS 3.5 (0028,0100)
    int m_bitsAllocated;

    /// Bits emmagatzemats per cada pixel. Cada mostra ha de tenir el mateix nombre de pixels emmagatzemats. Veure PS 3.5 (0028,0101)
    int m_bitsStored;

    /// Bit més significant. Veure PS 3.5. (0028,0102) Tipus 1
    int m_highBit;

    /// Representació de cada mostra. Valors enumerats 0000H=unsigned integer, 0001H=complement a 2. (0028,0103) Tipus 1
    int m_pixelRepresentation;

    /// Valors de rescalat de la MODALITY LUT. (0028,1053),(0028,1054). Tipus 1
    double m_rescaleSlope, m_rescaleIntercept;

    /// Valors de rescalat de la VOI LUT. (0028,1050),(0028,1051) Tipus 1C, present si no hi ha VOI LUT Sequence
    /// Poden incloure "Explicació" dels window levels si n'hi ha, texte descriptiu.(0028,1055) Tipus 3.
    /// Com que podem tenir més d'un tindrem una llista
    QList<VoiLut> m_voiLutList;

    // TODO millorar definició
    /// Situació especial de la llesca en mm. (0020,1041)
    /// SC->tipus 3
    /// NM->tipus 3
    /// CT-> A la documentació dicom aquest camp no hi figura però philips l'utiliza com a Table Position
    QString m_sliceLocation;

    /// Tipus d'imatge. Ens pot definir si es tracta d'un localizer, per exemple. Conté els valors separats per '\\'
    /// Es troba al mòdul General Image C.7.6.1 i als mòduls Enhanced MR/CT/XA/XRF Image (C.8.13.1/C.8.15.2/C.8.19.2)
    /// En el cas d'imatges Enhanced CT/MR l'omplirem amb el valor FrameType contingut al functional group CT/MR Image Frame Type
    QString m_imageType;

    /// Vista radiogràfica associada a Patient Position. El trobem als mòduls CR Series (C.8.1.1) i DX Positioning (C.8.11.5)
    /// Valors definits:
    /// AP = Anterior/Posterior
    /// PA = Posterior/Anterior
    /// LL = Left Lateral
    /// RL = Right Lateral
    /// RLD = Right Lateral Decubitus
    /// LLD = Left Lateral Decubitus
    /// RLO = Right Lateral Oblique
    /// LLO = Left Lateral Oblique
    QString m_viewPosition;

    /// Lateralitat de la possiblement aparellada part del cos examinada.
    /// El trobem als mòduls DX Anatomy (C.8.11.2), Mammography Image (C.8.11.7), Intra-oral Image (C.8.11.9) i Ocular Region Imaged (C.8.17.5)
    /// També el trobem al mòdul Frame Anatomy (C.7.6.16.2.8) comú a tots els enhanced, però el tag s'anomena Frame Laterality en comptes d'Image Laterality.
    /// Valors definits:
    /// R = right
    /// L = left
    /// U = unpaired
    /// B = both left and right
    QChar m_imageLaterality;

    /// Descripció del tipus de vista de la imatge. El seu ús l'aplicarem bàsicament pels casos de mammografia definits a
    /// PS 3.16 - Context ID 4014 (cranio-caudal, medio-lateral oblique, etc...) però podríem extendre el seu ús a d'altres tipus d'imatge
    /// que també fan ús d'aquest tag per guardar aquest tipus d'informació amb altres possibles valors específics.
    QString m_viewCodeMeaning;

    /// Frame number
    int m_frameNumber;

    /// Image phase number
    int m_phaseNumber;

    /// Volume number to which the image in the series belongs
    int m_volumeNumberInSeries;

    /// Image order number within vo
    int m_orderNumberInVolume;

    /// Time the pixel data was created
    QString m_imageTime;

    // TODO C.7.6.5 CINE MODULE: Multi-frame Cine Image

    /// Transfer Syntax UID (0002,0010)
    /// Transfer syntax defines how DICOM objects are serialized.
    QString m_transferSyntaxUID;

    /// NO-DICOM attributes

    /// The absolute path of the image
    QString m_path;

    /// Date the image was downloaded to the local database
    QDate m_retrievedDate;
    QTime m_retrieveTime;

    /// Attribute that will tell us how many overlays the image has
    unsigned short m_numberOfOverlays;

    /// List of loaded overlays
    QList <ImageOverlay> m_overlaysList;

    /// List containing the partition in optimal regions of the merging of all overlays
    QList <ImageOverlay> m_overlaysSplit;

    /// List of display shutters
    QList <DisplayShutter> m_shuttersList;

    /// DisplayShutter per display that we will save once created
    DisplayShutter m_displayShutterForDisplay;

    /// Tells us if we need to create the display shutter by display
    bool m_haveToBuildDisplayShutterForDisplay;

    /// Display shutter for display in vtkImageData format.
    vtkSmartPointer <vtkImageData> m_displayShutterForDisplayVtkImageData;

    /// The parent series
    Series * m_parentSeries;

    /// Cache preview image
    QPixmap m_thumbnail;

    // Indicates the origin of DICOM images
    DICOMSource m_imageDICOMSource;
};

}

#endif
