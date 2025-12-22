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

#include "screenshottool.h"
#include "qviewer.h"
#include "q2dviewer.h"
#include "logging.h"
#include "volume.h"
#include "coresettings.h"
// Definicions globals d'aplicació
#include "starviewerapplication.h"
// Vtk
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>
// Qt
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
// Pel "wait cursor"
#include <QApplication>

namespace udg {

const QString ScreenShotTool::PngFileFilter = tr("PNG (*.png)");
const QString ScreenShotTool::JpegFileFilter = tr("JPEG (*.jpg)");
const QString ScreenShotTool::BmpFileFilter = tr("BMP (*.bmp)");
const QString ScreenShotTool::TiffFileFilter = tr("TIFF (*.tiff)");
const QString ScreenShotTool::DcmFileFilter = tr("DCM (*.dcm)");
ScreenShotTool::ScreenShotTool(QViewer *viewer, QObject *parent)
    : Tool(viewer, parent)
{
    m_toolName = "ScreenShotTool";
    readSettings();
    m_fileExtensionFilters = PngFileFilter + ";;" + JpegFileFilter + ";;" + BmpFileFilter + ";;" + DcmFileFilter + ";;" + TiffFileFilter;
    if (!viewer)
    {
        DEBUG_LOG("El viewer proporcionat és NUL!");
    }
}

ScreenShotTool::~ScreenShotTool()
{
}

void ScreenShotTool::handleEvent(unsigned long eventID)
{
    switch (eventID)
    {
    case vtkCommand::KeyPressEvent:
    {
        int key = m_viewer->getInteractor()->GetKeyCode();
        // EVERYTHING You need to look at the proper way to handle the Ctrl + key with vtk
        // CTRL + s = key code 19
        // CTRL + a = key code 1
        switch (key)
        {
        // Ctrl+s, "single shot"
        case 19:
            this->screenShot();
            break;

            // Ctrl+a, "multiple shot"
        case 1:
            this->screenShot(false);
            break;
        }
    }
        break;
    }
}

void ScreenShotTool::singleCapture()
{
    screenShot(true);
}

void ScreenShotTool::completeCapture()
{
    screenShot(false);
}

void ScreenShotTool::screenShot(bool singleShot)
{
    readSettings();

    QString caption;
    if (singleShot)
    {
        caption = tr("Save single screenshot as...");
    }
    else
    {
        QMessageBox::information(0, tr("Information"),
                                 tr("You're going to save several screenshots at one time.\nIt's recommended you save them in an empty folder."));
        caption = tr("Save multiple screenshots as...");
    }
    QString filename = QFileDialog::getSaveFileName(0, caption, m_lastScreenShotPath + "/" + compoundSelectedName(), m_fileExtensionFilters,
                                                    &m_lastScreenShotExtensionFilter);

    if (!filename.isEmpty())
    {
        // Let's see that the file name does not contain things like: name.png, that is, the extension is not displayed
        QString selectedExtension = m_lastScreenShotExtensionFilter.mid(m_lastScreenShotExtensionFilter.lastIndexOf("."));
        selectedExtension.remove(")");

        if (filename.endsWith(selectedExtension))
        {
            filename.remove(filename.lastIndexOf(selectedExtension), selectedExtension.size() + 1);
        }

        //We save the last path of the image to know where we have to open the browser by default to save the file
        m_lastScreenShotPath = QFileInfo(filename).absolutePath();
        //Save the name of the last file
        m_lastScreenShotFileName = QFileInfo(filename).fileName();

        // So it may take the process
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (singleShot)
        {
            m_viewer->grabCurrentView();
        }
        else
        {
            Q2DViewer *viewer2D = Q2DViewer::castFromQViewer(m_viewer);
            if (viewer2D)
            {
                // We have a Q2DViewer, then we can save all the images
                // Save the current slice and phase to restore
                int currentSlice = viewer2D->getCurrentSlice();
                int currentPhase = viewer2D->getCurrentPhase();
                int maxSlice = viewer2D->getMaximumSlice() + 1;
                // In case we have phases we will do as many passes as phases
                int phases = viewer2D->getNumberOfPhases();
                for (int i = 0; i < maxSlice; i++)
                {
                    viewer2D->setSlice(i);
                    for (int j = 0; j < phases; j++)
                    {
                        viewer2D->setPhase(j);
                        viewer2D->grabCurrentView();
                    }
                }
                // Restaurem
                viewer2D->setSlice(currentSlice);
                viewer2D->setPhase(currentPhase);
            }
            //We have a viewfinder that is not 2D, so we make a "single shot"
            else
            {
                m_viewer->grabCurrentView();
            }
        }
        // We determine the file extension
        QViewer::FileType fileExtension;
        if (m_lastScreenShotExtensionFilter == PngFileFilter)
        {
            fileExtension = QViewer::PNG;
        }
        else if (m_lastScreenShotExtensionFilter == JpegFileFilter)
        {
            fileExtension = QViewer::JPEG;
        }
        else if (m_lastScreenShotExtensionFilter == BmpFileFilter)
        {
            fileExtension = QViewer::BMP;
        }
        else if (m_lastScreenShotExtensionFilter == TiffFileFilter)
        {
            fileExtension = QViewer::TIFF;
        }
		else if (m_lastScreenShotExtensionFilter == DcmFileFilter)
		{
			fileExtension = QViewer::DICOM;
		}
        else
        {
            DEBUG_LOG("No pattern matches, image cannot be saved! We assign PNG, by default. This mistake should NEVER happen! ");
            fileExtension = QViewer::PNG;
            m_lastScreenShotExtensionFilter = PngFileFilter;
        }
        // We save all the captured images
        m_viewer->saveGrabbedViews(filename, fileExtension);
        QApplication::restoreOverrideCursor();

        writeSettings();
    }
    else
    {
        // Do I have to do anything?
        // If it is "empty" it is because it has either canceled or entered nothing
    }
}

QString ScreenShotTool::compoundSelectedName()
{
    // EVERYTHING This would be better if done with the QRegExp class,
    // producing a much cleaner and clearer code
    QString compoundFile = "";

    if (!m_lastScreenShotFileName.isEmpty())
    {
        QChar lastChar = m_lastScreenShotFileName[m_lastScreenShotFileName.length() - 1];

        if (lastChar.isNumber())
        {
            int i = m_lastScreenShotFileName.length() - 1;

            do
            {
                i--;
                lastChar = m_lastScreenShotFileName[i];
            }
            while (i > 0 && lastChar.isNumber());

            bool ok;
            int sufix = m_lastScreenShotFileName.right(m_lastScreenShotFileName.length() - (i + 1)).toInt(&ok, 10);

            if (ok)
            {
                compoundFile = m_lastScreenShotFileName.mid(0, i + 1) + QString::number(sufix + 1, 10);
            }
            else
            {
                compoundFile = m_lastScreenShotFileName;
            }
        }
        else
        {
            compoundFile = m_lastScreenShotFileName + "1";
        }
    }
    return compoundFile;
}

void ScreenShotTool::readSettings()
{
    Settings settings;

    m_lastScreenShotPath = settings.getValue(CoreSettings::ScreenShotToolFolder).toString();
    m_lastScreenShotExtensionFilter = settings.getValue(CoreSettings::ScreenShotToolFileExtension).toString();
    m_lastScreenShotFileName = settings.getValue(CoreSettings::ScreenShotToolFilename).toString();
}

void ScreenShotTool::writeSettings()
{
    Settings settings;

    settings.setValue(CoreSettings::ScreenShotToolFolder, m_lastScreenShotPath);
    settings.setValue(CoreSettings::ScreenShotToolFileExtension, m_lastScreenShotExtensionFilter);
    settings.setValue(CoreSettings::ScreenShotToolFilename, m_lastScreenShotFileName);
}

}
