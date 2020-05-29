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

#ifndef UDGRELATEDSTUDIESMANAGER_H
#define UDGRELATEDSTUDIESMANAGER_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QDate>

#include "pacsdevice.h"
#include "pacsjob.h"

namespace udg {

class Patient;
class Study;
class DicomMask;
class PacsManager;
class QueryPacsJob;

/**
    Aquesta classe donat un Study demana els estudis relacionats o previs en els PACS configurats per defecte, degut a que
    ara actualment en el PACS podem tenir pacients que són el mateix però amb PatientID diferents, també a part de cercar estudis
    que coincideixin amb el PatientID també es farà una altre cerca per Patient Name.
  */
/* TODO: En teoria amb la implantació del SAP els problemes de que un Pacient té diversos Patient ID o que té el nom
   escrit de maneres diferents haurien de desapareixer, per tant d'aquí un temps quan la majoria d'estudis del PACS
   ja s'hagin fet a través del SAP i constatem que el Patient ID pel mateix pacient sempre és el mateix,
   la cerca per nom de pacient podria desapareixer
 */
class RelatedStudiesManager : public QObject {
Q_OBJECT
public:
    RelatedStudiesManager();
    ~RelatedStudiesManager();

    /// Enum to know if loadStudy method was able to load the study from database, it's being retrieved or failed.
    enum LoadStatus { Loaded, Retrieving, Failed };

    /// Fa una consulta d'estudis previs assíncrona als PACS que estiguin marcats per defecte, si dos del PACS retornen el mateix estudi només es tindrà en compte
    /// el del primer PACS que ha respós
    /// Si ja s'estigués executant una consulta la cancel·laria i faria la nova consulta
    void queryMergedPreviousStudies(Study *study);

    /// Fa una consulta d'estudis del pacient assíncrona als PACS que estiguin marcats per defecte, si dos del PACS retornen el mateix estudi només es tindrà en compte
    /// el del primer PACS que ha respós
    /// Si ja s'estigués executant una consulta la cancel·laria i faria la nova consulta
    void queryMergedStudies(Patient *patient);

    /// Cancel·la les consultes actuals que s'estan executant, i cancel·la les consultes encuades per executar
    void cancelCurrentQuery();

    /// Indica si s'executen queries en aquest moment
    bool isExecutingQueries();

    /// Retrieves (only) the given study from the specified PACS
    void retrieve(Study *study, const PacsDevice &pacsDevice);

    /// Retrieves and loads the given study from the specified PACS
    void retrieveAndLoad(Study *study, const PacsDevice &pacsDevice);

    /// Retrieves and views the given study from the specified PACS
    void retrieveAndView(Study *study, const PacsDevice &pacsDevice);

    /// Load study from the database. If it is not in the database it is retrieved from PACS before being loaded.
    RelatedStudiesManager::LoadStatus loadStudy(Study *study);

    /// Return the list of studies of the given patient stored in the database
    QList<Study*> getStudiesFromDatabase(Patient *patient);

signals:
    /// Signal que s'emet quan ha finalitzat la consulta d'estudis. La llista amb els resultats s'esborrarà quan es demani una altra cerca.
    void queryStudiesFinished(QList<Study*>);

    /// Signal que s'emet per indicar que s'ha produït un error a la consulta d'estudis d'un PACS
    void errorQueryingStudies(PacsDevice pacs);

    /// Signal que s'emet per indicar que s'ha produït un error durant la descarrega d'un estudi (pot ser previ o no)
    void errorDownloadingStudy(QString studyUID);

private:
    /// Realitza una consulta dels estudis del pacient "patient" als PACS marcats per defecte.
    /// Si s'especifica una data "until" només cercarà els estudis fins la data especificada (aquesta inclosa).
    /// Si no es passa cap data per paràmetre cercarà tots els estudis, independentment de la data.
    void makeAsynchronousStudiesQuery(Patient *patient, QDate untilDate = QDate());

    /// Retorna una màscara de cerca base a partir de les quals es generan les DicomMask per cercar estudis relacionats
    DicomMask getBasicDicomMask();

    /// Ens indica si aquell estudi està a la llista d'estudis ja rebuts, per evitar duplicats
    /// Hem de tenir en compte que com fem la cerca per ID i un altre per Patient Name per obtenir més resultats
    /// potser que en les dos consultes ens retornin el mateix estudi, per tant hem d'evitar duplicats.
    bool isStudyInMergedStudyList(Study *study);

    /// Ens indica si aquest estudi és el mateix pel qual ens han demanat els estudis relacionts, per evitar incloure'l a la llista
    bool isMainStudy(Study *study);

    /// Inicialitza les variables per realitzar una nova consulta
    void initializeQuery();

    /// Ens encua el QueryPACSJob al PACSManager i ens connecta amb els seus signals per poder processar els resultats. També afegeix el Job en una taula
    /// de hash on es guarden tots els QueryPACSJobs demanats per aquesta classe que estant pendents d'executar-se o s'estan executant
    void enqueueQueryPACSJobToPACSManagerAndConnectSignals(PACSJobPointer queryPACSJob);

    /// Ens afegeix els estudis trobats en una llista, si algun dels estudis ja existeix a la llista perquè s'ha trobat en algun altre PACS no
    /// se li afegeix
    void mergeFoundStudiesInQuery(PACSJobPointer queryPACSJob);

    /// Emet signal indicant que la consulta a un PACS ha fallat
    void errorQueringPACS(PACSJobPointer queryPACSJob);

    /// Emet signal indicant la la consulta ha acabat
    void queryFinished();

    /// Esborra els resultats de la cerca
    void deleteQueryResults();

    /// Retorna una llista indicant de quins PACS s'han descarregat els estudis que conté el pacient, sempre que continguin aquesta informació al DICOMSource
    QList<PacsDevice> getPACSRetrievedStudiesOfPatient(Patient *patient);
    
    /// TODO This enum is the very same as QInputOutputPacsWidget::ActionsAfterRetrieve. We don't use that to avoid dependencies in the header file.
    /// Maybe a common enum for both classes could improve this.
    enum ActionsAfterRetrieve { None, View, Load };
    
    /// Retrieves the given study from the specified PACS and applies the given action upon retrieval
    void retrieveAndApplyAction(Study *study, const PacsDevice &pacsDevice, ActionsAfterRetrieve action);

    /// Return DICOM Masks to know what to query. It takes into accound
    /// the PatientID and PatientName properties of the patient and the value of m_searchRelatedStudiesByName
    QList<DicomMask> getDicomMasks(Patient *patient);

private slots:
    /// Slot que s'activa quan finalitza un job de consulta al PACS
    void queryPACSJobFinished(PACSJobPointer pacsJob);

    /// Slot que s'activa quan un job de consulta al PACS és cancel·lat
    void queryPACSJobCancelled(PACSJobPointer pacsJob);

private:
    PacsManager *m_pacsManager;
    QList<Study*> m_mergedStudyList;

    /// Study instance UID de l'estudi a partir del qual hem de trobar estudis relacionats
    QString m_studyInstanceUIDOfStudyToFindRelated;
    
    /// Com fem una consulta dos consultes al mateix PACS si falla una segurament també fallarà la segona per això
    /// en aquesta llista registrarem l'ID dels Pacs pel quals hem emés el signal d'error i si rebem un segon error
    /// com ja el tindrem aquesta llista ja no en farem signal
    QStringList m_pacsDeviceIDErrorEmited;
    /// Hash que ens guarda tots els QueryPACSJob pendent d'executar o que s'estan executant llançats des d'aquesta classe
    QHash<int, PACSJobPointer> m_queryPACSJobPendingExecuteOrExecuting;
    /// Boolea per saber si s'ha de cercar estudis relacionats a partir del nom del pacient.
    bool m_searchRelatedStudiesByName;
};

}

#endif // UDGRELATEDSTUDIESMANAGER_H
