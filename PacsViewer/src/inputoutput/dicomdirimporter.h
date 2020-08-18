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

#ifndef UDGDICOMDIRIMPORTER_H
#define UDGDICOMDIRIMPORTER_H

#include <QObject>

#include "dicomdirreader.h"
#include <QProgressDialog>

class QString;
class QThread;

namespace udg {

class Image;
class DICOMTagReader;
class PatientFiller;
class LocalDatabaseManager;

/**
    Aquesta classe permet importar un dicomdir a la nostra base de dades.
    Només suporta importar dades d'un sol pacient a cada crida, per tant,
    cal assegurar-se que se li passa un studyUID correcte.
  */
class DICOMDIRImporter : public QObject {
Q_OBJECT

public:
    enum DICOMDIRImporterError { Ok, DatabaseError, NoEnoughSpace, ErrorFreeingSpace, ErrorCopyingFiles, PatientInconsistent,
                                 ErrorOpeningDicomdir, DicomdirInconsistent };

    /// Importa les dades del dicomdir que es trova a dicomdirPath que pertanyen a l'study amb UID studyUID
    void import(QString dicomdirPath, QString studyUID, QString seriesUID, QString imageUID);

    /// Retorna l'últim error produït al importar el dicomdir
    DICOMDIRImporterError getLastError();

signals:
    /// Senyal que ens indica que s'ha importat una imatge a disc. Quan s'emet aquest senyal encara no s'ha guardat a la bd.
    void imageImportedToDisk(DICOMTagReader *dicomTagReader);

    void importFinished();
    void importAborted();

private:
    DICOMDIRReader m_readDicomdir;
    DICOMDIRImporterError m_lastError;
    QProgressDialog *m_qprogressDialog;

    /// Crea les connexions necessàries per importar dicomdir
    void createConnections(PatientFiller *patientFiller, LocalDatabaseManager *localDatabaseManager, QThread *fillersThread);

    void importStudy(QString studyUID, QString seriesUID, QString sopInstanceUID);

    void importSeries(QString studyUID, QString seriesUID, QString sopInstanceUID);

    void importImage(Image *imageToImport, QString pathToImportImage);

    /// S'esborra de la caché les imatges que s'han importat en local d'un estudi que ha fallat la importació
    void deleteFailedImportedStudy(QString studyInstanceUID);

    /// Copia al disc dur una imatge del dicomdir
    bool copyDicomdirImageToLocal(QString dicomdirImagePath, QString localImagePath);

    /// Ens retorna el path de la imatge a importar, hem de tenir en compte que en funció del sistema de fitxers el nom del fitxer pot està en majúscules
    /// o minúscules, aquesta funció s'encarrega de comprovar-ho
    QString getDicomdirImagePath(Image *imageToImport);

    QString getDescriptionForQProgressDialog(QString studyInstanceUID, QString seriesInstanceUID, QString SOPInstanceUID);

};

}
#endif
