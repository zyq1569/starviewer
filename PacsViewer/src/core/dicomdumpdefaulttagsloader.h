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

#ifndef UDGDICOMDUMPDEFAULTTAGSLOADER_H
#define UDGDICOMDUMPDEFAULTTAGSLOADER_H

#include <QFileInfo>
#include <QStringList>

namespace udg {

/**
Class responsible for retrieving XML files that contain information regarding the Default Tags that have been
     to use for DICOMDump.
  */
class DICOMDumpDefaultTagsLoader {

public:
    DICOMDumpDefaultTagsLoader();

    ~DICOMDumpDefaultTagsLoader();

    /// Method that loads defined XML files to a default address. (Hardcode)
    void loadDefaults ();

    /// Method that loads XML files containing Tag information. The parameter can be the path of a Directory or File.
    void loadXMLFiles(const QString &path);

private:
    /// Private methods for reading default tag files
    QStringList loadXMLFiles(const QFileInfo &fileInfo);
    QString loadXMLFile(const QFileInfo &fileInfo);
};

}

#endif
