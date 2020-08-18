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
// itk
#include <itkObject.h> //Necessari per desactivar els warnings en release
// Recursos
#include "logging.h"

namespace udg {

AppImportFile::AppImportFile(QObject *parent)
 : QObject(parent)
{
    // TODO: De moment es desactiven els warnings en release perquè no apareixi la finestra vtkOutputWindow
    //      però la solució bona passa per evitar els warnings o bé redirigir-los a un fitxer.
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

void AppImportFile::openDirectory(bool recursively)
{
    QString directoryName = QFileDialog::getExistingDirectory(0, tr("Choose a directory to scan"), m_workingDicomDirectory, QFileDialog::ShowDirsOnly);
    if (!directoryName.isEmpty())
    {
        INFO_LOG("S'escaneja el directori: " + directoryName + " per obrir els estudis que hi contingui");
        m_workingDicomDirectory = directoryName;
        writeSettings();

        // Llista on guardarem tots els arxius compatibles que hi ha als directoris
        QStringList filenames;
        if (recursively)
        {
            // Explorem recursivament tots els directoris
            QStringList dirList;
            scanDirectories(directoryName, dirList);
            // Per cada directori, obtenim els arxius que podem tractar
            foreach (const QString &dirName, dirList)
            {
                filenames << generateFilenames(dirName);
            }
        }
        else
        {
            // Tindrem en compte únicament els arxius que hi hagi en el directori arrel sense explorar recursivament
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
    // Comprovem que el directori tingui arxius
    QDir dir(dirPath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);

    // Afegim a la llista cadascun dels paths absoluts dels arxius que contingui el directori
    foreach (const QFileInfo &fileInfo, fileInfoList)
    {
        list << fileInfo.absoluteFilePath();
    }

    return list;
}

void AppImportFile::scanDirectories(const QString &rootPath, QStringList &dirsList)
{
    QDir rootDir(rootPath);
    if (rootDir.exists())
    {
        // Afegim el directori actual a la llista
        dirsList << rootPath;
        // Busquem si tenim més directoris
        QStringList subdirs = rootDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        if (!subdirs.isEmpty())
        {
            // Per cada subdirectori escanejem recursivament
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
    m_workingDicomDirectory = settings.getValue(InterfaceSettings::OpenDirectoryLastPath).toString();
    m_lastExtension = settings.getValue(InterfaceSettings::OpenFileLastFileExtension).toString();
}

void AppImportFile::writeSettings()
{
    Settings settings;

    settings.setValue(InterfaceSettings::OpenFileLastPath, m_workingDirectory);
    settings.setValue(InterfaceSettings::OpenDirectoryLastPath, m_workingDicomDirectory);
    settings.setValue(InterfaceSettings::OpenFileLastFileExtension, m_lastExtension);
}

};  // end namespace udg
