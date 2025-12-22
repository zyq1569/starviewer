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

#ifndef UDGIMAGEOVERLAY_H
#define UDGIMAGEOVERLAY_H

#include <QString>
#include <QSharedPointer>

class QRect;

namespace gdcm {
class Overlay;
}

namespace udg {

class DrawerBitmap;

/**
Class that encapsulates the DICOM Overlay object.
For more information, see section C.9 OVERLAYS, PS 3.3.
*/
class ImageOverlay {
public:
    ImageOverlay();
    ~ImageOverlay();

    /// Assigns / obtains rows and columns of the overlay
    void setRows(int rows);
    void setColumns(int columns);
    int getRows() const;
    int getColumns() const;

    /// Assigns / obtains the origin of the Overlay
    void setOrigin(int x, int y);
    int getXOrigin() const;
    int getYOrigin() const;

    ///Assigns / returns overlay data
    void setData(unsigned char *data);
    unsigned char* getData() const;

    /// Returns true if the overlay is valid (if the number
    /// of rows and the number of columns are positive and has data).
    bool isValid() const;

    /// Creates a smaller overlay within the given region.
    /// If the overlay is invalid or the region is not complete
    /// contained within the overlay, returns an invalid overlay.
    ImageOverlay createSubOverlay(const QRect &region) const;

    /// Separate overlay into smaller overlays that have less wasted space to save memory.
    /// If the overlay is invalid or empty it returns an empty list.
    QList<ImageOverlay> split() const;

    /// Compara aquest overlay amb un altre i diu si són iguals.
    bool operator ==(const ImageOverlay &overlay) const;

    ///Build an ImageOverlay from a gdcm :: Overlay
    static ImageOverlay fromGDCMOverlay(const gdcm::Overlay &gdcmOverlay);

    /// Merge a list of overlays into a single overlay
    /// It will only merge those overlays that meet the necessary conditions to be considered valid, ie
    /// that has a number of rows and columns> 0 and that has data. The parameter ok, will serve to indicate the cases
    /// in which fusion could not be performed due to some error. If the list of valid Overlays is empty, it will return one
    /// empty overlay, if there is only one, will return that same overlay, merging the n overlays otherwise.
    /// For these cases
    /// the value of ok will be true. In case there is not enough memory to host
    /// a new buffer for the merged overlay will return an empty overlay and ok false.
    static ImageOverlay mergeOverlays(const QList<ImageOverlay> &overlaysList, bool &ok);

    /// Returns the overlay in DrawerBitmap format to overlay on an image with the given source and spacing
    DrawerBitmap* getAsDrawerBitmap(double origin[3], double spacing[3]) const;

private:
    /// Internal use for QSharedPointer. This will be the method that will be called to delete
    /// EVERYTHING It would be convenient to have defined a generic deleter defined in another class with a template
    /// template <typename T> void arrayDeleter (T array [])
    /// in case we have more shared pointers with pointers in arrays
    static void deleteDataArray(unsigned char dataArray[]);

    ///Returns a copy of the overlay data to the given region.
    unsigned char* copyDataForSubOverlay(const QRect &region) const;

private:
    ///Overlay rows and columns
    int m_rows, m_columns;

    /// Location of the first point of the overlay with respect to the pixels in the image, given as row \ column.
    /// The upper left pixel has the 1 \ 1 coordinate
    /// Column values greater than 1 indicate that the origin of the overlay plane is to the right of the image source.
    /// Row values greater than 1 indicate that the origin of the overlay plane is below the source of the image.
    /// Values below 1 indicate that the origin of the overlay plane is above or to the left of the image source.
    int m_origin[2];

    /// DA's S face overlay
    QSharedPointer<unsigned char> m_data;
};

}

#endif
