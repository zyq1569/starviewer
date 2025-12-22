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

#ifndef HARDDISKINFORMATION_H
#define HARDDISKINFORMATION_H

#include <QString>

namespace udg {

/**
Class that allows us to get information about a hard drive. More specifically it allows us to get the Bytes, MB, GB of a partition and which of these are free.
Keep in mind that when you want to know the free space of a hard drive you must specify from which partition. In windows the
drive (c :, d :, ...) and a directory and in linux you must specify the full path of the directory to know the partition. Currently this class only supports
Unix-based operating systems (Linux, Mac OS X ...).
*/
class HardDiskInformation {
public:
    /// Class builder
    HardDiskInformation();

    ///Class Destroyer
    ~HardDiskInformation();

    /// Method used to know the total number of Bytes (free + occupied)
    /// that is on a particular partition. To indicate the
    /// partition must specify the absolute path to a file or directory that is in it.
    ///
    /// The method does not check that the path exists.
    ///
    /// If there is any error the method will return 0.
    /// @param path Indicates the path to a directory / file within the partition
    /// @return The total number of bytes that a partition has
    quint64 getTotalNumberOfBytes(const QString &path);

    /// Returns the number of bytes of free space we have left on a partition
    /// specific and that can be used by the user. Must do
    /// note that this number may be different from the actual number of free bytes
    /// (for example in cases where there are fees per user). Per
    /// indicate the partition the absolute path must be specified in a file or directory that is in it.
    ///
    /// The method does not check that the path exists.
    ///
    /// If there is any error the method will return 0.
    /// @param path Indicates the path to a directory / file within the partition
    /// @return The number of free bytes that a partition has and
    /// can be used by the user running the program
    quint64 getNumberOfFreeBytes(const QString &path);

    /// It behaves exactly like getTotalNumberOfBytes () but returns
    /// MBytes instead of Bytes. It should be noted, however, that
    /// this is not a method to know the absolute number of MBytes
    /// accuracy (so use getTotalNumberOfBytes ())
    /// @param path Indicates the path to a directory / file within the partition
    /// @return The number of truncated MBytes (eg if it is 1.9MBytes real it will return 1Mbytes)
    quint64 getTotalNumberOfMBytes(const QString &path);

    /// It behaves exactly like getNumberOfFreeBytes () but returns
    /// MBytes instead of Bytes. It should be noted, however, that
    /// this is not a method to know the absolute number of MBytes
    /// accuracy (so use getNumberOfFreeBytes ())
    /// @param path Indicates the path to a directory / file within the partition
    /// @return The number of truncated free MBytes (eg if it is 1.9MBytes real it will return 1MByte)
    quint64 getNumberOfFreeMBytes(const QString &path);

    /// Returns the Bytes that occupy the contents of the given directory
    static qint64 getDirectorySizeInBytes(const QString &directoryPath);

private:
    quint64 getTotalBytesPlataformEspecific(const QString &path);
    quint64 getFreeBytesPlataformEspecific(const QString &path);

    ///Logs the last error according to the platform
    void logLastError(const QString &additionalInformation);

};

};  // End namespace udg

#endif
