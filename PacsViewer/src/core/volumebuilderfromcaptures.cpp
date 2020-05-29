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

#include "volumebuilderfromcaptures.h"

#include <vtkImageAppend.h>
#include <vtkImageData.h>
#include <vtkImageFlip.h>
#include <vtkSmartPointer.h>

#include "volume.h"
#include "series.h"
#include "study.h"
#include "image.h"
#include "logging.h"

#include <dcuid.h>

namespace udg {

VolumeBuilderFromCaptures::VolumeBuilderFromCaptures()
{
    m_vtkImageAppend = vtkImageAppend::New();
    m_vtkImageAppend->SetAppendAxis(2);

    this->setSeriesDescription(QString("Generated from screen captures"));
    m_modality = QString("OT");
}

VolumeBuilderFromCaptures::~VolumeBuilderFromCaptures()
{
    m_vtkImageAppend->Delete();
}

void VolumeBuilderFromCaptures::addCapture(vtkImageData *data)
{
    // \TODO Realitzem alguna comprovació o deixem que el vtkImageAppend es queixi?
    vtkSmartPointer<vtkImageFlip> imageFlip = vtkSmartPointer<vtkImageFlip>::New();
    imageFlip->SetInputData(data);
    // Flip horitzontal
    imageFlip->SetFilteredAxis(1);
    imageFlip->Update();

    m_vtkImageAppend->AddInputData(imageFlip->GetOutput());
}

void VolumeBuilderFromCaptures::setParentStudy(Study *study)
{
    m_parentStudy = study;
}

bool VolumeBuilderFromCaptures::setModality(QString modality)
{
    // Obtenim la llista de modalitats vàlides
    QStringList allowedModalities = Image::getSupportedModalities();

    if (allowedModalities.contains(modality))
    {
        m_modality = modality;
        return true;
    }
    else
    {
        return false;
    }
}

Volume* VolumeBuilderFromCaptures::build()
{
    Q_ASSERT(m_parentStudy);
    Q_ASSERT(m_vtkImageAppend->GetNumberOfInputs());

    // Creem la nova sèrie
    Series *newSeries = new Series();

    // Omplim la informació de la sèrie a partir de la sèrie de referència

    // Assignem la modalitat segons el valor introduit. El valor per defecte és 'OT' (Other).
    newSeries->setModality(m_modality);
    newSeries->setSOPClassUID(QString(UID_SecondaryCaptureImageStorage));

    // Generem el SeriesInstanceUID a partir del generador de DCMTK. \TODO Utilitzar el nostre UID_ROOT?
    char seriesUid[100];
    dcmGenerateUniqueIdentifier(seriesUid, SITE_SERIES_UID_ROOT);
    newSeries->setInstanceUID(QString(seriesUid));
    // \TODO Quin criteri volem seguir per donar nous noms?
    newSeries->setSeriesNumber(QString("0000") + QString::number(m_parentStudy->getSeries().count()));
    newSeries->setDescription(this->getSeriesDescription());

    // Assignem la sèrie a l'estudi al qual partenyia l'inputVolume.
    newSeries->setParentStudy(m_parentStudy);
    m_parentStudy->addSeries(newSeries);

    // Obtenim el nou vtkImageData a partir de la sortida del vtkImageAppend.
    // Fem un flip horitzontal per tal utilitzar el mateix sistema de coordenades que DICOM.
    m_vtkImageAppend->Update();

    vtkSmartPointer<vtkImageData> newVtkData = vtkSmartPointer<vtkImageData>::New();
    newVtkData->ShallowCopy(m_vtkImageAppend->GetOutput());

    // Creem el nou volume
    Volume *newVolume = new Volume();
    newSeries->addVolume(newVolume);

    // Generem les noves imatges a partir del vtkData generat per vtkImageAppend
    int samplesPerPixel = newVtkData->GetNumberOfScalarComponents();
    int bitsAllocated;
    int bitsStored;
    int highBit;
    int pixelRepresentation;
    int rows;
    int columns;

    // \TODO Potser podriem ser mes precisos
    QString photometricInterpretation;
    if (samplesPerPixel == 1)
    {
        photometricInterpretation = QString("MONOCHROME2");
    }
    else if (samplesPerPixel == 3)
    {
        photometricInterpretation = QString("RGB");
    }

    int scalarType = newVtkData->GetScalarType();

    switch (scalarType)
    {
        //case VTK_CHAR:
        //break;
        case VTK_SIGNED_CHAR:
            bitsAllocated = 8;
            pixelRepresentation = 1;
            break;
        case VTK_UNSIGNED_CHAR:
            bitsAllocated = 8;
            pixelRepresentation = 0;
            break;
        case VTK_SHORT:
            bitsAllocated = 16;
            pixelRepresentation = 1;
            break;
        case VTK_UNSIGNED_SHORT:
            bitsAllocated = 16;
            pixelRepresentation = 0;
            break;
        case VTK_INT:
            bitsAllocated = 32;
            pixelRepresentation = 1;
            break;
        case VTK_UNSIGNED_INT:
            bitsAllocated = 32;
            pixelRepresentation = 0;
            break;
//        case VTK_FLOAT:
//            bitsAllocated = 32;
//            pixelRepresentation = 1; ?
//            break;
//        case VTK_DOUBLE:
//            bitsAllocated = 64;
//            pixelRepresentation = 1; ?
//            break;
        default:
            DEBUG_LOG(QString("Pixel Type no suportat: ") + newVtkData->GetScalarTypeAsString());

    }

    bitsStored = bitsAllocated;
    highBit = bitsStored - 1;

    int *dimensions = newVtkData->GetDimensions();
    double *spacing = newVtkData->GetSpacing();

    rows = dimensions[1];
    columns = dimensions[0];

    Image *currentImage;

    for (int i = 0; i < dimensions[2]; i++)
    {
        currentImage = new Image();

        // Generem el SOPInstanceUID a partir del generador de DCMTK. \TODO Utilitzar el nostre UID_ROOT?
        char instanceUid[100];
        dcmGenerateUniqueIdentifier(instanceUid, SITE_INSTANCE_UID_ROOT);
        currentImage->setSOPInstanceUID(QString(instanceUid));
        newSeries->addImage(currentImage);
        newVolume->addImage(currentImage);
        currentImage->setParentSeries(newSeries);
        currentImage->setOrderNumberInVolume(i);
        currentImage->setVolumeNumberInSeries(0);

        currentImage->setBitsAllocated(bitsAllocated);
        currentImage->setBitsStored(bitsStored);
        currentImage->setHighBit(highBit);
        currentImage->setColumns(columns);
        currentImage->setInstanceNumber(QString::number(i + 1));
        currentImage->setPhotometricInterpretation(photometricInterpretation);
        currentImage->setPixelRepresentation(pixelRepresentation);
        currentImage->setPixelSpacing(spacing[0], spacing[1]);
        currentImage->setRows(rows);
        currentImage->setSamplesPerPixel(samplesPerPixel);

    }

    // Es fa després d'haver inserit les imatges perquè el nou Volume activi el flag de dades carregades.
    newVolume->setData(newVtkData);

    newVolume->setNumberOfPhases(1);
    newVolume->setNumberOfSlicesPerPhase(newSeries->getImages().count());

    // Informació de DEBUG
    DEBUG_LOG(QString("\nNova sèrie generada:") +
     QString("\n  SeriesInstanceUID: ") + newSeries->getInstanceUID() +
     QString("\n  SeriesNumber: ") + newSeries->getSeriesNumber() +
     QString("\n  SeriesDescription: ") + newSeries->getDescription() +
     QString("\n  SOPClassUID: ") + newSeries->getSOPClassUID() +
     QString("\n  Modality: ") + newSeries->getModality() +
     QString("\n  SamplesPerPixel: ") + QString::number(samplesPerPixel) +
     QString("\n  PhotometricInterpretation: ") + photometricInterpretation +
     QString("\n  BitsAllocated: ") + QString::number(bitsAllocated) +
     QString("\n  BitsStored: ") + QString::number(bitsStored) +
     QString("\n  HighBit: ") + QString::number(highBit) +
     QString("\n  PixelRepresentation: ") + QString::number(pixelRepresentation) +
     QString("\n  PixelSpacing: ") + QString::number(spacing[0]) + QString(",") + QString::number(spacing[1]) +
     QString("\n  Num.Imatges: ") + QString::number(newSeries->getImages().count()));

    return newVolume;

}

}
