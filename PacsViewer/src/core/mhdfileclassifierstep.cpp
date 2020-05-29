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

#include "mhdfileclassifierstep.h"

#include "logging.h"
#include "patientfillerinput.h"
#include "patient.h"
#include "image.h"

#include <QDateTime>
#include <QFileInfo>

// Fer servir la llibreria metaIO de les itk
#include <metaCommand.h>
// Fer servir el reader de vtk
#include <vtkMetaImageReader.h>

namespace udg {

MHDFileClassifierStep::MHDFileClassifierStep()
 : PatientFillerStep()
{
}

MHDFileClassifierStep::~MHDFileClassifierStep()
{
}

bool MHDFileClassifierStep::fillIndividually()
{
    Q_ASSERT(m_input);

    QString file = m_input->getFile();

    // Primer comprovem si l'arxiu es pot processar
    vtkMetaImageReader *mhdReader = vtkMetaImageReader::New();
    switch (mhdReader->CanReadFile(qPrintable(file)))
    {
        // No és un arxiu mhd :(
        case 0:
            DEBUG_LOG(file + " no es pot llegir com arxiu mhd vàlid amb vtkMetaImageReader");
            mhdReader->Delete();
            return false;

        // I think I can read the file but I cannot prove it
        case 1:
            DEBUG_LOG("vtkMetaImageReader creu que pot llegir l'arxiu " + file + " però no pot provar-ho de totes totes");
            break;

        // I definitely can read the file
        case 2:
            DEBUG_LOG("vtkMetaImageReader assegura que pot llegir l'arxiu " + file);
            break;

        // I can read the file and I have validated that I am the correct reader for this file
        case 3:
            DEBUG_LOG("vtkMetaImageReader assegura que pot llegir l'arxiu " + file + " i que a més ha validat ser el reader adequat per llegir l'arxiu");
            break;
    }
    mhdReader->Delete();

    // Abans de res comprovar que l'arxiu no estigui ja classificat
    // comprovem primer que l'arxiu no estigui ja dins de l'estructura, el qual vol dir que ja l'han classificat
    bool found = false;
    unsigned int i = 0;
    while (i < m_input->getNumberOfPatients() && !found)
    {
        found = m_input->getPatient(i)->hasFile(file);
        i++;
    }
    if (found)
    {
        return true;
    }

    // TODO faltaria comprovar si tenim algun pacient igual o no? i assignar al mateix estudi o algo semblant? En certa manera si sempre assignem la mateixa
    // informació, posteriorment els pacients que anem creant, ja es fusionaran ells mateixos. Els id's que han de ser diferents haurien de ser els de les series

    Patient *patient;
    Study *study;
    if (m_input->getNumberOfPatients() == 0)
    {
        // Creem el pacient
        patient = new Patient;

        patient->setFullName("MHD File ");
        patient->setID("MHD Patient");

        m_input->addPatient(patient);

        // Creem l'estudi
        study = new Study;
        study->setInstanceUID("MHDStudy-#123456#");
        study->setDate(QDate::currentDate());
        study->setTime(QTime::currentTime());
        study->setID("MHDStudy-#123456#");
        study->setDescription("MHD Study");
        patient->addStudy(study);
    }
    else
    {
        patient = m_input->getPatient();
        study = patient->getStudies().first();
    }

    // Creem la serie
    Series *series = new Series;
    static int seriesUID = 123456;
    static int seriesNumber = 0;
    series->setInstanceUID(QString("MHDSeries-#%1#").arg(seriesUID++));

    // TODO de moment li diem OTHER, però si l'mhd té aquesta info, li donarem de l'mhd
    series->setModality("OT");
    series->setSeriesNumber(QString::number(seriesNumber += 101));
    series->setDate(QDate::currentDate());
    series->setTime(QTime::currentTime());

    QFileInfo fileInfo(file);
    series->setDescription(fileInfo.fileName());
    study->addSeries(series);

    // Aquí fem el pas de l'image filler step: creem la imatge
    Image *image = new Image;
    image->setPath(file);
    static int mhdSOPIntanceUID = 0;
    image->setSOPInstanceUID(QString("MHDImage-#%1#").arg(mhdSOPIntanceUID++));
    // TODO obtenir l'spacing i thickness de l'mhd, aquests valors són temporals i per defecte únicament
    image->setPixelSpacing(1.0, 1.0);
    image->setSliceThickness(1.0);
    image->setSamplesPerPixel(1);

    // TODO obtenir valor restants d'imatge de l'mhd si és possible

    series->addImage(image);

    m_input->addLabelToSeries("MHDFileClassifierStep", series);

    return true;
}

}
