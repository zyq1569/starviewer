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

#ifndef UDGVOLUMEPIXELDATA_H
#define UDGVOLUMEPIXELDATA_H

#include <QObject>
#include <QVector>

#include <itkImage.h>
#include <vtkSmartPointer.h>
// Els filtres per passar itk<=>vtk: InsightApplications/auxiliary/vtk --> ho tenim a /tools

// Converts an ITK image into a VTK image and plugs a itk data pipeline to a VTK datapipeline.
#include "itkImageToVTKImageFilter.h"
// Converts a VTK image into an ITK image and plugs a vtk data pipeline to an ITK datapipeline.
#include "itkVTKImageToImageFilter.h"

class vtkImageData;

namespace udg {

class VolumePixelDataIterator;
class Voxel;

/**
Class whose responsibility is to maintain the date pixel of a Volume.
The date pixel of a volume is the memory location where the different voxel values of a Volume are stored.
*/
class VolumePixelData {

public:

    ///Default internal image type of itk
    typedef signed short int ItkPixelType;
    static const unsigned int VDimension = 3;

    typedef itk::Image<ItkPixelType, VDimension> ItkImageType;
    typedef ItkImageType::Pointer ItkImageTypePointer;

    VolumePixelData();

    /// Assignem/Retornem les dades en format ITK
    void setData(ItkImageTypePointer itkImage);
    ItkImageTypePointer getItkData();

    /// Assignem/Retornem les dades en format VTK
    void setData(vtkImageData *vtkImage);
    vtkImageData* getVtkData();

    /// We create the data from an unsigned chars buffer
    /// The extent must be consistent with the size of the buffer data and the indicated ByPixel bytes
    /// If deleteData = true, this class will be responsible for destroying the buffer when this object is destroyed
    /// If deleteData = false, (by default) it will not clear the buffer
    /// Spacing and origin characteristics will not be assigned with this method.
    /// This must be done by later accessing the vtk data
    void setData(unsigned char *data, int extent[6], int bytesPerPixel, bool deleteData = false);

    /// Sets the number of phases of this pixel data.
    /// This information is needed to be able to access to the right pixels when accessing through world coordinate
    /// The minimum value must be 1, is less than, the method will do nothing
    void setNumberOfPhases(int numberOfPhases);
    
    /// Retorna cert si conté dades carregades.
    bool isLoaded() const;

    /// Returns a pointer to the raw pixel data at index [x, y, z]. Avoid its use if possible and prefer using an iterator instead.
    void* getScalarPointer(int x, int y, int z);
    /// Returns a pointer to the raw pixel data. Avoid its use if possible and prefer using an iterator instead.
    void* getScalarPointer();

    /// Returns a VolumePixelDataIterator pointing to the voxel at index [x, y, z].
    VolumePixelDataIterator getIterator(int x, int y, int z);
    /// Returns a VolumePixelDataIterator pointing to the first voxel.
    VolumePixelDataIterator getIterator();

    /// Given a world coordinate, it gives us the index of the corresponding voxel.
    /// If the coordinate is within the volume returns true, false otherwise.
    /// EVERYTHING The coordinate is expected to be within the VTK world!
    /// It should be determined whether it should be like this or it should be DICOM or another system.
    /// HACK The phaseNumber parameter is needed to be able to calculate the correct index within the volume corresponding to the current phase
    bool computeCoordinateIndex(const double coordinate[3], int index[3], int phaseNumber = 0);

    /// Given a world coordinate, it gives us the value of the corresponding voxel.
    /// TODO: The coordinate is expected to be within the VTK world!
    /// It should be determined whether it should be like this or it should be DICOM or another system.
    /// HACK The phaseNumber parameter is needed to be able to calculate the correct index within the volume corresponding to the current phase
    Voxel getVoxelValue(double coordinate[3], int phaseNumber = 0);

    /// Returns the voxel corresponding to the given index. If index is out of range, a default constructed value will be returned.
    Voxel getVoxelValue(int index[3]);

    /// It is responsible for converting VolumePixelData into a neutral data pixel that allows it to be used in cases in
    /// those who run out of memory or encounter other problems when trying to host one in memory
    void convertToNeutralPixelData();

    ///Assign / Obtain the source
    void setOrigin(double origin[3]);
    void setOrigin(double x, double y, double z);
    void getOrigin(double origin[3]);

    /// Assign / Obtain the source
    void setSpacing(double spacing[3]);
    void setSpacing(double x, double y, double z);
    void getSpacing(double spacing[3]);

    /// Returns the extension
    void getExtent(int extent[6]);

    /// Returns the number of scalar components
    int getNumberOfScalarComponents();

    /// Returns the number of scalar components
    int getScalarSize();
    /// Get the type of scalar
    int getScalarType();

    // Get the number of points
    int getNumberOfPoints();
   
private:
    /// Filtres per importar/exportar
    typedef itk::ImageToVTKImageFilter<ItkImageType> ItkToVtkFilterType;
    typedef itk::VTKImageToImageFilter<ItkImageType> VtkToItkFilterType;

    /// The data in vtk format
    vtkSmartPointer<vtkImageData> m_imageDataVTK;

    /// Indicates whether it contains loaded data or not.
    bool m_loaded;

    /// Number of phases of the pixel data. Its minimum value must be 1
    int m_numberOfPhases;

    /// Filters to switch from vtk to itk
    ItkToVtkFilterType::Pointer m_itkToVtkFilter;
    VtkToItkFilterType::Pointer m_vtkToItkFilter;
};

}

#endif // UDGVOLUMEPIXELDATA_H
