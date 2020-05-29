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

#ifndef UDGPATIENTFILLERINPUT_H
#define UDGPATIENTFILLERINPUT_H

#include <QStringList>
#include <QMultiMap>
#include <QHash>

#include "dicomsource.h"

namespace udg {

class Patient;
class Series;
class Image;
class DICOMTagReader;

/**
    Classe que encapsula els paràmetres d'input que es faran servir a PatientFiller
  */
class PatientFillerInput {
public:
    PatientFillerInput();

    ~PatientFillerInput();

    /// Afegeix un pacient a la llista
    void addPatient(Patient *patient);

    /// Obté un pacient de la llista indexat. Si l'índex supera el nombre de membres de la llista es retorna NUL
    Patient* getPatient(int index = 0);

    /// Obté un pacient identificat pel seu nom
    Patient* getPatientByName(QString name);

    /// Obté un pacient identificat pel seu ID
    Patient* getPatientByID(QString id);

    /// Retorna el nombre de pacients que tenim a la llista
    unsigned int getNumberOfPatients();

    /// Retorna la llista de Patients que tenim.
    QList<Patient*> getPatientsList();

    /// Assigna/Obté l'arxiu a tractar. TODO: De moment és independent del setDICOMFile i només té sentit per fitxers
    /// no DICOM, com els fitxers MHD. Per DICOM cal utilitzar setDICOMFile(DICOMTagReader*).
    void setFile(QString file);
    QString getFile() const;

    /// Afegim etiquetes a nivell global/Series
    void addLabel(QString label);
    void addLabelToSeries(QString label, Series *series);

    /// Obtenim totes les etiquetes que s'han aplicat fins al moment, tant a nivell global com a nivell de sèries.
    /// Retorna una llista composada per les etiquetes globals i de series
    QStringList getLabels() const;

    /// Retorna true en el cas que es tinguin tots els labels (ja sigui a nivell de sèrie o global)
    bool hasAllLabels(QStringList requiredLabelsList) const;

    /// Buida totes les llistes d'etiquetes.
    void initializeAllLabels();

    /// S'indica/obté quin serà el DICOMTagReader a processar. Aquest mètode esborrarà l'objecte que es tenia guardat
    /// anteriorment fent que no es pugui utilitzar més: es pren el control absolut de l'objecte.
    /// Per objectes no dicom, cal utilitzar set/getFile(QString)
    void setDICOMFile(DICOMTagReader *dicomTagReader);
    DICOMTagReader* getDICOMFile();

    /// Assignar/Obtenir la llista d'imatges que s'han de processar.
    void setCurrentImages(const QList<Image*> &images);
    QList<Image*> getCurrentImages();

    /// Afegir / Obtenir la sèrie del fitxer que s'ha de processar.
    void setCurrentSeries(Series *series);
    Series* getCurrentSeries();

    /// Incrementa el número de volum (multiframe) actual
    void increaseCurrentMultiframeVolumeNumber();

    /// Retorna el número de volum (multiframe) actual
    int getCurrentMultiframeVolumeNumber();

    /// Returns true if the current series already contains at least one multiframe volume.
    bool currentSeriesContainsAMultiframeVolume() const;

    /// Incrementa el número de volum (single frame) actual
    void increaseCurrentSingleFrameVolumeNumber();

    /// Retorna el corresponent número de volum pel conjunt d'imatges single frame actual
    int getCurrentSingleFrameVolumeNumber();

    /// Assigna/Retorna el número de volum actual que estem tractant, necessari pels
    /// passos posteriors a l'ImageFillerStep
    void setCurrentVolumeNumber(int volumeNumber);
    int getCurrentVolumeNumber() const;

    /// Assigna/Retorna el DICOMSource de les imatges
    void setDICOMSource(const DICOMSource &imagesDICOMSource);
    DICOMSource getDICOMSource() const;

private:
    /// Llista de pacients a omplir
    QList<Patient*> m_patientList;

    /// Arxius que cal tractar per omplir la llista de pacients
    QString m_file;

    /// Llista d'etiquetes assignades a nivell global
    QStringList m_globalLabels;

    /// Llista que té totes les labels aplicades sense repeticions de labels globals i de series.
    /// Aquesta variable la tenim per ser més àgils en el getLabels per ser una mica més ràpids.
    QStringList m_allLabels;

    /// Llista d'etiquetes assignades a nivell de sèries. Per cada Series tenim vàries etiquetes
    QMultiMap<Series*, QString> m_seriesLabels;

    /// Atribut que s'utilitza per executar els fillers individualment.
    DICOMTagReader *m_dicomFile;

    /// Guarda les imatges que els fillers han de processar.
    QList<Image*> m_currentImages;

    /// Guardem la sèrie del fitxer que els fillers han de processar. S'utilitza si es vol exectuar els fillers individualment per fitxers.
    Series *m_currentSeries;

    /// Guardem el volume number de la sèrie que els fillers han de processar. S'utilitza si es vol exectuar els fillers individualment per fitxers.
    int m_currentVolumeNumber;

    /// Manté el número actual de volum pel subconjunt de volums multiframe
    QHash<Series*, int> m_currentMultiframeVolumeNumber;

    /// Manté el número actual de volum pel subconjunt de volums single frame
    QHash<Series*, int> m_currentSingleFrameVolumeNumber;

    /// Conté el DICOMSource del qual provenen les imatges
    DICOMSource m_imagesDICOMSource;
};

}

#endif
