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

#ifndef UDGDIRECTORYUTILITIES_H
#define UDGDIRECTORYUTILITIES_H

#include <QObject>

class QString;
class QDir;

namespace udg {

/**
Class that encapsulates utility operations with directories
*/
class DirectoryUtilities : public QObject {
    Q_OBJECT
public:
    DirectoryUtilities();
    ~DirectoryUtilities();

    /// Clears the contents of the directory and the directory passed by parameters
    /// @param directoryPath Path of the directory to delete
    /// @param deleteRootDirectory Indicates whether to delete only the contents of the directory or also the root directory passed by parameter. If false only
    /// deletes the content, if true deletes the content and the directory passed by parameter
    /// @return Indicates whether the operation was successful
    bool deleteDirectory(const QString &directoryPath, bool deleteRootDirectory);

    /// Copy the source directory to the destination directory
    static bool copyDirectory(const QString &sourceDirectory, const QString &sourceDestination);
    
    ///Tells us if a directory is empty
    bool isDirectoryEmpty(const QString &directoryPath);

signals:
    void directoryDeleted();

private:
    bool removeDirectory(const QDir &dir, bool deleteRootDirectory);
};

}

#endif
