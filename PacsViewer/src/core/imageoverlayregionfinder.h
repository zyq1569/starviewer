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

#ifndef UDGIMAGEOVERLAYREGIONFINDER_H
#define UDGIMAGEOVERLAYREGIONFINDER_H

#include <QList>

class QBitArray;
class QRect;

namespace udg {

class ImageOverlay;

/**
This class allows you to find regions in an ImageOverlay
trying to minimize the total empty area but without making many regions very small.
*/
class ImageOverlayRegionFinder {

public:

    ImageOverlayRegionFinder(const ImageOverlay &overlay);

    /// Find the overlay regions that contain objects and save them in the region list.
    /// If optimizeForPowersOf2 is true, it joins the regions that together occupy less
    /// memory of textures that separately, given that textures have
    /// sizes that are powers of 2.
    void findRegions(bool optimizeForPowersOf2);
    /// Returns the list of overlay regions.
    const QList<QRect>& regions() const;

protected:

    /// Returns the distance between the given regions, calculated as
    /// a distance from Chebyshev (http://en.wikipedia.org/wiki/Chebyshev_distance).
    static int distanceBetweenRegions(const QRect &region1, const QRect &region2);

private:

    ///Returns the index to access the data from the row and column.
    int getDataIndex(int row, int column) const;
    /// Returns the index of the row from the index of the data.
    int getRowIndex(int i) const;
    ///Returns the index of the column from the index of the data.
    int getColumnIndex(int i) const;

    ///Makes a region grow from the indicated pixel. Fill in the mask and return the found region.
    QRect growRegion(int row, int column, QBitArray &mask);
    ///Set to 1 all mask pixels that belong to the region.
    void fillMaskForRegion(QBitArray &mask, const QRect &region);
    /// cSet to 1 all mask pixels that belong to the region..
    void addPadding(QRect &region);
    ///Removes one-pixel padding around the region.
    void removePadding(QRect &region);
    /// Add the region to the list, merging it with others if they are very close.
    /// If optimizeForPowersOf2 is true, it also merges them if together they make better use of texture memory.
    void addRegion(QRect &region, bool optimizeForPowersOf2);

private:

    /// L'overlay que s'ha de separar.
    const ImageOverlay &m_overlay;
    /// Llista de regions trobades.
    QList<QRect> m_regions;

};

}

#endif // UDGIMAGEOVERLAYREGIONFINDER_H
