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

#include "volumepixeldatareaderitkdcmtk.h"

#include "logging.h"
#include "dicomtagreader.h"
#include "volumepixeldata.h"
// ITK
#include <itkTileImageFilter.h>
// Qt
#include <QFileInfo>
#include <QDir>

namespace udg {

VolumePixelDataReaderITKDCMTK::VolumePixelDataReaderITKDCMTK(QObject *parent)
: VolumePixelDataReader(parent)
{
    m_seriesReader = SeriesReaderType::New();

    m_dcmtkIO = ImageIOType::New();
    m_dcmtkIO->SetReadFrameByFrame(true);

    m_progressSignalAdaptor = new itk::QtSignalAdaptor();
    // Connect the adaptor as an observer of a Filter's event
    m_seriesReader->AddObserver(itk::ProgressEvent(), m_progressSignalAdaptor->GetCommand());

    // Connect the adaptor's Signal to the Qt Widget Slot
    connect(m_progressSignalAdaptor, SIGNAL(Signal()), SLOT(slotProgress()));

    m_seriesReader->SetImageIO(m_dcmtkIO);
}

VolumePixelDataReaderITKDCMTK::~VolumePixelDataReaderITKDCMTK()
{
}

int VolumePixelDataReaderITKDCMTK::read(const QStringList &filenames)
{
    int errorCode = NoError;

    if (filenames.isEmpty())
    {
        WARN_LOG("La llista de noms de fitxer per carregar és buida");
        DEBUG_LOG("La llista de noms de fitxer per carregar és buida");
        errorCode = InvalidFileName;
    }
    else
    {
        if (filenames.size() > 1)
        {
            errorCode = readMultipleFiles(filenames);
        }
        else
        {
            errorCode = readSingleFile(filenames.first());
        }
    }

    return errorCode;
}

void VolumePixelDataReaderITKDCMTK::requestAbort()
{
    m_seriesReader->AbortGenerateDataOn();
}

int VolumePixelDataReaderITKDCMTK::readMultipleFiles(const QStringList &filenames)
{
    // Convertim la QStringList al format std::vector<std::string> que s'esperen les itk
    std::vector<std::string> stlFilenames;
    for (int i = 0; i < filenames.size(); i++)
    {
        stlFilenames.push_back(filenames.at(i).toStdString());
    }
    m_seriesReader->SetFileNames(stlFilenames);

    int errorCode = NoError;
    try
    {
        m_seriesReader->Update();
    }
    catch (itk::ProcessAborted)
    {
        errorCode = ReadAborted;
    }
    catch (itk::ExceptionObject &e)
    {
        WARN_LOG(QString("Excepció llegint els arxius del directori [%1] Descripció: [%2]").arg(QFileInfo(filenames.at(0)).dir().path())
                    .arg(e.GetDescription()));
        DEBUG_LOG(QString("Excepció llegint els arxius del directori [%1] Descripció: [%2]").arg(QFileInfo(filenames.at(0)).dir().path())
                     .arg(e.GetDescription()));
        // Llegim el missatge d'error per esbrinar de quin error es tracta
        errorCode = identifyErrorMessage(QString(e.GetDescription()));
    }
    catch (std::bad_alloc)
    {
        errorCode = OutOfMemory;
    }
    catch (...)
    {
        WARN_LOG(QString("Excepció desconeguda llegint els arxius del directori [%1]").arg(QFileInfo(filenames.at(0)).dir().path()));
        DEBUG_LOG(QString("Excepció desconeguda els arxius del directori [%1]").arg(QFileInfo(filenames.at(0)).dir().path()));
        errorCode = UnknownError;
    }

    switch (errorCode)
    {
        case NoError:
            setData(m_seriesReader->GetOutput());
            break;

        case SizeMismatch:
            errorCode = NoError;
            readDifferentSizeImagesIntoOneVolume(filenames);
            break;

        case ZeroSpacingNotAllowed:
            errorCode = NoError;
            // Assignem les dades llegides, aquesta excepció simplement és una mena de warning.
            // En el cas del z-spacing 0 es pot deure a que la informació estigui "amagada" en una seqüència privada
            // o que realment la imatge en sÃí només té sentit com a 2D i no 3D
            setData(m_seriesReader->GetOutput());
            checkZeroSpacingException();
            break;
    }

    return errorCode;
}

int VolumePixelDataReaderITKDCMTK::readSingleFile(const QString &fileName)
{
    int errorCode = NoError;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(qPrintable(fileName));
    reader->SetImageIO(m_dcmtkIO);
    emit progress(0);
    try
    {
        reader->Update();
    }
    catch (itk::ProcessAborted)
    {
        errorCode = ReadAborted;
    }
    catch (itk::ExceptionObject &e)
    {
        WARN_LOG(QString("Excepció llegint l'arxiu [%1] Descripció: [%2]").arg(fileName).arg(e.GetDescription()));
        DEBUG_LOG(QString("Excepció llegint l'arxiu [%1] Descripció: [%2]").arg(fileName).arg(e.GetDescription()));
        // Llegim el missatge d'error per esbrinar de quin error es tracta
        errorCode = identifyErrorMessage(QString(e.GetDescription()));
    }
    catch (std::bad_alloc)
    {
        errorCode = OutOfMemory;
    }
    catch (...)
    {
        WARN_LOG(QString("Excepció desconeguda llegint l'arxiu [%1]").arg(fileName));
        DEBUG_LOG(QString("Excepció desconeguda llegint l'arxiu [%1]").arg(fileName));
        errorCode = UnknownError;
    }

    switch (errorCode)
    {
        case NoError:
            setData(reader->GetOutput());
            break;

        case ZeroSpacingNotAllowed:
            errorCode = NoError;
            // Assignem les dades llegides, aquesta excepció simplement és una mena de warning.
            // En el cas del z-spacing 0 es pot deure a que la informació estigui "amagada" en una seqüència privada
            // o que realment la imatge en sí­ només té sentit com a 2D i no 3D
            setData(reader->GetOutput());
            checkZeroSpacingException();
            break;
    }

    emit progress(100);

    return errorCode;
}

void VolumePixelDataReaderITKDCMTK::readDifferentSizeImagesIntoOneVolume(const QStringList &filenames)
{
    int errorCode = NoError;
    // Declarem el filtre de tiling
    typedef itk::TileImageFilter<Volume::ItkImageType, Volume::ItkImageType> TileFilterType;
    TileFilterType::Pointer tileFilter = TileFilterType::New();
    // Inicialitzem les seves variables
    // El layout ens serveix per indicar cap on creix la cua. En aquest cas volem fer creixer la coordenada Z
    TileFilterType::LayoutArrayType layout;
    layout[0] = 1;
    layout[1] = 1;
    layout[2] = 0;
    tileFilter->SetLayout(layout);

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetImageIO(m_dcmtkIO);

    int progressCount = 0;
    int progressIncrement = static_cast<int>((1.0 / (double)filenames.count()) * 100);

    foreach (const QString &file, filenames)
    {
        emit progress(progressCount);
        // Declarem la imatge que volem carregar
        Volume::ItkImageTypePointer itkImage;
        reader->SetFileName(qPrintable(file));
        try
        {
            reader->UpdateLargestPossibleRegion();
        }
        catch (itk::ExceptionObject &e)
        {
            WARN_LOG(QString("Excepció llegint els arxius del directori [%1] Descripció: [%2]").arg(QFileInfo(filenames.at(0)).dir().path())
                        .arg(e.GetDescription()));
            DEBUG_LOG(QString("Excepció llegint els arxius del directori [%1] Descripció: [%2]").arg(QFileInfo(filenames.at(0)).dir().path())
                         .arg(e.GetDescription()));

            // Llegim el missatge d'error per esbrinar de quin error es tracta
            errorCode = identifyErrorMessage(QString(e.GetDescription()));
        }
        catch (std::bad_alloc)
        {
            errorCode = OutOfMemory;
        }
        catch (...)
        {
            WARN_LOG(QString("Excepció desconeguda llegint els arxius del directori [%1]").arg(QFileInfo(filenames.at(0)).dir().path()));
            DEBUG_LOG(QString("Excepció desconeguda els arxius del directori [%1]").arg(QFileInfo(filenames.at(0)).dir().path()));
            errorCode = UnknownError;
        }
        
        if (errorCode == NoError)
        {
            itkImage = reader->GetOutput();
            reader->GetOutput()->DisconnectPipeline();
        }
        // TODO No es fa tractament d'errors!

        // Un cop llegit el block, fem el tiling
        tileFilter->PushBackInput(itkImage);
        progressCount += progressIncrement;
    }
    tileFilter->Update();
    setData(tileFilter->GetOutput());
    emit progress(100);
}

int VolumePixelDataReaderITKDCMTK::identifyErrorMessage(const QString &errorMessage)
{
    if (errorMessage.contains("Size mismatch") || errorMessage.contains("ImageIO returns IO region that does not fully contain the requested regionRequested"))
    {
        return SizeMismatch;
    }
    else if (errorMessage.contains("Failed to allocate memory for image") || errorMessage.contains("Impossible to allocate"))
    {
        return OutOfMemory;
    }
    else if (errorMessage.contains("The file doesn't exists"))
    {
        return MissingFile;
    }
    else if (errorMessage.contains("A spacing of 0 is not allowed"))
    {
        return ZeroSpacingNotAllowed;
    }
    else if (errorMessage.contains("Cannot read requested file"))
    {
        return CannotReadFile;
    }
    else
    {
        return UnknownError;
    }
}

void VolumePixelDataReaderITKDCMTK::setData(Volume::ItkImageTypePointer itkImage)
{
    // Desconnectem el pipeline del reader de manera que si es canvia el reader, no afecti a aquestes dades.
    itkImage->DisconnectPipeline();

    m_volumePixelData = new VolumePixelData();
    m_volumePixelData->setData(itkImage);
}

void VolumePixelDataReaderITKDCMTK::checkZeroSpacingException()
{
    if (m_volumePixelData)
    {
        double spacing[3];

        m_volumePixelData->getSpacing(spacing);
        DEBUG_LOG(QString("checkZeroSpacing: (x, y, z) = (%1, %2, %3)").arg(spacing[0]).arg(spacing[1]).arg(spacing[2]));

        if (spacing[0] == 0.0 || spacing[1] == 0.0)
        {
            WARN_LOG(QString("x o y spacing és 0; [x,y] = [%1,%2]. Donem el volum per vàlid igualment.").arg(spacing[0]).arg(spacing[1]));
            DEBUG_LOG(QString("x o y spacing és 0; [x,y] = [%1,%2]. Donem el volum per vàlid igualment.").arg(spacing[0]).arg(spacing[1]));
        }
        else if (spacing[2] == 0.0)
        {
            WARN_LOG("El z-spacing de les dades llegides és 0. Possiblement la informació corresponent (SliceThikness/SpacingBetweenSlices) estigui dins "
                     "de seqüències privades. Donem el volum per vàlid igualment.");
            DEBUG_LOG("El z-spacing de les dades llegides és 0. Possiblement la informació corresponent (SliceThikness/SpacingBetweenSlices) estigui dins "
                     "de seqüències privades. Donem el volum per vàlid igualment.");
        }
    }
    else
    {
        DEBUG_LOG("No s'han assignat les dades vtk! No podem fer cap comprovació.");
    }
}

void VolumePixelDataReaderITKDCMTK::slotProgress()
{
    emit progress((int)(m_seriesReader->GetProgress() * 100));
}

}// End namespace udg
