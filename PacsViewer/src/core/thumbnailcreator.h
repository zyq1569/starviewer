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

#ifndef UDGTHUMBNAILCREATOR_H
#define UDGTHUMBNAILCREATOR_H

class QImage;
class QPixmap;
class QString;
class DicomImage;
//class DcmDataset;
namespace udg {

class Series;
class Image;
class DICOMTagReader;
class ThumbnailCreator {
public:
    /// Create a thumbnail from the images in the series
    QImage getThumbnail(const Series *series, int resolution = 96);

    /// Creates the thumbnail of the image passed by parameter
    QImage getThumbnail(const Image *image, int resolution = 96);

    /// Get the thumbnail from the DICOMTagReader
    QImage getThumbnail(const DICOMTagReader *reader, int resolution = 96);

    /// Create a custom blank thumbnail with the text we give it
    static QImage makeEmptyThumbnailWithCustomText(const QString &text, int resolution = 96);

//    void jpeg2kregisterCodecs();
//    void jpeg2kregisterCleanup();
//    DcmDataset decompressImage( const DcmDataset *olddataset);

private:
    /// Create the thumbnail of an object that is said to be an image
    QImage createImageThumbnail(const QString &imageFileName, int resolution);

    /// Creates a thumbnail from an icon file to the specified resolution
    QImage createIconThumbnail(const QString &iconFileName, int resolution);

    /// Create the thumbnail from a DICOMTagReader
    QImage createThumbnail(const DICOMTagReader *reader, int resolution);

    /// Create the thumbnail from a DicomImage
    QImage createThumbnail(DicomImage *dicomImage, int resolution);

    /// Check that the dataset meets the requirements to be able to make a thumbnail
    /// Returns true if it is a valid dataset, false otherwise
    bool isSuitableForThumbnailCreation(const DICOMTagReader *reader) const;

    /// Convert DicomImage to a QPixmap
    QPixmap convertToQPixmap(DicomImage *dicomImage);
};

}

#endif
