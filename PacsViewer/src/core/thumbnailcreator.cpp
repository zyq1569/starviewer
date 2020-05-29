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
#include <QString>
#include <QPainter>

#include "series.h"
#include "image.h"
#include "logging.h"
#include "dicomtagreader.h"
// Fem servir dcmtk per l'escalat de les imatges dicom
#include <dcmimage.h>
#include <ofbmanip.h>
#include <dcdatset.h>
// Necessari per suportar imatges de color
#include <diregist.h>

namespace udg {

const QString PreviewNotAvailableText(QObject::tr("Preview image not available"));

QImage ThumbnailCreator::getThumbnail(const Series *series, int resolution)
{
    QImage thumbnail;

    if (series->getModality() == "KO")
    {
        thumbnail.load(":/images/kinThumbnail.png");
    }
    else if (series->getModality() == "PR")
    {
        thumbnail.load(":/images/presentationStateThumbnail.png");
    }
    else if (series->getModality() == "SR")
    {
        thumbnail.load(":/images/structuredReportThumbnail.png");
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
            // Si la sèrie no conté imatges en el thumbnail ho indicarem
            thumbnail = makeEmptyThumbnailWithCustomText(QObject::tr("No Images Available"));
        }
    }

    return thumbnail;
}

QImage ThumbnailCreator::getThumbnail(const Image *image, int resolution)
{
    return createImageThumbnail(image->getPath(), resolution);
}

QImage ThumbnailCreator::getThumbnail(DICOMTagReader *reader, int resolution)
{
    return createThumbnail(reader, resolution);
}

QImage ThumbnailCreator::makeEmptyThumbnailWithCustomText(const QString &text, int resolution)
{
    QImage thumbnail;

    thumbnail = QImage(resolution, resolution, QImage::Format_RGB32);
    thumbnail.fill(Qt::black);

    QPainter painter(&thumbnail);
    painter.setPen(Qt::white);
    painter.drawText(0, 0, resolution, resolution, Qt::AlignCenter | Qt::TextWordWrap, text);

    return thumbnail;
}

QImage ThumbnailCreator::createImageThumbnail(const QString &imageFileName, int resolution)
{
    DICOMTagReader reader(imageFileName);
    return createThumbnail(&reader, resolution);
}

QImage ThumbnailCreator::createThumbnail(DICOMTagReader *reader, int resolution)
{
    QImage thumbnail;

    if (isSuitableForThumbnailCreation(reader))
    {
        try
        {
            // Carreguem el fitxer dicom a escalar
            // Fem que en el cas que sigui una imatge multiframe, només carregui la primera imatge i prou, estalviant allotjar memòria innecessàriament
            DicomImage *dicomImage = new DicomImage(reader->getDcmDataset(), reader->getDcmDataset()->getOriginalXfer(), CIF_UsePartialAccessToPixelData, 0, 1);
            thumbnail = createThumbnail(dicomImage, resolution);

            // Cal esborrar la DicomImage per no tenir fugues de memòria
            if (dicomImage)
            {
                delete dicomImage;
            }
        }
        catch (std::bad_alloc &e)
        {
            ERROR_LOG(QString("No s'ha pogut generar el thumbnail per falta de memòria: %1").arg(e.what()));
            thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText);
        }
    }
    else
    {
        // Creem thumbnail alternatiu indicant que no es pot mostrar una imatge de preview
        thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText);
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
        DEBUG_LOG("Memòria insuficient per carregar l'imatge DICOM al fer el thumbnail o punter nul");
    }
    else if (dicomImage->getStatus() == EIS_Normal)
    {
        dicomImage->hideAllOverlays();
        dicomImage->setMinMaxWindow(1);
        // Escalem la imatge
        DicomImage *scaledImage;
        // Escalem pel cantó més gran
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
            DEBUG_LOG("La imatge escalada s'ha retornat com a nul");
        }
        else if (scaledImage->getStatus() == EIS_Normal)
        {
            thumbnail = convertToQImage(scaledImage);
            if (thumbnail.isNull())
            {
                DEBUG_LOG("No s'ha pogut convertir la DicomImage a QImage. Es crea un thumbnail de Preview not available.");
                ok = false;
            }
            else
            {
                ok = true;
            }

            // Cal esborrar la DicomImage per no tenir fugues de memòria
            delete scaledImage;
        }
        else
        {
            ok = false;
            DEBUG_LOG(QString("La imatge escalada té errors. Error: %1 ").arg(DicomImage::getString(scaledImage->getStatus())));
        }
    }
    else
    {
        ok = false;
        DEBUG_LOG(QString("Error en carregar la DicomImage. Error: %1 ").arg(DicomImage::getString(dicomImage->getStatus())));
    }

    // Si no hem pogut generar el thumbnail, creem un de buit
    if (!ok)
    {
        thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText);
    }

    return thumbnail;
}

bool ThumbnailCreator::isSuitableForThumbnailCreation(DICOMTagReader *reader) const
{
    if (!reader)
    {
        DEBUG_LOG("El DICOMTagReader donat és NUL!");
        return false;
    }

    if (!reader->getDcmDataset())
    {
        DEBUG_LOG("El DICOMTagReader no té cap DcmDataset assignat, no podem generar el thumbnail.");
        return false;
    }

    return true;
}

QImage ThumbnailCreator::convertToQImage(DicomImage *dicomImage)
{
    Q_ASSERT(dicomImage);

    // El següent codi crea una imatge PGM o PPM a memòria i carreguem aquest buffer directament a la QImage
    // Basat en el codi de http://forum.dcmtk.org/viewtopic.php?t=120&highlight=qpixmap

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

    // QImage en la que carregarem el buffer de dades
    QImage thumbnail;
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
                DEBUG_LOG("La càrrega del buffer al thumbnail ha fallat :(");
            }
        }
        // Delete temporary pixel buffer
        delete[] buffer;
    }
    else
    {
        DEBUG_LOG("Memòria insuficient per crear el buffer del thumbnail!");
    }

    return thumbnail;
}

};
