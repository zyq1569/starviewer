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


#ifndef UDGLOCALDATABASEMANAGER_H
#define UDGLOCALDATABASEMANAGER_H

#include <QList>
#include <QObject>

#include "image.h"
#include "series.h"
#include "study.h"
#include "patient.h"

namespace udg {

class DicomMask;
class DatabaseConnection;
class DisplayShutter;

/**
    Manager de la base de dades local, permet interactuar amb tots els objectes de la base de dades
  */
class LocalDatabaseManager : public QObject {
Q_OBJECT
public:
    /// Es defineix els tipus d'error que podem tenir, el DatabaseError indica quan és error de Sqlite
    enum LastError { Ok, DatabaseLocked, DatabaseCorrupted, SyntaxErrorSQL, DeletingFilesError, DatabaseError, PatientInconsistent };

    LocalDatabaseManager();

    /// Guarda les dades de la sèria a la BD, guarda el pacient i estudi de la sèrie, i les imatges que conté,
    /// si aquests no existeixes insereix a les dades
    /// i si alguna de les dades ja existeix l'actualitza
    void save(Series *seriesToSave);

    /// Ens retorna els pacients que compleixen amb els criteris de la màscara, només es té en compte el patientID
    QList<Patient*> queryPatient(const DicomMask &patientMaskToQuery);

    /// Ens retorna els estudis que compleixen amb els criteris de la màscara, només es té en compte l'StudyUID
    QList<Study*> queryStudy(const DicomMask &studyMaskToQuery);

    /// Ens retorna les series que compleixen amb els criteris de la màscara, només es té en compte l'StudyUID i SeriesUID
    QList<Series*> querySeries(const DicomMask &seriesMaskToQuery);

    /// Ens retorna les imatges que compleixen amb els criteris de la màscara, només es té en compte l'StudyUID, SeriesUID i SOPInstanceUID
    QList<Image*> queryImage(const DicomMask &imageMaskToQuery);

    /// Ens retorna els pacients que tenen estudis que compleixen amb els criteris de la màscara.
    /// Té en compte el patientID, patient name, data de l'estudi i l'study instance UID
    /// Retorna l'estructura omplerta fins al nivell d'study (no omple ni les sèries ni les imatges).
    QList<Patient*> queryPatientStudy(const DicomMask &patientStudyMaskToQuery);

    /// Retorna si existeix l'estudi a la base de dades. Comprova si hi ha una estudi a la base de dades amb l'StudyInstanceUID de l'estudi passat.
    bool existsStudy(Study *study);

    /// Retorna tota l'estructura Patient,Study,Series,Image, de l'estudi que compleix amb el criteri de cerca.
    /// Té en compte el Study InstanceUID, el SeriesInstanceUID i el SOP Instance UID
    Patient* retrieve(const DicomMask &maskToRetrieve);

    /// Esborra de la base de dades i del disc l'estudi passat per paràmetre
    void deleteStudy(const QString &studyInstanceUIDToDelete);

    /// Esborra la sèrie passada per paràmetre, si l'estudi al qual pertany la sèrie només té aquesta
    /// també s'esborra l'estudi, ja que no té cap sentit tenir un estudi sense sèries inserit a la BD
    void deleteSeries(const QString &studyInstanceUID, const QString &seriesInstanceUID);

    /// Esborra els estudis vells que fa que superen el temps màxim que poden estar a la base de dades sense ser visualitzats.
    /// Si la opció de configuració d'esborrar els estudis vells que no han estat visualitzats en un determinat
    ///  número de dies no està activada aquest mètode no farà res
    void deleteOldStudies();

    /// Compacta la base de dades
    void compact();

    /// Retorna la revisió de la base de dades, sinó ha trobat de quina revisió és la base de dades retorna -1
    int getDatabaseRevision();

    /// Ens actualitza la revisió de la base de dades
    void setDatabaseRevision(int databaseRevision);

    /// Comprova si es disposa d'espai suficient al disc dur per descarregar nous objectes.
    /// Si la opció de configuració d'esborrar estudis automàticament està activada intentarà
    /// esborrar estudis vells tant GB d'estudis vell com s'hagi especificat a la configuració
    /// per tal d'alliberar suficient espai per permetre noves descàrregues
    bool thereIsAvailableSpaceOnHardDisk();

    /// Donat un study instance UID ens indica a quin ha de ser el directori de l'estudi
    QString getStudyPath(const QString &studyInstanceUID);

    LastError getLastError();

    /// Ens permet indicar que tenim un estudi que s'està descarregant, aquest mètode ens permet que en el cas
    /// que l'starviewer tanqui de forma anómala, saber quin estudis s'estava descarregant, per deixar la
    /// base de dades local en un estat consistent.
    /// Atenció! Aquest mètode només permet establir com a Retrieving un estudi a la vegada
    /// @return retorna indicant si s'ha pogut realitzar l'operació amb èxit, si indica fals serà perquè ja hi ha un estudi descarregant
    bool setStudyRetrieving(const QString &studyInstanceUID);

    /// Indiquem que l'estudi que s'havia indicat a través del mètode setStudyRetrieving ja s'ha descarregat
    void setStudyRetrieveFinished();

    /// Aquest mètode està pensat pel cas de que mentre s'està descarregant un estudi, l'starviewer finalitzi de forma anómala.
    /// El mètode comprovarà si teníem estudies en estat de descarregant i si és així esborra les imatges descarregades fins
    /// el moment i deixarà la base de dades co
    void checkNoStudiesRetrieving();

    /// Indica si hi algun estudi descarregant
    bool isStudyRetrieving();

    /// Ens dóna la ruta absoluta al fitxer de bases de dades, fitxer de bases de dades inclós
    static QString getDatabaseFilePath();

    /// Ens dóna la ruta absoluta al directori on es troba la caché
    static QString getCachePath();

signals:
    /// Aquest signal s'emet per indicar que un estudi serà borrat de la base de dades per alliberar espai
    void studyWillBeDeleted(const QString &studyInstanceUID);

public slots:
    /// Guarda el pacient a la base de dades, si no existeix insereix les dades, i si alguna de les dades ja existeix a la BD l'actualitza
    void save(Patient *newPatient);

private:
    /// Ens retorna els estudis que compleixen amb els criteris de la màscara, només es té en compte l'StudyUID ordenats per LastAccessDate de forma creixen
    QList<Study*> queryStudyOrderByLastAccessDate(const DicomMask &studyMaskToQuery);

    /// Guarda a la base de dades la llista d'estudis passada per paràmetre, si algun dels estudis ja existeix actualitza la info
    int saveStudies(DatabaseConnection *dbConnect, QList<Study*> listStudyToSave, const QDate &currentDate, const QTime &currentTime);

    /// Guarda a la base de dades la llista de series passada per paràmetre, si alguna de les series ja existeix actualitza la info
    int saveSeries(DatabaseConnection *dbConnect, QList<Series*> listSeriesToSave, const QDate &currentDate, const QTime &currentTime);

    /// Guarda a la base de dades la llista d'imatges passada per paràmetre, si alguna de les imatges ja existeix actualitza la info
    int saveImages(DatabaseConnection *dbConnect, QList<Image*> listImageToSave, const QDate &currentDate, const QTime &currentTime);

    /// Guarda a la base de dades la llista de display shutters relacionades amb la imatge passada per paràmetre
    int saveDisplayShutters(DatabaseConnection *dbConnect, QList<DisplayShutter> shuttersList, Image *relatedImage);
    
    /// Guarda el pacient a la base de dades, si ja existeix li actualitza la informació
    int savePatientOfStudy(DatabaseConnection *dbConnect, Study *study);

    /// Guarda el pacient a la base de dades, si ja existeix li actualitza la informació
    int saveStudy(DatabaseConnection *dbConnect, Study *studyToSave);

    /// Guarda el pacient a la base de dades, si ja existeix li actualitza la informació
    int saveSeries(DatabaseConnection *dbConnect, Series *seriesToSave);

    /// Guarda la imatge a la base de dades, si ja existeix li actualitza la informació
    int saveImage(DatabaseConnection *dbConnect, Image *imageToSave);

    /// Esborra a base la jerarquia pacient/estudi/series/imatge de l'estudi passat per paràmetre, si es passar un valor buit no esborra res.
    int deleteStudyStructureFromDatabase(DatabaseConnection *dbConnect, const QString &studyInstanceUIDToDelete);

    /// Esborra a base la jerarquia /series/imatge de l'estudi passat per paràmetre. Si només es passa el studyInstaceUIDToDelete esborra totes
    /// les sèries d'aquell estudi.
    int deleteSeriesStructureFromDatabase(DatabaseConnection *dbConnect, const QString &studyInstanceUIDToDelete, const QString &seriesIntanceUID);

    /// Esborra el pacient que compleixi amb la màscara a esborrar.
    /// A la màscara hem d'indicar el UID de l'estudi a esborrar i comprova si el pacient
    /// el qual pertany l'estudi té més d'un estudi, si és així no l'esborrar, si només en té un l'esborra
    int deletePatientOfStudyFromDatabase(DatabaseConnection *dbConnect, const DicomMask &maskToDelete);

    /// Esborra el pacient que compleix la màscara passada per paràmetre, només es té en compte el patientID
    int deletePatientFromDatabase(DatabaseConnection *dbConnect, qlonglong patientID);

    /// Esborra el l'estudi que compleixi amb la màscara a esborrar
    int deleteStudyFromDatabase(DatabaseConnection *dbConnect, const DicomMask &maskToDelete);

    /// Esborra la serie que compleixi amb la màscara a esborrar
    int deleteSeriesFromDatabase(DatabaseConnection *dbConnect, const DicomMask &maskToDelete);

    /// Esborra la imatge que compleixi amb la màscara a esborrar
    int deleteImageFromDatabase(DatabaseConnection *dbConnect, const DicomMask &maskToDelete);

    /// Aquesta classe s'encarrega d'esborrar les objectes descarregats si es produeix un error mentre s'insereixen els nous objectes a la base de dades
    void deleteRetrievedObjects(Patient *failedPatient);

    /// Aquesta classe s'encarrega d'esborrar les objectes descarregats si es produeix un error mentre s'insereixen els nous objectes a la base de dades
    void deleteRetrievedObjects(Series *failedSeries);

    /// Esborra estudis  fins alliberar l'espai passat per paràmetre, comença esborrant els estudisque fa més que no es visualitzen
    void freeSpaceDeletingStudies(quint64 MbytesToErase);

    /// Esborra l'estudi del disc dur
    void deleteStudyFromHardDisk(const QString &studyInstanceToDelete);

    /// Esborra la sèrie del disc dur
    void deleteSeriesFromHardDisk(const QString &studyInstanceToDelete, const QString &seriesInstanceToDelete);

    /// Crea i guarda el thumbnails de les sèries de l'estudi al directori on estan guardades les imatges de cada serie
    void createStudyThumbnails(Study *studyToGenerateSeriesThumbnails);

    /// Crea i guarda el thumbnail de les sèries al directori on estan guardades les imatges de la serie
    void createSeriesThumbnail(Series *seriesToGenerateThumbnail);

    /// Ens emplena els thumbnails d'una llistà de sèries
    void loadSeriesThumbnail(QString studyInstanceUID, QList<Series*> series);

    /// Retorna el path + el nom del thumbnail d'una sèrie
    QString getSeriesThumbnailPath(QString studyInstanceUID, Series *series);

    /// Passant un status de sqlite ens el converteix al nostra status
    void setLastError(int sqliteLastError);

private:
    /// Guardem a partir de quina data de lastAccessDate cerquem els estudis.
    /// D'aquesta manera sabem quins estudis vells s'han d'esborrar, quins hem de mostrar al fer cerques i evitem incoherències, com la que podria
    /// ser que al cercar estudis en una mateixa sessió de l'apliació a les 23:59,o a les 0:01 de l'endemà donint resultats diferents, perquè hi han estudis
    /// que passen a ser considerats estudis vells. D'aquesta manera en tota la vida de l'aplicació mantenim  el mateix criteri de data per establir
    /// si un estudi es vell o no i s'ha de mostrar a les cerques.
    static QDate LastAccessDateSelectedStudies;

    LastError m_lastError;
};

}

#endif
