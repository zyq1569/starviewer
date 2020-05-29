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

#include "patientfiller.h"

#include <QTime>
#include <QtAlgorithms>

#include "patientfillerinput.h"
#include "logging.h"
#include "dicomtagreader.h"

// TODO Include's temporals mentre no tenim un registre:
#include "imagefillerstep.h"
#include "dicomfileclassifierfillerstep.h"
#include "temporaldimensionfillerstep.h"
#include "mhdfileclassifierstep.h"
#include "orderimagesfillerstep.h"
// TODO encara no hi ha suport a KINs i Presentation States, per tant
// fins que no tinguem suport i implementem correctament els respectius
// filler steps no caldrà afegir-los dins del pipeline
//#include "keyimagenotefillerstep.h"
//#include "presentationstatefillerstep.h"

namespace udg {

PatientFiller::PatientFiller(DICOMSource dicomSource, QObject *parent)
 : QObject(parent)
{
    registerSteps();
    m_patientFillerInput = new PatientFillerInput();
    m_imageCounter = 0;

    m_patientFillerInput->setDICOMSource(dicomSource);
}

PatientFiller::~PatientFiller()
{
    foreach (PatientFillerStep *fillerStep, m_registeredSteps)
    {
        delete fillerStep;
    }

    delete m_patientFillerInput;
}

// Mètode intern per poder realitzar l'ordenació dels patientfiller
bool patientFillerMorePriorityFirst(const PatientFillerStep *s1, const PatientFillerStep *s2)
{
    return (*s1) < (*s2);
}

void PatientFiller::registerSteps()
{
    m_registeredSteps.append(new ImageFillerStep());
    m_registeredSteps.append(new DICOMFileClassifierFillerStep());
    m_registeredSteps.append(new OrderImagesFillerStep());
    // \TODO Donat que al postProcessing no tenim política d'etiquetes, s'ha posat el Temporal al final
    // perquè necessita que s'hagi executat l'Order abans. S'hauria de millorar.
    m_registeredSteps.append(new TemporalDimensionFillerStep());

    // TODO encara no hi ha suport a KINs i Presentation States, per tant
    // fins que no tinguem suport i implementem correctament els respectius
    // filler steps no caldrà afegir-los dins del pipeline
    //m_registeredSteps.append(new KeyImageNoteFillerStep());
    //m_registeredSteps.append(new PresentationStateFillerStep());
}

void PatientFiller::processDICOMFile(DICOMTagReader *dicomTagReader)
{
    Q_ASSERT(dicomTagReader);

    m_patientFillerInput->setDICOMFile(dicomTagReader);

    QList<PatientFillerStep*> processedFillerSteps;
    QList<PatientFillerStep*> candidatesFillerSteps = m_registeredSteps;
    bool continueIterating = true;

    while (!candidatesFillerSteps.isEmpty() && continueIterating)
    {
        QList<PatientFillerStep*> fillerStepsToProcess;
        QList<PatientFillerStep*> newCandidatesFillerSteps;
        continueIterating = false;

        for (int i = 0; i < candidatesFillerSteps.size(); ++i)
        {
            if (m_patientFillerInput->hasAllLabels(candidatesFillerSteps.at(i)->getRequiredLabels()))
            {
                fillerStepsToProcess.append(candidatesFillerSteps.at(i));
                continueIterating = true;
            }
            else
            {
                newCandidatesFillerSteps.append(candidatesFillerSteps.at(i));
            }
        }
        candidatesFillerSteps = newCandidatesFillerSteps;

        // Ordenem segons la seva prioritat
        qSort(fillerStepsToProcess.begin(), fillerStepsToProcess.end(), patientFillerMorePriorityFirst);

        foreach (PatientFillerStep *fillerStep, fillerStepsToProcess)
        {
            fillerStep->setInput(m_patientFillerInput);
            fillerStep->fillIndividually();
        }
    }

    m_patientFillerInput->initializeAllLabels();

    emit progress(++m_imageCounter);
}

void PatientFiller::finishDICOMFilesProcess()
{
    foreach (PatientFillerStep *fillerStep, m_registeredSteps)
    {
        fillerStep->postProcessing();
    }

    emit patientProcessed(m_patientFillerInput->getPatient());

    // Al acabar hem de reiniciar el comptador d'imatges
    m_imageCounter = 0;
}

QList<Patient*> PatientFiller::processFiles(const QStringList &files)
{
    // HACK per fer el cas especial dels mhd. Això està així perquè perquè el mètode
    // processDICOMFile s'espera un DICOMTagReader, que no podem crear a partir d'un mhd.
    // El filler d'mhd realment no s'està utilitzant a dintre del process de fillers com la resta.
    if (containsMHDFiles(files))
    {
        return processMHDFiles(files);
    }
    else
    {
        return processDICOMFiles(files);
    }
}

bool PatientFiller::containsMHDFiles(const QStringList &files)
{
    if (!files.isEmpty())
    {
        return files.first().endsWith(".mhd", Qt::CaseInsensitive);
    }
    else
    {
        return false;
    }
}

QList<Patient*> PatientFiller::processMHDFiles(const QStringList &files)
{
    PatientFillerInput patientFillerInput;
    m_imageCounter = 0;
    foreach (const QString &file, files)
    {
        patientFillerInput.setFile(file);

        MHDFileClassifierStep mhdFileClassiferStep;
        mhdFileClassiferStep.setInput(&patientFillerInput);
        if (!mhdFileClassiferStep.fillIndividually())
        {
            DEBUG_LOG("No s'ha pogut processar el fitxer MHD: " + file);
            ERROR_LOG("No s'ha pogut processar el fitxer MHD: " + file);
        }
        emit progress(++m_imageCounter);
    }

    return patientFillerInput.getPatientsList();
}

QList<Patient*> PatientFiller::processDICOMFiles(const QStringList &files)
{
    m_imageCounter = 0;

    foreach (const QString &dicomFile, files)
    {
        DICOMTagReader *dicomTagReader = new DICOMTagReader(dicomFile);
        if (dicomTagReader->canReadFile())
        {
            this->processDICOMFile(dicomTagReader);
        }

        emit progress(++m_imageCounter);
    }

    foreach (PatientFillerStep *fillerStep, m_registeredSteps)
    {
        fillerStep->postProcessing();
    }

    return m_patientFillerInput->getPatientsList();
}

}
