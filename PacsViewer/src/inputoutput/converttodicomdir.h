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

#ifndef UDGCONVERTTODICOMDIR_H
#define UDGCONVERTTODICOMDIR_H

#include <QObject>
#include <QStringList>

#include "createdicomdir.h"

// Fordward declarations
class QProgressDialog;

namespace udg {

// Fordward declarations
class Status;
class Study;
class Series;
class Image;
class DICOMAnonymizer;

/**
    Converteix un estudi a DICOMDIR, invocant el mètodes i classes necessàries.
    Per crear un dicomdir, s'han de seguir les normes especificades a la IHE per PDI (portable data information) i DICOM : Aquestes normes són :
    El nom dels directoris i imatges no pot ser de mes de 8 caràcters, i ha d'estar en majúscules
    Les imatges no poden tenir extensió
    S'ha de seguir l'estructura jeràrquica de directoris de Pacient/Estudi/Series/Imatges
    La imatge ha d'estar en format littleEndian
  */
class ConvertToDicomdir : public QObject {
Q_OBJECT
public:
    ConvertToDicomdir();

    /// Ens permet indicar que volem anonimitzar l'estudi DICOMDIR, i en el cas que l'anonimitzem se li pot indicar quin nom de pacient que han de tenir els
    /// estudis anonimitzats. Si s'indica que no es vol anonimitzar l'estudi i es passar un valor al segon paràmetre aquest s'ignorarà.
    /// TODO: Repassar al fer refactoring de la classe si és la millor manera de passar el nom de pacientAnonimitzat
    void setAnonymizeDICOMDIR(bool anonymizeDICOMDIR, QString patientNameAnonymized = "");

    /// Afegeix un estudi a la llista per convertir-se a dicomsdir. Quan afageix l'estudi, l'afageix a la llista ordenats per pacient. Ja que els dicomdir
    /// s'han d'agrupar primerament per pacients
    /// @param studyUID UID de l'estudi a convertir a dicomdir
    void addStudy (const QString &studyUID);

    /// Converteix a DICOMDIR en el path especificat els estudis que hi ha a la llista. ATENCIÓ!!! Si al crear el DICOMDIR falla s'esborrar el contingut
    /// de la carpeta destí. La carpeta destí ha d'estar buida sinó la creació del DICOMDIR fallà i esborrarà tot el contingut del a carperta destí.
    /// @param dicomdirPath directori on es guardarà el dicomdir
    /// @param indica si s'ha de copiar el contingut del directori guardat al settings InputOutputSettings::DICOMDIRFolderPathToCopy al DICOMDIR
    /// @return Indica l'estat en què finalitza el mètode
    // TODO:La comprovació de que la carpeta destí estigui buida es fa a QCreateDicomdir s'hauria de traslladar en aquesta classe
    Status convert(const QString &dicomdirPath, CreateDicomdir::recordDeviceDicomDir selectedDevice, bool copyFolderContent);

    /// Crea un fitxer README.TXT, amb informació sobre quina institució ha generat el dicomdir per quan es grava en un cd o dvd en el path que se
    /// li especifiqui. En el cas que el txt es vulgui afegir en el mateix directori arrel on hi ha el dicomdir s'haura de fer després d'haver convertir
    /// el directori en un dicomdir, si es fes abans el mètode de convertir el directori a dicomdir fallaria, perquè no sabia com tractar el README.txt
    void createReadmeTxt();

    /// Especifica/Retorna si les imatges amb les que crearan el dicomdir s'han de convertir a LittleEndian o han de mantenir
    /// la transfer syntax original, si no s'especifica per defecte les imatges mantenen la seva Transfer Syntax original
    void setConvertDicomdirImagesToLittleEndian(bool convertDicomdirImagesToLittleEndian);
    bool getConvertDicomdirImagesToLittleEndian();

    /// Starviewer pot copiar el contingut d'un directori especificat per l'usuari al DICOMDIR que es crea. Aquest directori ha de complir
    /// un requeriment: que no contingui cap fitxer ni carpeta que s'anomeni DICOMDIR o DICOM.
    /// Aquest mètode ens comprova que es compleixi aquest requeriment.
    bool AreValidRequirementsOfFolderContentToCopyToDICOMDIR(QString path);

private:
    /// Estructura que conté la informació d'un estudi a convertir a dicomdir.
    /// És necessari guardar el Patient ID perquè segons la normativa de l'IHE,
    /// els estudis s'han d'agrupar per id de pacient
    struct StudyToConvert
        {
            QString patientId;
            QString studyUID;
        };

    /// Crea un dicomdir, al directori especificat
    /// @param dicomdirPath lloc a crear el dicomdir
    /// @param selectedDevice dispositiu on es crearà el dicomdir
    /// @return  estat del mètode
    Status createDicomdir(const QString &dicomdirPath, CreateDicomdir::recordDeviceDicomDir selectedDevice);

    /// Copia els estudis seleccionats per passar a dicomdir, al directori desti
    Status copyStudiesToDicomdirPath(QList<Study*> studyList);

    /// Converteix un estudi al format littleendian, i la copia al directori dicomdir
    /// @param studyUID Uid de l'estudi a convertir
    /// @return Indica l'estat en què finalitza el mètode
    Status copyStudyToDicomdirPath(Study *study);

    /// Converteix una sèrie al format littleendian, i la copia al directori dicomdir
    /// @param series
    /// @return Indica l'estat en què finalitza el mètode
    Status copySeriesToDicomdirPath(Series *series);

    /// Does the process of copying a list of images to the corresponding DICOMDIR destination 
    Status copyImages(QList<Image*> images);
    
    /// Converteix una imatge al format littleendian, i la copia al directori dicomdir
    /// @param image
    /// @return Indica l'estat en què finalitza el mètode
    Status copyImageToDicomdirPath(Image *image);

    /// Gets the corresponding output prefix name
    QString getDICOMDIROutputFilenamePrefix() const;

    /// Gets the corresponding output path for the current item to be copied
    QString getCurrentItemOutputPath();
    
    /// Copies source file to destination file and sets the Status for the operation
    void copyFileToDICOMDIRDestination(const QString &sourceFile, const QString &destinationFile, Status &status);

    /// Anonymizes sourceFile and puts the result in destinationFile.
    /// isLittleEndian is needed in order to give an accurate message in status in case there are some error.
    void anonymizeFile(const QString &sourceFile, const QString &destinationFile, Status &status, bool isLittleEndian);
    
    /// Starviewer té l'opció de copiar el contingut d'una carpeta al DICOMDIR. Aquest mètode copia el contingut de la carpeta al DICOMDIR
    bool copyFolderContentToDICOMDIR();

private:
    QList<StudyToConvert> m_studiesToConvert;
    QProgressDialog *m_progress;
    bool m_convertDicomdirImagesToLittleEndian;

    QString m_dicomDirPath;
    QString m_dicomDirStudyPath;
    QString m_dicomDirSeriesPath;
    QString m_oldPatientId;
    QString m_dicomdirPatientPath;

    QStringList m_patientDirectories;

    int m_patient;
    int m_study;
    int m_series;
    /// Holds the number of the item being copied
    int m_currentItemNumber;

    /// És necessari crear-la global per mantenir la consistència dels UID dels fitxers DICOM
    DICOMAnonymizer *m_DICOMAnonymizer;
    bool m_anonymizeDICOMDIR;
    QString m_patientNameAnonymized;
};

}

#endif
