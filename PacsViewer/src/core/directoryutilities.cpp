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

#include "directoryutilities.h"

#include "logging.h"

#include <QDir>
#include <QCoreApplication>

namespace udg {

DirectoryUtilities::DirectoryUtilities()
{
}

DirectoryUtilities::~DirectoryUtilities()
{
}

bool DirectoryUtilities::deleteDirectory(const QString &directoryPath, bool deleteRootDirectory)
{
    bool result;
    result = removeDirectory(QDir(directoryPath), deleteRootDirectory);

    if (!result)
    {
        if (deleteRootDirectory)
        {
            ERROR_LOG("There were errors deleting the directory and its contents" + directoryPath);
        }
        else
        {
            ERROR_LOG("There were errors deleting the contents of the directory" + directoryPath);
        }
    }
    return result;
}

bool DirectoryUtilities::copyDirectory(const QString &sourceDirectoryPath, const QString &destinationDirectoryPath)
{
    QDir sourceDirectory(sourceDirectoryPath), destinationDirectory(destinationDirectoryPath);
    QStringList files, directories;

    if (!sourceDirectory.exists())
    {
        ERROR_LOG(QString("Source directory %1 does not exist").arg(sourceDirectoryPath));
        return false;
    }

    if (!destinationDirectory.exists())
    {
        if (!destinationDirectory.mkdir(destinationDirectoryPath))
        {
            ERROR_LOG("Could not create directory" + destinationDirectoryPath);
            return false;
        }
    }

    // We copy the files from the directory
    files = sourceDirectory.entryList(QDir::Files);
    for (int i = 0; i < files.count(); i++)
    {
        QString sourceFile = sourceDirectoryPath + QDir::separator() + files[i];
        QString destinationFile = destinationDirectoryPath + QDir::separator() + files[i];

        if (!QFile::copy(sourceFile, destinationFile))
        {
            ERROR_LOG(QString("Could not copy file %1 to destination %2").arg(sourceFile, destinationFile));
            return false;
        }
    }

    // We copy the subdirectories
    directories = sourceDirectory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < directories.count(); i++)
    {
        QString sourceSubDirectory = sourceDirectoryPath + QDir::separator() + directories[i];
        QString destinationSubDirectory = destinationDirectoryPath + QDir::separator() + directories[i];
        if (!copyDirectory(sourceSubDirectory, destinationSubDirectory))
        {
            return false;
        }
    }

    return true;
}

bool DirectoryUtilities::isDirectoryEmpty(const QString &path)
{
    QDir dir(path);

    return dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files).count() == 0;
}

bool DirectoryUtilities::removeDirectory(const QDir &dir, bool deleteRootDirectory)
{
    bool ok = true;
    // QDir::NoDotAndDotDot
    if (dir.exists())
    {
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int i = 0; i < count && ok; i++)
        {
            QFileInfo entryInfo = entries[i];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                ok = removeDirectory(QDir(path), true);
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                {
                    ok = false;
                    ERROR_LOG("The file could not be deleted" + path);
                }
                else
                {
                    emit directoryDeleted();
                    QCoreApplication::processEvents();
                }
            }
        }

        if (deleteRootDirectory)
        {
            if (!dir.rmdir(dir.absolutePath()))
            {
                ok = false;
                ERROR_LOG("The directory could not be deleted " + dir.absolutePath());
            }
        }
    }

    return ok;
}

}
