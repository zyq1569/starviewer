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

#ifndef UDGPATIENTFILLER_H
#define UDGPATIENTFILLER_H

#include <QStringList>

#include "dicomsource.h"

namespace udg {

class PatientFillerInput;
class PatientFillerStep;
class Patient;
class DICOMTagReader;

/**
    Classe que s'encarrega de "omplir" un Patient a partir de fitxers DICOM. Bàsicament té dos modes d'operació: "asíncron" i "síncron".

    En el mode d'operació asíncron el que podem fer és anar enviant fitxers dicom a processar i, un cop no tenim més fitxers, indiquem que
    que finalitzi el procés per obtenir el resultat amb finishDICOMFilesProcess. El resultat ens el donarà en el signal
    patientProcessed(Patient*).

    En canvi, en el mode d'operació síncron, simplement cal cridar el mètode processFiles passant la llista de fitxers que
    volem processar.

    El mètode asíncron està pensant per poder processar fitxers DICOM un a un a mesura que els anem obtenit en threads diferents, siguent
    recomenable el mètode síncron en qualsevol altra situació.

    No es poden utilitzar els dos mètodes de processament alhora en el mateix fitxer.

    TODO De moment hi ha la limitiació de que es pressuposa que totes les imatges que se li passen són del mateix pacient.
  */
class PatientFiller : public QObject {
Q_OBJECT
public:
    PatientFiller(DICOMSource dicomSource = DICOMSource(), QObject *parent = 0);
    ~PatientFiller();

public slots:
    /// Processem un fitxer DICOM. Ens permet anar passant fitxers un a un i, un cop acabem, cridar el mètode finishDICOMFilesProcess
    /// per obtenir el resultat a partir del signal patientProcessed.
    /// Es presuposa que el fitxer DICOM passat no està buit.
    void processDICOMFile(DICOMTagReader *dicomTagReader);

    /// Indica que ja hem acabat de processar fitxers i ja podem obtenir el resultat final, és a dir, l'estructura Patient omplerta.
    /// Aquesta se'ns dona a partir del signal patientProcessed.
    void finishDICOMFilesProcess();

    /// Processa tots els fitxers que se li passin de cop, retornant la llista d'objectes Patient que es generin.
    QList<Patient*> processFiles(const QStringList &files);

signals:
    /// Senyal que s'emet cada vegada que es processa un fitxer indicant quin és dintre del "lot" a processar.
    void progress(int);

    /// Senyal que s'emet quan en el mode asíncron s'ha acabat de processar totes les images passades.
    void patientProcessed(Patient *patient);

private:
    /// S'encarrega de registrar els mòduls/steps que processaran l'input.
    // TODO Això en un futur ho farà una classe registradora, ara es fa tot aquí per conveniència
    void registerSteps();

    /// Ens diu si la llista d'arxius conté fitxers mhd
    bool containsMHDFiles(const QStringList &files);

    /// Processa els arxius assumint que aquests són MHD i ens retorna la pertinent llista de pacients
    QList<Patient*> processMHDFiles(const QStringList &files);

    /// Processa els arxius assumint que aquests són DICOM i ens retorna la pertinent llista de pacients
    QList<Patient*> processDICOMFiles(const QStringList &files);

private:
    /// Registre d'steps
    QList<PatientFillerStep*> m_registeredSteps;

    /// S'encarrega de guardar l'input durant tota l'execucció dels mòduls. S'utilitza
    /// en cas que es processin fitxer individualment.
    PatientFillerInput *m_patientFillerInput;

    // Contador per saber el núm. d'imatge que estem tractant.
    int m_imageCounter;
};

}

#endif
