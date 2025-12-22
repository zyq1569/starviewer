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

#ifndef UDGIMAGEOVERLAYREADER_H
#define UDGIMAGEOVERLAYREADER_H

#include <QList>

#include "imageoverlay.h"

namespace gdcm {
class Image;
}

namespace udg {

class ImageOverlay;

/**
Class to read overlays through a file. To read the overlays you must first assign the file name
of what we want to read the overlays and then read them. Once done
reading we can get overlays with getOverlays ()

Exemple:
\code
ImageOverlayReader reader;
reader.setFilename("C:\samples\image.dcm");
if (read())
{
    return reader.getOverlays();
}
\endcode
 */
class ImageOverlayReader {
public:
    ImageOverlayReader();
    ~ImageOverlayReader();

    /// File name from which we should read the overlay
    void setFilename(const QString &filename);

    /// Once given a file name, read the overlays. Returns false in case of error, true otherwise
    bool read();

    /// Returns the list of read overlays
    QList<ImageOverlay> getOverlays() const;

private:
    /// Returns us a gdcm :: Image from the specified filename. It will return null in case of error
    virtual gdcm::Image getGDCMImageFromFile(const QString &filename);

private:
    ///File name from which we should read the overlays
    QString m_filename;

    /// List of overlays read
    QList<ImageOverlay> m_overlaysList;
};

}

#endif
