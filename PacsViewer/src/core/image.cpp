﻿/*************************************************************************************
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

#include "image.h"

#include "series.h"
#include "logging.h"
#include "thumbnailcreator.h"
#include "mathtools.h"
#include "imageoverlayreader.h"
#include "preferredpixelspacingselector.h"

#include <QFileInfo>

#include <vtkImageData.h>

namespace udg {

Image::Image(QObject *parent)
    : QObject(parent), m_sliceThickness(0.0), m_samplesPerPixel(1), m_photometricInterpretation("MONOCHROME2"), m_rows(0), m_columns(0), m_bitsAllocated(16),
      m_bitsStored(16), m_pixelRepresentation(0), m_rescaleSlope(1), m_rescaleIntercept(0), m_frameNumber(0), m_phaseNumber(0), m_volumeNumberInSeries(0),
      m_orderNumberInVolume(0), m_overlaysSplitComputed(false), m_parentSeries(NULL)
{
    m_estimatedRadiographicMagnificationFactor = 1.0;
    
    m_numberOfOverlays = 0;
    memset(m_imagePositionPatient, 0, 3 * sizeof(double));

    m_haveToBuildDisplayShutterForDisplay = false;
}

Image::~Image()
{
}

void Image::setSOPInstanceUID(const QString &uid)
{
    m_SOPInstanceUID = uid;
}

QString Image::getSOPInstanceUID() const
{
    return m_SOPInstanceUID;
}

void Image::setInstanceNumber(const QString &number)
{
    m_instanceNumber = number;
}

QString Image::getInstanceNumber() const
{
    return m_instanceNumber;
}

void Image::setImageOrientationPatient(const ImageOrientation &imageOrientation)
{
    m_imageOrientationPatient = imageOrientation;
}

ImageOrientation Image::getImageOrientationPatient() const
{
    return m_imageOrientationPatient;
}

void Image::setPatientOrientation(const PatientOrientation &orientation)
{
    m_patientOrientation = orientation;
}

PatientOrientation Image::getPatientOrientation() const
{
    return m_patientOrientation;
}

void Image::setPixelSpacing(double x, double y)
{
    m_pixelSpacing.setX(x);
    m_pixelSpacing.setY(y);
}

void Image::setPixelSpacing(const PixelSpacing2D &spacing)
{
    m_pixelSpacing = spacing;
}

PixelSpacing2D Image::getPixelSpacing() const
{
    return m_pixelSpacing;
}

void Image::setImagerPixelSpacing(double x, double y)
{
    m_imagerPixelSpacing.setX(x);
    m_imagerPixelSpacing.setY(y);
}

void Image::setImagerPixelSpacing(const PixelSpacing2D &spacing)
{
    m_imagerPixelSpacing = spacing;
}

PixelSpacing2D Image::getImagerPixelSpacing() const
{
    return m_imagerPixelSpacing;
}

void Image::setEstimatedRadiographicMagnificationFactor(double x)
{
    m_estimatedRadiographicMagnificationFactor = x;
}

double Image::getEstimatedRadiographicMagnificationFactor() const
{
    return m_estimatedRadiographicMagnificationFactor;
}

PixelSpacing2D Image::getPreferredPixelSpacing() const
{
    PreferredPixelSpacingSelector preferredPixelSpacingSelector;
    return preferredPixelSpacingSelector.getPreferredPixelSpacing(this);
}

void Image::setSliceThickness(double z)
{
    m_sliceThickness = z;
}

double Image::getSliceThickness() const
{
    return m_sliceThickness;
}

void Image::setImagePositionPatient(double position[3])
{
    memcpy(m_imagePositionPatient, position, 3 * sizeof(double));
}

const double* Image::getImagePositionPatient() const
{
    return m_imagePositionPatient;
}

void Image::setSamplesPerPixel(int samples)
{
    m_samplesPerPixel = samples;
}

int Image::getSamplesPerPixel() const
{
    return m_samplesPerPixel;
}

void Image::setPhotometricInterpretation(const QString &value)
{
    m_photometricInterpretation.setValue(value.trimmed());
}

PhotometricInterpretation Image::getPhotometricInterpretation() const
{
    return m_photometricInterpretation;
}

void Image::setRows(int rows)
{
    m_rows = rows;
}

int Image::getRows() const
{
    return m_rows;
}

void Image::setColumns(int columns)
{
    m_columns = columns;
}

int Image::getColumns() const
{
    return m_columns;
}

void Image::setBitsAllocated(int bits)
{
    m_bitsAllocated = bits;
}

int Image::getBitsAllocated() const
{
    return m_bitsAllocated;
}

void Image::setBitsStored(int bits)
{
    m_bitsStored = bits;
}

int Image::getBitsStored() const
{
    return m_bitsStored;
}

void Image::setHighBit(int highBit)
{
    m_highBit = highBit;
}

int Image::getHighBit() const
{
    return m_highBit;
}

void Image::setPixelRepresentation(int representation)
{
    m_pixelRepresentation = representation;
}

int Image::getPixelRepresentation() const
{
    return m_pixelRepresentation;
}

void Image::setRescaleSlope(double slope)
{
    m_rescaleSlope = slope;
}

double Image::getRescaleSlope() const
{
    return m_rescaleSlope;
}

void Image::setRescaleIntercept(double intercept)
{
    m_rescaleIntercept = intercept;
}

double Image::getRescaleIntercept() const
{
    return m_rescaleIntercept;
}

void Image::setSliceLocation(const QString &sliceLocation)
{
    m_sliceLocation = sliceLocation;
}

QString Image::getSliceLocation() const
{
    return m_sliceLocation;
}

void Image::addVoiLut(const VoiLut &voiLut)
{
    if (voiLut.isWindowLevel() && !voiLut.getWindowLevel().isValid())
    {
        QString logMessage = QString("WW / WL Inconsistent: %1, %2. It will not be added to the image").arg(voiLut.getWindowLevel().getWidth())
                .arg(voiLut.getWindowLevel().getCenter());
        WARN_LOG(logMessage);
        DEBUG_LOG(logMessage);
    }
    else
    {
        m_voiLutList << voiLut;
    }
}

VoiLut Image::getVoiLut(int index) const
{
    if (index >= 0 && index < m_voiLutList.size())
    {
        return m_voiLutList.at(index);
    }
    else
    {
        DEBUG_LOG("Index out of range");
        return VoiLut();
    }
}

void Image::setVoiLutList(const QList<VoiLut> &voiLutList)
{
    m_voiLutList.clear();
    foreach (const VoiLut &voiLut, voiLutList)
    {
        addVoiLut(voiLut);
    }
}

int Image::getNumberOfVoiLuts() const
{
    return m_voiLutList.size();
}

void Image::setRetrievedDate(QDate retrievedDate)
{
    m_retrievedDate = retrievedDate;
}

void Image::setRetrievedTime(QTime retrievedTime)
{
    m_retrieveTime = retrievedTime;
}

QDate Image::getRetrievedDate() const
{
    return m_retrievedDate;
}

QTime Image::getRetrievedTime() const
{
    return m_retrieveTime;
}

const QString& Image::getAcquisitionNumber() const
{
    return m_acquisitionNumber;
}

void Image::setAcquisitionNumber(QString acquisitionNumber)
{
    m_acquisitionNumber = std::move(acquisitionNumber);
}

void Image::setImageType(const QString &imageType)
{
    m_imageType = imageType;
}

QString Image::getImageType() const
{
    return m_imageType;
}

void Image::setViewPosition(const QString &viewPosition)
{
    m_viewPosition = viewPosition;
}

QString Image::getViewPosition() const
{
    return m_viewPosition;
}

void Image::setImageLaterality(const QChar &imageLaterality)
{
    m_imageLaterality = imageLaterality;
}

QChar Image::getImageLaterality() const
{
    return m_imageLaterality;
}

void Image::setViewCodeMeaning(const QString &viewCodeMeaning)
{
    m_viewCodeMeaning = viewCodeMeaning;
}

QString Image::getViewCodeMeaning() const
{
    return m_viewCodeMeaning;
}

void Image::setFrameNumber(int frameNumber)
{
    m_frameNumber = frameNumber;
}

int Image::getFrameNumber() const
{
    return m_frameNumber;
}

void Image::setPhaseNumber(int phaseNumber)
{
    m_phaseNumber = phaseNumber;
}

int Image::getPhaseNumber() const
{
    return m_phaseNumber;
}

void Image::setVolumeNumberInSeries(int volumeNumberInSeries)
{
    m_volumeNumberInSeries = volumeNumberInSeries;
}

int Image::getVolumeNumberInSeries() const
{
    return m_volumeNumberInSeries;
}

void Image::setOrderNumberInVolume(int orderNumberInVolume)
{
    m_orderNumberInVolume = orderNumberInVolume;
}

int Image::getOrderNumberInVolume() const
{
    return m_orderNumberInVolume;
}

void Image::setImageTime(const QString &imageTime)
{
    m_imageTime = imageTime;
}

QString Image::getImageTime() const
{
    return m_imageTime;
}

QString Image::getFormattedImageTime() const
{
    QString formattedTime = m_imageTime;
    if (!formattedTime.isEmpty())
    {
        /// We follow the suggestion in Table 6.2-1 of Part 5 of the
                /// DICOM standard to take into account the format hh: mm: ss.frac
        formattedTime = formattedTime.remove(":");

        QStringList split = formattedTime.split(".");
        QTime convertedTime = QTime::fromString(split[0], "hhmmss");

        /// It has fraction at the end
        if (split.size() == 2)
        {
            ///We truncate to milliseconds and not to millionths of a second
            convertedTime = convertedTime.addMSecs(split[1].leftJustified(3, '0', true).toInt());
        }
        formattedTime = convertedTime.toString("HH:mm:ss");
    }

    return formattedTime;
}

void Image::setTransferSyntaxUID(const QString &transferSyntaxUID)
{
    m_transferSyntaxUID = transferSyntaxUID;
}

const QString& Image::getTransferSyntaxUID() const
{
    return m_transferSyntaxUID;
}

double Image::distance(Image *image)
{
    ///Distance calculation (based on Jolinda Smith's algorithm)
    double distance = 0.0;
    
    ///Origin of the plan
    const double *imagePosition = image->getImagePositionPatient();
    ///Normal of the plan on which we will project the origin
    QVector3D normalVector = image->getImageOrientationPatient().getNormalVector();
    distance = normalVector.x() * imagePosition[0] + normalVector.y() * imagePosition[1] + normalVector.z() * imagePosition[2];

    return distance;
}

bool Image::hasOverlays() const
{
    return m_numberOfOverlays > 0 ? true : false;
}

unsigned short Image::getNumberOfOverlays() const
{
    return m_numberOfOverlays;
}

void Image::setNumberOfOverlays(unsigned short overlays)
{
    m_numberOfOverlays = overlays;
}

QList<ImageOverlay> Image::getOverlays()
{
    if (hasOverlays())
    {
        // If the number of overlays we have assigned does not match the number of items in the list
        // means we haven't loaded them yet
        if (getNumberOfOverlays() != m_overlaysList.count())
        {
            readOverlays(false);
        }
    }

    return m_overlaysList;
}

QList<ImageOverlay> Image::getOverlaysSplit()
{
    if (hasOverlays())
    {
        if (!m_overlaysSplitComputed)
        {
            readOverlays(true);
            m_overlaysSplitComputed = true;
        }
    }

    return m_overlaysSplit;
}

bool Image::hasDisplayShutters() const
{
    return !m_shuttersList.isEmpty();
}

void Image::addDisplayShutter(const DisplayShutter &shutter)
{
    m_shuttersList << shutter;
    m_haveToBuildDisplayShutterForDisplay = true;
}

void Image::setDisplayShutters(const QList<DisplayShutter> &shuttersList)
{
    m_shuttersList = shuttersList;
    m_haveToBuildDisplayShutterForDisplay = true;
}

QList<DisplayShutter> Image::getDisplayShutters() const
{
    return m_shuttersList;
}

DisplayShutter Image::getDisplayShutterForDisplay()
{
    if (!hasDisplayShutters())
    {
        return DisplayShutter();
    }
    
    if (!m_haveToBuildDisplayShutterForDisplay)
    {
        return m_displayShutterForDisplay;
    }

    /// If we get this far, then it means we need to build the DisplayShutter for display

    /// First we remove the shutters that don't make sense to apply
    /// what would be the rectangular shutters that have a size equal
    /// to or larger than the image and those that have no defined shape
    QList<DisplayShutter> shutterList = this->getDisplayShutters();
    QRect imageRect(1, 1, this->getColumns(), this->getRows());
    for (int i = 0; i < shutterList.count(); ++i)
    {
        const DisplayShutter &shutter = shutterList.at(i);
        if (shutter.getShape() == DisplayShutter::RectangularShape)
        {
            QPolygon points = shutter.getAsQPolygon();
            QRect shutterRect(points.at(0), points.at(2));
            if (imageRect.intersected(shutterRect).contains(imageRect, false))
            {
                shutterList.removeAt(i);
            }
        }
        else if (shutter.getShape() == DisplayShutter::UndefinedShape)
        {
            shutterList.removeAt(i);
        }
    }

    /// Once the sieve is done, we return the intersection of the resulting shutters
    m_displayShutterForDisplay = DisplayShutter::intersection(shutterList);
    m_haveToBuildDisplayShutterForDisplay = false;
    
    return m_displayShutterForDisplay;
}

vtkImageData* Image::getDisplayShutterForDisplayAsVtkImageData()
{
    if (!hasDisplayShutters())
    {
        return 0;
    }

    if (!m_displayShutterForDisplayVtkImageData)
    {
        DisplayShutter shutter = this->getDisplayShutterForDisplay();
        if (shutter.getShape() != DisplayShutter::UndefinedShape)
        {
            m_displayShutterForDisplayVtkImageData = shutter.getAsVtkImageData(m_columns, m_rows);
            if (m_displayShutterForDisplayVtkImageData)
            {
                m_displayShutterForDisplayVtkImageData->SetOrigin(m_imagePositionPatient);
            }
        }
    }

    return m_displayShutterForDisplayVtkImageData;
}

void Image::setDICOMSource(const DICOMSource &imageDICOMSource)
{
    m_imageDICOMSource = imageDICOMSource;
}

DICOMSource Image::getDICOMSource() const
{
    return m_imageDICOMSource;
}

QString Image::getKeyIdentifier() const
{
    return m_SOPInstanceUID + "#" + QString::number(m_frameNumber);
}

void Image::setParentSeries(Series *series)
{
    m_parentSeries = series;
    this->setParent(m_parentSeries);
}

Series* Image::getParentSeries() const
{
    return m_parentSeries;
}

void Image::setPath(const QString &path)
{
    m_path = path;
}

QString Image::getPath() const
{
    return m_path;
}

QPixmap Image::getThumbnail(bool getFromCache, int resolution)
{
    ThumbnailCreator thumbnailCreator;
//    bool createThumbnail = true;

    if (m_thumbnail.isNull())
    {
        // if (getFromCache)
        // {
        //     //First we try to find the thumbnail with volume number and then without volume number
        //     ///If we can't find any files, we'll have to re-create them

        //     ///We get the base directory where the thumbnail can be found
        //     QString thumbnailPath = QFileInfo(getPath()).absolutePath();
        //     ///Absolute path of the thumbnail file
        //     QString thumbnailFilePath = QString("%1/thumbnail%2.png").arg(thumbnailPath).arg(getVolumeNumberInSeries());

        //     QFileInfo thumbnailFile(thumbnailFilePath);
        //     if (thumbnailFile.exists())
        //     {
        //         m_thumbnail = QPixmap(thumbnailFilePath);
        //         createThumbnail = false;
        //     }
        //     else
        //     {
        //         thumbnailFilePath = QString("%1/thumbnail.png").arg(thumbnailPath);
        //         thumbnailFile.setFile(thumbnailFilePath);
        //         if (thumbnailFile.exists())
        //         {
        //             m_thumbnail = QPixmap(thumbnailFilePath);
        //             createThumbnail = false;
        //         }
        //     }
        // }

        // if (createThumbnail)
        //{
        m_thumbnail = QPixmap::fromImage(thumbnailCreator.getThumbnail(this, resolution));
        //}
    }
    return m_thumbnail;
}

QStringList Image::getSupportedModalities()
{
    // Modalitats extretes de DICOM PS 3.3 C.7.3.1.1.1
    QStringList supportedModalities;
    ///Modalities that we know are image and that in principle we must be able to support
    supportedModalities << "CR" << "CT" << "MR" << "US" << "BI" << "DD" << "ES" << "PT" << "ST" << "XA" << "RTIMAGE" << "DX" << "IO" << "GM" << "XC" << "OP"
                        << "NM" << "OT" << "CD" << "DG" << "LS" << "RG" << "TG" << "RF" << "MG" << "PX" << "SM" << "ECG" << "IVUS";
    ///"Non-standard" modes but that would correspond to images we can support
    supportedModalities << "SC";

    ///These modalities are in principle not image. We keep them documented in case they need to be included in the list
    /// TODO You need to check if they are image modes and delete them as appropriate
    /// "RTSTRUCT" << "RTRECORD" << "EPS" << "RTDOSE" << "RTPLAN" << "HD" << "SMR" << "AU"

    return supportedModalities;
}

bool Image::readOverlays(bool splitOverlays)
{
    ImageOverlayReader reader;
    reader.setFilename(this->getPath());
    if (reader.read())
    {
        if (splitOverlays)
        {
            bool mergeOk;
            ImageOverlay mergedOverlay = ImageOverlay::mergeOverlays(reader.getOverlays(), mergeOk);
            if (!mergeOk)
            {
                ERROR_LOG("Overlays merge failed! Possible cause: lack of memory");
                //DEBUG_LOG("Overlays merge failed! Possible cause: lack of memory");
                return false;
            }

            m_overlaysSplit = mergedOverlay.split();
            return true;
        }
        else
        {
            m_overlaysList = reader.getOverlays();
            return true;
        }
    }
    else
    {
        ERROR_LOG("Failed to read image overlay with path: " + this->getPath());
        //DEBUG_LOG("Failed to read image overlay with path: " + this->getPath());
        return false;
    }
}

void Image::setDICOMKVP(QString KVP)
{
	m_dicomKVP = KVP;
}
QString Image::getDICOMKVP()
{
	return m_dicomKVP;
}

void Image::setXRayTubeCurrent(QString XRayTubeCurrent)
{
	m_dicomXRayTubeCurrent = XRayTubeCurrent;
}

QString Image::getXRayTubeCurrent()
{
	return m_dicomXRayTubeCurrent;
}


}
