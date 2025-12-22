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

#include "thumbnailcreator.h"

#include <QObject>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QPainter>

#include "series.h"
#include "image.h"
#include "logging.h"
#include "dicomtagreader.h"
// We use dcmtk for scaling dicom images
#include <dcmimage.h>
#include <ofbmanip.h>
#include <dcdatset.h>

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmjpeg/djdecode.h"    /* for dcmjpeg decoders */

//#include "../fmjpeg2k/fmjpeg2k/djencode.h"
// Needed to support color images
#include <diregist.h>

//itk
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkGDCMImageIO.h>
#include <itkTileImageFilter.h>
#include <itkJPEGImageIOFactory.h>
//
namespace udg {

const QString PreviewNotAvailableText(QObject::tr("Preview image not available"));

QImage ThumbnailCreator::getThumbnail(const Series *series, int resolution)
{
    QImage thumbnail;

    if (series->getModality() == "KO")
    {
        thumbnail = createIconThumbnail(":/images/icons/mime-ko.svg", resolution);
    }
    else if (series->getModality() == "PR")
    {
        thumbnail = createIconThumbnail(":/images/icons/mime-ps.svg", resolution);
    }
    else if (series->getModality() == "SR" || (!series->hasImages() && series->hasEncapsulatedDocuments()))
    {
        thumbnail = createIconThumbnail(":/images/icons/mime-sr.svg", resolution);
    }
    else
    {
        int numberOfImages = series->getImages().size();
        if (numberOfImages > 0)
        {
            thumbnail = createImageThumbnail(series->getImages()[numberOfImages / 2]->getPath(), resolution);
        }
        else
        {
            thumbnail = createIconThumbnail(":/images/icons/mime-unknown.svg", resolution);

            // If the series does not contain images in the thumbnail we will indicate it
            //thumbnail = makeEmptyThumbnailWithCustomText(QObject::tr("No Images Available"));
        }
    }

    return thumbnail;
}

QImage ThumbnailCreator::getThumbnail(const Image *image, int resolution)
{
    return createImageThumbnail(image->getPath(), resolution);
}

QImage ThumbnailCreator::getThumbnail(const DICOMTagReader *reader, int resolution)
{
    return createThumbnail(reader, resolution);
}

QImage ThumbnailCreator::makeEmptyThumbnailWithCustomText(const QString &text,  const DICOMTagReader *reader, int resolution)
{
    QImage thumbnail;
	
	if (reader)
    {
        QString dcmfileName = reader->getFileName();

        using InputPixelType = signed short;
        const unsigned int InputDimension = 2;
        using InputImageType = itk::Image<InputPixelType, InputDimension>;
        //创建reader，设置读取的文件名
        using ReaderType = itk::ImageFileReader<InputImageType>;
        ReaderType::Pointer reader = ReaderType::New();
        //
        reader->SetFileName(dcmfileName.toLatin1().data());
        //创建读取DCM的GDCMIOImage类
        using ImageIOType = itk::GDCMImageIO;
        ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
        reader->SetImageIO(gdcmImageIO);
        //调用Update()触发过程,放置在try-catch模块
        try
        {
            reader->Update();
        }
        catch (itk::ExceptionObject& e)
        {
            std::cerr << "exception in file reader" << std::endl;
            std::cerr << e << std::endl;

            thumbnail = QImage(resolution, resolution, QImage::Format_RGB32);
            thumbnail.fill(Qt::black);

            QPainter painter(&thumbnail);
            painter.setPen(Qt::white);
            painter.drawText(0, 0, resolution, resolution, Qt::AlignCenter | Qt::TextWordWrap, text);

            return thumbnail;
        }
        using WriterPixelType = unsigned char;
        using WriteImageType = itk::Image<WriterPixelType, 2>;
        using RescaleFilterType = itk::RescaleIntensityImageFilter<InputImageType, WriteImageType>;
        RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
        rescaler->SetOutputMinimum(0);
        rescaler->SetOutputMaximum(255);
        //再次创建一个writer，写入到文件,这个时候文件变成了一个灰度图
        using Writer2Type = itk::ImageFileWriter<WriteImageType>;
        Writer2Type::Pointer writer2 = Writer2Type::New();
        QString jpegpath = dcmfileName+".jpg";
        writer2->SetFileName(jpegpath.toLatin1().data());
        rescaler->SetInput(reader->GetOutput());
        writer2->SetInput(rescaler->GetOutput());
        itk::JPEGImageIOFactory::RegisterOneFactory();
        try
        {
            writer2->Update();
			if (thumbnail.load(jpegpath))
			{
				if (thumbnail.height() < thumbnail.width())
				{
					return thumbnail.scaledToWidth(96);
				}
				else
				{
					return thumbnail.scaledToHeight(96);
				}
			}
        }
        catch (itk::ExceptionObject& e)
        {
            std::cerr << "Exception in file writer " << std::endl;
            std::cerr << e << std::endl;
        }
		ERROR_LOG("Failed to generate thumbnail: ITK");
    }
	
    thumbnail = QImage(resolution, resolution, QImage::Format_RGB32);
    thumbnail.fill(Qt::black);

    QPainter painter(&thumbnail);
    painter.setPen(Qt::white);
    painter.drawText(0, 0, resolution, resolution, Qt::AlignCenter | Qt::TextWordWrap, text);

    return thumbnail;
}

QImage ThumbnailCreator::createImageThumbnail(const QString &imageFileName, int resolution)
{
	if (imageFileName.right(3).toUpper().contains("MHD"))
	{
		QImage thumbnail;
		thumbnail = QImage(resolution, resolution, QImage::Format_RGB32);
		thumbnail.fill(Qt::black);
		QPainter painter(&thumbnail);
		painter.setPen(Qt::white);
		painter.drawText(0, 0, resolution, resolution, Qt::AlignCenter | Qt::TextWordWrap, "MHD image:    Preview image not available");
		return thumbnail;
	}
    DICOMTagReader reader(imageFileName);
    return createThumbnail(&reader, resolution);
}

QImage ThumbnailCreator::createIconThumbnail(const QString &iconFileName, int resolution)
{
    QImage thumbnail;
    QIcon icon = QIcon(iconFileName);
    thumbnail = icon.pixmap(resolution).toImage();
    return thumbnail;
}

QImage ThumbnailCreator::createThumbnail(const DICOMTagReader *reader, int resolution)
{
	resolution = 256;
    QImage thumbnail;

    if (isSuitableForThumbnailCreation(reader))
    {
        try
        {
            /// Let's load the dicom file to scale
            /// Let's say that in the case of a multiframe image,
            /// just load the first image and enough, saving unnecessarily hosting memory
            /// /// JPEG 2000 (lossless)
            //EXS_JPEG2000LosslessOnly = 26,
            ///// JPEG 2000 (lossless or lossy)
            //EXS_JPEG2000 = 27,
            ///// JPEG 2000 part 2 multi-component extensions (lossless)
            //EXS_JPEG2000MulticomponentLosslessOnly = 28,
            ///// JPEG 2000 part 2 multi-component extensions (lossless or lossy)
            //EXS_JPEG2000Multicomponent = 29,
            DicomImage *dicomImage = NULL;
			dicomImage = new DicomImage(qPrintable(reader->getFileName()));
			if (dicomImage)
			{
				OFString value;
				reader->getDcmDataset()->findAndGetOFString(DCM_SeriesDescription, value);
				QString vl = value.c_str();		
				dicomImage->hideAllOverlays();
				if (vl.length() > 1 && vl.toLower().contains("report"))
				{
					dicomImage->setWindow(-2, 1);
					thumbnail = createThumbnail(dicomImage, 512);
				}
				else
				{
					dicomImage->setMinMaxWindow(1);
					thumbnail = createThumbnail(dicomImage, resolution);
				}
			}
            // DicomImage must be deleted to avoid memory leaks
            if (dicomImage)
            {
                delete dicomImage;
                dicomImage = NULL;
            }
        }
        catch (std::bad_alloc &e)
        {
            ERROR_LOG(QString("Failed to generate thumbnail due to memory failure: %1").arg(e.what()));
            thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText, reader);
        }
    }
    else
    {
        //We create an alternative thumbnail indicating that a preview image cannot be displayed
        thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText, reader);
    }

    return thumbnail;
}

QImage ThumbnailCreator::createThumbnail(DicomImage *dicomImage, int resolution)
{
    QImage thumbnail;
    bool ok = false;

    if (dicomImage == NULL)
    {
        ok = false;
        DEBUG_LOG("Insufficient memory to load DICOM image when making thumbnail or null pointer");
    }
    else if (dicomImage->getStatus() == EIS_Normal)
    {
        //dicomImage->hideAllOverlays();
        //dicomImage->setMinMaxWindow(1);

        // We scale the image
        DicomImage *scaledImage;
        // We climb the biggest corner
        unsigned long width, height;
        if (dicomImage->getWidth() < dicomImage->getHeight())
        {
            width = 0;
            height = resolution;
        }
        else
        {
            width = resolution;
            height = 0;
        }
        scaledImage = dicomImage->createScaledImage(width, height, 1, 1);
        if (scaledImage == NULL)
        {
            ok = false;
            DEBUG_LOG("Scaled image returned as null");
        }
        else if (scaledImage->getStatus() == EIS_Normal)
        {
            QPixmap pixmap = convertToQPixmap(scaledImage);
            if (pixmap.isNull())
            {
                DEBUG_LOG("Could not convert DicomImage to QImage. A Preview not available thumbnail is created.");
                ok = false;
            }
            else
            {
                // The smallest side will be of "resolution" size.
                pixmap = pixmap.scaled(resolution,resolution, Qt::AspectRatioMode::KeepAspectRatioByExpanding, Qt::TransformationMode::SmoothTransformation);

                // By cropping the longer side, a squared image is made.
                int width = pixmap.width();
                int height = pixmap.height();
                if (width > height) // heigth == resolution
                {
                    pixmap = pixmap.copy((width-resolution) / 2, 0, height, height);
                }
                else if (height > width) // width == resolution
                {
                    pixmap = pixmap.copy(0, (height-resolution) / 2, width, width);
                }
                else
                {
                    // A perfect square, nothing to do
                }

                thumbnail = pixmap.toImage();
                ok = true;
            }

            // DicomImage must be deleted to avoid memory leaks
            delete scaledImage;
        }
        else
        {
            ok = false;
            DEBUG_LOG(QString("The scaled image has errors. Error: %1 ").arg(DicomImage::getString(scaledImage->getStatus())));
        }
    }
    else
    {
        ok = false;
        DEBUG_LOG(QString("Error loading the DicomImage. Error: %1 ").arg(DicomImage::getString(dicomImage->getStatus())));
    }

    //If we were unable to generate the thumbnail, we create a blank one
    if (!ok)
    {
        thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText);
    }

    return thumbnail;
}

bool ThumbnailCreator::isSuitableForThumbnailCreation(const DICOMTagReader *reader) const
{
    if (!reader)
    {
        DEBUG_LOG("The given DICOMTagReader is NULL!");
        return false;
    }

    if (!reader->getDcmDataset())
    {
        DEBUG_LOG("DICOMTagReader has no DcmDataset assigned, we cannot generate the thumbnail.");
        return false;
    }

    return true;
}

QPixmap ThumbnailCreator::convertToQPixmap(DicomImage *dicomImage)
{
    Q_ASSERT(dicomImage);

    // The following code creates a PGM or PPM image in memory and we load this buffer directly into the QImage
    // Based on the code of http://forum.dcmtk.org/viewtopic.php?t=120&highlight=qpixmap

    int bytesPerComponent;
    QString imageFormat;
    QString imageHeader;
    if (dicomImage->isMonochrome())
    {
        imageHeader = "P5";
        bytesPerComponent = 1;
        imageFormat = "PGM";
    }
    else
    {
        imageHeader = "P6";
        bytesPerComponent = 3;
        imageFormat = "PPM";
    }

    // Create PGM/PPM header
    const int width = (int)(dicomImage->getWidth());
    const int height = (int)(dicomImage->getHeight());
    imageHeader += QString("\n%1 %2\n255\n").arg(width).arg(height);

    // QPixmap in which we will load the data buffer
    QPixmap thumbnail;
    // Create output buffer for DicomImage class
    const int offset = imageHeader.size();
    const unsigned int length = (width * height) * bytesPerComponent + offset;
    Uint8 *buffer = new Uint8[length];
    if (buffer != NULL)
    {
        // Copy PGM/PPM header to buffer
        OFBitmanipTemplate<Uint8>::copyMem((const Uint8*)imageHeader.toLatin1().data(), buffer, offset);
        if (dicomImage->getOutputData((void*)(buffer + offset), length, 8))
        {
            if (!thumbnail.loadFromData((const unsigned char*)buffer, length, imageFormat.toLatin1()))
            {
                DEBUG_LOG("Buffer upload to thumbnail failed :(");
            }
            //thumbnail.save("E:/test.png");
        }
        // Delete temporary pixel buffer
        delete[] buffer;
    }
    else
    {
        DEBUG_LOG("Insufficient memory to create thumbnail buffer!");
    }

    return thumbnail;
}

};
