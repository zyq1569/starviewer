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

#include "appimportfile.h"

#include "interfacesettings.h"
#include "starviewerapplication.h"
// Qt
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
// itk
#include <itkObject.h> // Required to disable release warnings
// Recursos
#include "logging.h"

namespace udg {

AppImportFile::AppImportFile(QObject *parent)
    : QObject(parent)
{
    // TODO: Warnings in release are currently disabled so that the vtkOutputWindow window does not appear
    // but the good solution is to avoid warnings or redirect them to a file.
#ifdef QT_NO_DEBUG
    itk::Object::GlobalWarningDisplayOff();
#endif

    readSettings();
}

AppImportFile::~AppImportFile()
{
}

void AppImportFile::open()
{
    const QString MetaIOImageFilter("MetaIO Image (*.mhd)"), DICOMImageFilter("DICOM Images (*.dcm)"), AllFilesFilter("All Files (*)");
    QStringList imagesFilter;
    imagesFilter << MetaIOImageFilter << DICOMImageFilter << AllFilesFilter;

    QFileDialog *openDialog = new QFileDialog(0);
    openDialog->setWindowTitle(tr("Select files to open..."));
    openDialog->setDirectory(m_workingDirectory);
    openDialog->setNameFilters(imagesFilter);
    openDialog->selectNameFilter (m_lastExtension);
    openDialog->setFileMode(QFileDialog::ExistingFiles);
    openDialog->setAcceptMode(QFileDialog::AcceptOpen);

    if (openDialog->exec() == QDialog::Accepted)
    {
        QStringList fileNames = openDialog->selectedFiles();

        emit selectedFiles(fileNames);

        m_workingDirectory = QFileInfo(fileNames.first()).dir().path();
        m_lastExtension = openDialog->selectedNameFilter();

        writeSettings();
    }
    delete openDialog;
}

typedef QString  OFString;

bool ReadStudyInfo(OFString filename,OFString dir, QStringList &data);


bool ReadStudyInfo(OFString filename,OFString dir, QStringList &data)
{
    OFString value;
    QFile aFile(filename);
    if (!aFile.exists()) //no file
        return false;
    if (!aFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream aStream(&aFile);
    value = aStream.readLine();
    aStream.atEnd();
    //value = OFString(buffer);
    do
    {
        if (value == "[SERIES]")
        {
            value = aStream.readLine();
            OFString seruid = value.section('|',2,2);//substr(pos + 1, value.length());
            aStream.readLine();
            value = aStream.readLine();
            if (aStream.atEnd())
            {
                //pos = value.find('|');
                OFString imageuid = value.section('|',1,1);//substr(pos + 1, value.length());
                value = dir + "/";
                value += seruid;
                value += "/";
                value += imageuid + ".dcm";
                data.push_back(value);
                break;
            }
            while (value != "[SERIES]")
            {
                OFString imageuid = value.section('|',1,1);//substr(pos + 1, value.length());
                value = dir + "/";
                value += seruid;
                value += "/";
                value += imageuid+".dcm";
                data.push_back(value);

                value = aStream.readLine();
                if (aStream.atEnd())
                {
                    imageuid = value.section('|',1,1);//substr(pos + 1, value.length());
                    value = dir + "/";
                    value += seruid;
                    value += "/";
                    value += imageuid + ".dcm";
                    data.push_back(value);
                    break;
                }
            }
        }
        else
        {
            value = aStream.readLine();
            ///out.getline(buffer, max, '\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
            /// value = OFString(buffer);
        }
    } while (!aStream.atEnd());
    //out.close();
    aFile.close();
    return true;
}
void AppImportFile::openDirectory(bool recursively)
{
    QString directoryName = QFileDialog::getExistingDirectory(0,tr("Choose a directory to scan"), m_workingDicomDirectory, QFileDialog::ShowDirsOnly);
    if (!directoryName.isEmpty())
    {
        INFO_LOG("The directory is scanned: " + directoryName + " to open the studies it contains");
        m_workingDicomDirectory = directoryName;
        writeSettings();
        //------------------------2020923-----------------------------------------------
        //int pos = directoryName.lastIndexOf("/");
        //QString studyini = directoryName.right(directoryName.size()-pos-1)+".ini";
        //QStringList data;
        //if (ReadStudyInfo(directoryName + "/"+studyini,directoryName,data))
        //{
        //    if  (!data.isEmpty())
        //    {
        //        emit selectedFiles(data);
        //        return;
        //    }
        //}
        //------------------------------------------------------------------------
        //INFO_LOG("scanDirectories(...)" + directoryName + "start!");
        //List where we will save all the supported files in the directories
        QStringList filenames;
        if (recursively)
        {
            // We recursively explore all directories
            QStringList dirList;
            scanDirectories(directoryName, dirList);
            //For each directory, we get the files we can handle
            foreach (const QString &dirName, dirList)
            {
                filenames << generateFilenames(dirName);
            }
        }
        else
        {
            //We will only consider files that are in the root directory without being recursively scanned
            filenames << generateFilenames(directoryName);
        }

        if (!filenames.isEmpty())
        {
            emit selectedFiles(filenames);
        }
        else
        {
            QMessageBox::warning(0, ApplicationNameString, tr("No supported input files found"));
        }
    }
}

QStringList AppImportFile::generateFilenames(const QString &dirPath)
{
    QStringList list;
    //We check that the directory has files
    QDir dir(dirPath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);

    QString suffix;
    //We add to the list each of the absolute paths of the files contained in the directory
    foreach (const QFileInfo &fileInfo, fileInfoList)
    {
        suffix = fileInfo.suffix();
        if(( suffix.length() > 0 && suffix.toLower() == "dcm" ) || suffix.length() == 0)
        {
            list << fileInfo.absoluteFilePath();
        }
    }

    return list;
}

void AppImportFile::scanDirectories(const QString &rootPath, QStringList &dirsList)
{
    QDir rootDir(rootPath);
    if (rootDir.exists())
    {
        // We add the current directory to the list
        dirsList << rootPath;
        // We look to see if we have more directories
        QStringList subdirs = rootDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        if (!subdirs.isEmpty())
        {
            // For each subdirectory we scan recursively
            foreach (const QString &subDir, subdirs)
            {
                scanDirectories(rootPath + "/" + subDir, dirsList);
            }
        }
    }
}

void AppImportFile::readSettings()
{
    Settings settings;

    m_workingDirectory = settings.getValue(InterfaceSettings::OpenFileLastPath).toString();
    INFO_LOG("workingDirectory:"+m_workingDirectory);
    m_workingDicomDirectory = settings.getValue(InterfaceSettings::OpenDirectoryLastPath).toString();
    INFO_LOG("workingDicomDirectory:"+m_workingDicomDirectory);
    m_lastExtension = settings.getValue(InterfaceSettings::OpenFileLastFileExtension).toString();
    INFO_LOG("lastExtension:"+m_lastExtension);
}

void AppImportFile::writeSettings()
{
    Settings settings;

    settings.setValue(InterfaceSettings::OpenFileLastPath, m_workingDirectory);
    settings.setValue(InterfaceSettings::OpenDirectoryLastPath, m_workingDicomDirectory);
    settings.setValue(InterfaceSettings::OpenFileLastFileExtension, m_lastExtension);
}

};  // end namespace udg
