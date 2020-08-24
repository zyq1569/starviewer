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

#ifndef UDGAPPIMPORTFILE_H
#define UDGAPPIMPORTFILE_H

#include <QObject>
#include <QStringList>

namespace udg {

/**
     Mini-application responsible for loading a model from the file system to the volume repository
*/
class AppImportFile : public QObject {
Q_OBJECT
public:
    AppImportFile(QObject *parent = 0);
    ~AppImportFile();

    ///Opens the dialog to be able to open files
    void open();

    /// Opens the dialog to be able to open files from a DICOM directory
    /// By default it explores the contents recursively, searching the subdirectories
    void openDirectory(bool recursively = true);

signals:
    ///Signal that is emitted when one or more files have been chosen that will be processed externally
    void selectedFiles(QStringList);

private:
    /// Returns the list of DICOM files groupable in series in a directory
    /// @param directory Name of the directory where the files are located
    /// @return The sorted filename list
    QStringList generateFilenames(const QString &dirPath);

    /// Given a root path, it scans all subdirectories recursively and returns them to us in a list including the root path
    /// All paths returned are absolute
    void scanDirectories(const QString &rootPath, QStringList &dirsList);

    ///Read write settings
    void readSettings();
    void writeSettings();

private:
    /// Directori de treball per fitxers ordinaris
    QString m_workingDirectory;

    /// Directori de treball per directoris dicom
    QString m_workingDicomDirectory;

    ///Last User Selected Extension
    QString m_lastExtension;
};

};  // end namespace udg

#endif
