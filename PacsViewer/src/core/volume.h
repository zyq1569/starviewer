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

#ifndef UDGVOLUME_H
#define UDGVOLUME_H

#include <itkImage.h>

#include "identifier.h"
#include "volumepixeldata.h"
#include "anatomicalplane.h"
#include "orthogonalplane.h"
// Qt
#include <QPixmap>
#include <QVector>
// FWD declarations
class vtkImageData;

namespace udg {

class Image;
class Series;
class Study;
class Patient;
class VolumeReader;
class ImagePlane;

/**
This class represents a volume of data. This will be the class where the data we want to process will be saved.
It will give us methods to get the data in the format we want: ITK, VTK, etc.

It can be initialized with data of type itk or vtk with the method \ c setData ().
For efficiency reasons, as the program will mainly display the data in native vtk format.
They will only be converted to itk when explicitly requested.
*/
class Volume : public QObject {
    Q_OBJECT
public:
    /// TODO: Typedef's duplicates of VolumePixelData, is maintained
    /// for the time being as long as it is not replaced wherever it is referenced
    typedef VolumePixelData::ItkPixelType ItkPixelType;
    static const unsigned int VDimension = VolumePixelData::VDimension;
    typedef VolumePixelData::ItkImageType ItkImageType;
    typedef VolumePixelData::ItkImageTypePointer ItkImageTypePointer;

    Volume(QObject *parent = 0);
    ~Volume();

    /// Assignem/Retornem les dades de pixel data en format ITK
    void setData(ItkImageTypePointer itkImage);
    ItkImageTypePointer getItkData();

    /// Assignem/Retornem les dades de pixel data en format VTK
    void setData(vtkImageData *vtkImage);
    vtkImageData* getVtkData();

    /// Assign / Return the Volume Pixel Data
    /// Assignment does not accept null pointers.
    void setPixelData(VolumePixelData *pixelData);
    VolumePixelData* getPixelData();

    /// Tells us if the data pixel is loaded.
    /// If you don't have it, the methods that ask about volume data can give incorrect answers.
    bool isPixelDataLoaded() const;

    /// Get the source of the volume
    void getOrigin(double xyz[3]);
    double* getOrigin();

    ///Get the model spacing along the axes
    void getSpacing(double xyz[3]);
    double* getSpacing();

    /// Returns bounding box?
    void getExtent(int extent[6]);
    int* getExtent();

    /// Returns the dimensions of the volume
    int* getDimensions();
    void getDimensions(int dims[3]);

    /// Return the range of volume values (minimum and maximum value).
    void getScalarRange(double range[2]);

    /// Assigns / Returns the volume identifier.
    void setIdentifier(const Identifier &id);
    Identifier getIdentifier() const;

    /// Assign / Return the volume thumbnail
    void setThumbnail(const QPixmap &thumbnail);
    QPixmap getThumbnail() const;

    /// TODO Transitional methods for design changes to the phase theme
    void setNumberOfPhases(int phases);
    int getNumberOfPhases() const;
    Volume* getPhaseVolume(int index);
    QList<Image*> getPhaseImages(int index);
    void setNumberOfSlicesPerPhase(int slicesPerPhase);
    int getNumberOfSlicesPerPhase() const;

    /// Returns the modality, if available, of the images that compose the volume
    QString getModality() const;

    /// We add an image to the set of images that make up the volume
    void addImage(Image *image);

    /// We directly assign the set of images that make up this volume
    void setImages(const QList<Image*> &imageList);

    /// Returns the images that make up the volume
    QList<Image*> getImages() const;

    /// Returns the total number of frames the volume has with the Image objects it contains
    int getNumberOfFrames() const;

    /// Returns true if the volume is multiframe.
    bool isMultiframe() const;

    /// Shortcut methods to get the parent series / study / patient which this volume belongs to
    Series* getSeries() const;
    Study* getStudy() const;
    Patient* getPatient() const;

    /// Dump information into a string so you can print it where you want it
    QString toString(bool verbose = false);

    /// It gives us the image corresponding to the given slice and phase
    /// By default, we will only specify the image regardless of the phase
    /// @param sliceNumber slice
    /// @param phaseNumber phase
    /// @return the image in case the indexes are correct, NULL otherwise
    Image* getImage(int sliceNumber, int phaseNumber = 0) const;

    /// Given a slice and an orthogonal plane, returns the corresponding ImagePlane
    /// @param vtkReconstructionHack HACK enables a hack for cases which the "real" plane is not the really wanted
    /// applying a correction to satisfy some restrictions with vtk.
    /// This should only be used on very concrete cases. Default value should be used.
    /// @return The corresponding image plane
    ImagePlane* getImagePlane(int sliceNumber, const OrthogonalPlane &plane, bool vtkReconstructionHack = false);

    /// Returns the pixel units for this volume. If the units cannot be specified, an empty string will be returned
    QString getPixelUnits();
    /// Returns the slice range of the current volume corresponding to an specified orthogonal plane
    void getSliceRange(int &min, int &max, const OrthogonalPlane &plane);

    /// Returns the slice range of the current volume corresponding to a specified orthogonal plane
    int getMaximumSlice(const OrthogonalPlane &plane);
    int getMinimumSlice(const OrthogonalPlane &plane);


    /// It returns the REAL address (DICOM) in which they are stacked
    /// the images that make up the volume. Because within the same volume we can have more
    /// of a frame / stack, we have to indicate from which frame / stack we want the direction
    /// EVERYTHING at the moment, since the support for stacks / frames is quite pathetic,
    /// we assume that there is only one, but this needs to be corrected
    /// You will also need to have at least two images in the same stack / frame to give a reliable direction.
    /// In case we only have one image for the given frame / stack, we will return the normal of that image
    /// which is the closest thing you can get
    /// @param stack
    /// @param direction []
    void getStackDirection(double direction[3], int stack = 0);

    /// Returns a pointer to the raw pixel data at index [x, y, z].
    ///  Avoid its use if possible and prefer using an iterator instead.
    void* getScalarPointer(int x = 0, int y = 0, int z = 0);
    /// Returns a pointer to the raw pixel data at the given index.
    /// Avoid its use if possible and prefer using an iterator instead.
    void* getScalarPointer(int index[3]);

    /// Returns a VolumePixelDataIterator pointing to the voxel at index [x, y, z].
    VolumePixelDataIterator getIterator(int x, int y, int z);
    /// Returns a VolumePixelDataIterator pointing to the first voxel.
    VolumePixelDataIterator getIterator();

    /// Returns value of voxel at index [x, y, z].
    double getScalarValue(int x, int y, int z);

    /// It is responsible for converting the volume to a "minimum" volume to give an output in cases that
    /// we run out of memory or run into other problems. It would become one
    /// neutral volume to prevent the application from crashing in uncontrolled error cases
    /// TODO This method should perhaps be transformed into a subclass of Volume that
    /// only create this type of volume
    void convertToNeutralVolume();

    /// Returns the volume acquisition plan
    /// In case we don't have images, NotAvailable will return.
    /// EVERYTHING Because the volume can be made up of images from different planes, the acquisition plan
    /// is calculated based on the first image and enough
    AnatomicalPlane getAcquisitionPlane() const;

    /// Returns which orthogonal plane of the current volume corresponds to the given anatomical plane
    OrthogonalPlane getCorrespondingOrthogonalPlane(const AnatomicalPlane &anatomicalPlane) const;

    /// Returns the internal image index corresponding to the indicated slice and phase
    int getImageIndex(int sliceNumber, int phaseNumber) const;

    /// Given a world coordinate, it gives us the index of the corresponding voxel.
    /// If the coordinate is within the volume returns true, false otherwise.
    /// EVERYTHING The coordinate is expected to be within the VTK world!
    bool computeCoordinateIndex(const double coordinate[3], int index[3]);

    /// Returns the number of components
    int getNumberOfScalarComponents();

    /// Returns the size of the stairs
    int getScalarSize();

    /// Returns the scalar pointer of the image in the form of QByteArray.
    QByteArray getImageScalarPointer(int imageNumber);

    /// Returns true if all the images in this volume are in the same anatomical plane.
    bool areAllImagesInTheSameAnatomicalPlane() const;

	//zyq20240412
	bool is3Dimage() const;

	bool isMHDImage() const;

signals:
    /// It emits the state of the progress in which the data load of the volume is
    /// @param progress load progress on a scale of 1 to 100
    void progress(int);

private:

    virtual VolumeReader* createVolumeReader();

    /// Lazy loading of the units of the pixels of PT series
    QString getPTPixelUnits(const Image *image);

private:

    /// Set of images that make up the volume
    QList<Image*> m_imageSet;
    /// True if it has been checked that all images are in the same anatomical
    ///  plane since the last change in the image set.
    mutable bool m_checkedImagesAnatomicalPlane;
    /// True if all the images in this volume are in the same anatomical plane.
    mutable bool m_allImagesAreInTheSameAnatomicalPlane;

    /// Volume identifier
    Identifier m_identifier;

    /// Volume Thumbnail
    QPixmap m_thumbnail;

    /// Pixel volume date
    VolumePixelData *m_volumePixelData;

    /// EVERY temporary member for the transition to phase treatment
    int m_numberOfPhases;
    int m_numberOfSlicesPerPhase;

    /// Stores the units of the pixel values of PT series.
    /// getPTPixelUnits should always be used to get this value
    QString m_PTPixelUnits;
};

}  // End namespace udg

#endif
