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

  This file incorporates work covered by the following copyright and
  permission notice:

    Copyright Insight Software Consortium

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

           http://www.apache.org/licenses/LICENSE-2.0.txt

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 *************************************************************************************/

#include "itkDCMTKImageIO.h"

#include "itkByteSwapper.h"
#include "itksys/SystemTools.hxx"
#include "itkDCMTKFileReader.h"
#include <iostream>

#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmjpls/djdecode.h"
#include "dcmtk/dcmdata/dcrledrg.h"
#include "dcmtk/dcmdata/dcfilefo.h"

namespace itk
{
/** Constructor */
DCMTKImageIO::DCMTKImageIO()
{
  m_DImage = NULL;

  // standard ImageIOBase variables
  m_ByteOrder = BigEndian;
  this->SetNumberOfDimensions(3); // otherwise, things go crazy w/dir cosines
  m_PixelType = SCALAR;
  m_ComponentType = UCHAR;
  //m_FileType =

  // specific members
  m_UseJPEGCodec = false;
  m_UseJPLSCodec = false;
  m_UseRLECodec  = false;
  m_DicomImageSetByUser = 0;

  this->AddSupportedWriteExtension(".dcm");
  this->AddSupportedWriteExtension(".DCM");
  this->AddSupportedWriteExtension(".dicom");
  this->AddSupportedWriteExtension(".DICOM");

  // this->AddSupportedReadExtension(".dcm");
  // this->AddSupportedReadExtension(".DCM");
  // this->AddSupportedReadExtension(".dicom");
  // this->AddSupportedReadExtension(".DICOM");

  m_ReadFrameByFrame = false;
}

/** Destructor */
DCMTKImageIO::~DCMTKImageIO()
{
  if(!this->m_DicomImageSetByUser)
    {
    delete m_DImage;
    }
  //DJDecoderRegistration::cleanup();
  //DcmRLEDecoderRegistration::cleanup();
}

bool DCMTKImageIO::CanReadFile(const char *filename)
{
  // First check the filename extension
  std::string fname = filename;

  if ( fname == "" )
    {
    itkDebugMacro(<< "No filename specified.");
    }

  return DCMTKFileReader::IsImageFile(filename);
}

bool DCMTKImageIO::CanWriteFile(const char *name)
{
  std::string fname = name;

  if ( fname == "" )
    {
    itkDebugMacro(<< "No filename specified.");
    }

  bool                   extensionFound = false;
  std::string::size_type dcmPos = fname.rfind(".dcm");
  if ( ( dcmPos != std::string::npos )
       && ( dcmPos == fname.length() - 4 ) )
    {
    extensionFound = true;
    }

  dcmPos = fname.rfind(".DCM");
  if ( ( dcmPos != std::string::npos )
       && ( dcmPos == fname.length() - 4 ) )
    {
    extensionFound = true;
    }

  dcmPos = fname.rfind(".dicom");
  if ( ( dcmPos != std::string::npos )
       && ( dcmPos == fname.length() - 6 ) )
    {
    extensionFound = true;
    }

  dcmPos = fname.rfind(".DICOM");
  if ( ( dcmPos != std::string::npos )
       && ( dcmPos == fname.length() - 6 ) )
    {
    extensionFound = true;
    }

  if ( !extensionFound )
    {
    itkDebugMacro(<< "The filename extension is not recognized");
    return false;
    }

  if ( extensionFound )
    {
    return true;
    }
  return false;
}

void
DCMTKImageIO
::OpenDicomImage()
{
  if(this->m_DImage != 0)
    {
    if( !this->m_DicomImageSetByUser &&
        this->m_FileName != this->m_LastFileName)
      {
      delete m_DImage;
      this->m_DImage = 0;
      }
    }
  if( m_DImage == NULL )
    {
    DcmFileFormat dicomFile;
    OFCondition status = dicomFile.loadFile(m_FileName.c_str());
    if (status.good())
    {
        DcmDataset *dataset = dicomFile.getAndRemoveDataset();
        if (m_ReadFrameByFrame)
        {
            m_DImage = new DicomImage(dataset, dataset->getOriginalXfer(), this->m_RescaleSlope, this->m_RescaleIntercept,
                                      CIF_TakeOverExternalDataset | CIF_UsePartialAccessToPixelData | CIF_UseAbsolutePixelRange, 0, 1);
        }
        else
        {
            m_DImage = new DicomImage(dataset, dataset->getOriginalXfer(), this->m_RescaleSlope, this->m_RescaleIntercept, CIF_TakeOverExternalDataset);
        }
        this->m_LastFileName = this->m_FileName;
    }
    }
  if(this->m_DImage == 0)
    {
    itkExceptionMacro(<< "Can't create DicomImage for "
                      << this->m_FileName)
    }
}
//------------------------------------------------------------------------------
void
DCMTKImageIO
::Read(void *buffer)
{
  this->OpenDicomImage();
  if (m_DImage->getStatus() == EIS_Normal)
    {
    m_Dimensions[0] = (unsigned int)(m_DImage->getWidth());
    m_Dimensions[1] = (unsigned int)(m_DImage->getHeight());
    // m_Spacing[0] =
    // m_Spacing[1] =
    // m_Origin[0] =
    // m_Origin[1] =

    // pick a size for output image (should get it from DCMTK in the ReadImageInformation()))
    // NOTE ALEX: EP_Representation is made for that
    // but i don t know yet where to fetch it from
    size_t scalarSize = 0;
    switch(this->m_ComponentType)
      {
      case UCHAR:
        scalarSize = sizeof(unsigned char);
        break;
      case CHAR:
        scalarSize = sizeof(char);
        break;
      case USHORT:
        scalarSize = sizeof(unsigned short);
        break;
      case SHORT:
        scalarSize = sizeof(short);
        break;
      case UINT:
        scalarSize = sizeof(unsigned int);
        break;
      case INT:
        scalarSize = sizeof(int);
        break;
      case ULONG:
        scalarSize = sizeof(unsigned long);
        break;
      case LONG:
        scalarSize = sizeof(long);
        break;
      case UNKNOWNCOMPONENTTYPE:
      case FLOAT:
      case DOUBLE:
        itkExceptionMacro(<< "Bad component type" <<
                          ImageIOBase::GetComponentTypeAsString(this->m_ComponentType));
        break;
      }
    size_t voxelSize(scalarSize);
    switch(this->m_PixelType)
      {
      case VECTOR:
        voxelSize *= this->GetNumberOfComponents();
        break;
      case RGB:
        voxelSize *= 3;
        break;
      case RGBA:
        voxelSize *= 4;
        break;
      default:
        voxelSize *= 1;
        break;
      }
    // get the image in the DCMTK buffer
    size_t readBytes = 0;
    do
    {
        const DiPixel * const interData = m_DImage->getInterData();
        memcpy(static_cast<char*>(buffer) + readBytes, interData->getData(), interData->getCount() * voxelSize);
        readBytes += interData->getCount() * voxelSize;
    }
    while (m_DImage->processNextFrames());
    }
  else
    {
    std::cerr << "Error: cannot load DICOM image (";
    std::cerr << DicomImage::getString(m_DImage->getStatus());
    std::cerr << ")" << std::endl;
    }

}

/**
 *  Read Information about the DICOM file
 */
void DCMTKImageIO::ReadImageInformation()
{

  DJDecoderRegistration::registerCodecs();
  DcmRLEDecoderRegistration::registerCodecs();

  DCMTKFileReader reader;
  reader.SetFileName(this->m_FileName);
  try
    {
    reader.LoadFile();
    }
  catch(...)
    {
    std::cerr << "DCMTKImageIO::ReadImageInformation: "
              << "DicomImage could not read the file." << std::endl;
    }
  unsigned short rows,columns;
  reader.GetDimensions(rows,columns);
  this->m_Dimensions[0] = columns;
  this->m_Dimensions[1] = rows;
  this->m_Dimensions[2] = reader.GetFrameCount();

  vnl_vector<double> dir1(3),dir2(3),dir3(3);
  reader.GetDirCosines(dir1,dir2,dir3);
  this->SetDirection(0,dir1);
  this->SetDirection(1,dir2);
  if(this->m_NumberOfDimensions > 2)
    {
    this->SetDirection(2,dir3);
    }
  // get slope and intercept
  reader.GetSlopeIntercept(this->m_RescaleSlope,this->m_RescaleIntercept);
  this->m_ComponentType = reader.GetImageDataType();
  this->m_PixelType = reader.GetImagePixelType();
  double spacing[3];
  double origin[3];
  reader.GetSpacing(spacing);
  reader.GetOrigin(origin);
  this->m_Origin.resize(3);
  for(unsigned i = 0; i < 3; i++)
    {
    this->m_Origin[i] = origin[i];
    }

  this->m_Spacing.clear();
  for(unsigned i = 0; i < 3; i++)
    {
    this->m_Spacing.push_back(spacing[i]);
    }

  this->OpenDicomImage();
  const DiPixel *interData = this->m_DImage->getInterData();

  if(interData == 0)
    {
    itkExceptionMacro(<< "Missing Image Data in "
                      << this->m_FileName);
    }

  EP_Representation pixelRep = this->m_DImage->getInterData()->getRepresentation();
  switch(pixelRep)
    {
    case EPR_Uint8:
      this->m_ComponentType = UCHAR; break;
    case EPR_Sint8:
      this->m_ComponentType = CHAR; break;
    case EPR_Uint16:
      this->m_ComponentType = USHORT; break;
    case EPR_Sint16:
      this->m_ComponentType = SHORT; break;
    case EPR_Uint32:
      this->m_ComponentType = UINT; break;
    case EPR_Sint32:
      this->m_ComponentType = INT; break;
    default: // HACK should throw exception
      this->m_ComponentType = USHORT; break;
    }
  int numPlanes = this->m_DImage->getInterData()->getPlanes();
  switch(numPlanes)
    {
    case 1:
      this->m_PixelType = SCALAR; break;
    case 2:
      // hack, supposedly Luminence/Alpha
      this->SetNumberOfComponents(2);
      this->m_PixelType = VECTOR; break;
      break;
    case 3:
      this->m_PixelType = RGB; break;
    case 4:
      this->m_PixelType = RGBA; break;
    }
}

bool DCMTKImageIO::GetReadFrameByFrame() const
{
    return m_ReadFrameByFrame;
}

void DCMTKImageIO::SetReadFrameByFrame(bool readFrameByFrame)
{
    m_ReadFrameByFrame = readFrameByFrame;
}

void
DCMTKImageIO
::WriteImageInformation(void)
{}

/** */
void
DCMTKImageIO
::Write(const void *buffer)
{
  (void)(buffer);
}

/** Print Self Method */
void DCMTKImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace itk
