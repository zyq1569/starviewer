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

#ifndef UDGDICOMANONYMIZER_H
#define UDGDICOMANONYMIZER_H

#include <QHash>
#include <QString>

#include "gdcmanonymizerstarviewer.h"

class QString;

namespace udg {

/**
    Aquesta classe permet anonimitzar fitxers DICOM seguint les normes descrites pel Basic Application Level Confidentiality Profile de DICOM
    que podem trobar al annex E del PS 3.15, mantenint la consistència en tags com Frame Of Reference o Image Of Reference, Study Instance UID,
    Series Instance UID, ... després de ser anonimitzats. Per defecte també treu els tags privats de les imatges ja que aquests poden contenir
    informació sensible del pacient, ens aconsellen que els treiem a http://groups.google.com/group/comp.protocols.dicom/browse_thread/thread/fb89f7f5d120db44

    Ens permet anonimitzar fitxers sols o tots els fitxers dins i subdirectoris del directori especificat.
  */
class DICOMAnonymizer {

public:
    DICOMAnonymizer();
    ~DICOMAnonymizer();

    /// Ens anonimitza els fitxers d'un Directori
    bool anonymyzeDICOMFilesDirectory(const QString &directoryPath);

    /// Ens anonimitza un fitxer DICOM
    /// Atenció!!! si utilitzem aquesta opció per anonimitzar diversos fitxers d'un mateix estudi, aquests fitxers s'han d'anonimitzar utilitzant la mateixa
    /// instància del DICOMAnonymizer per mantenir la consitència de Tags com Study Instance UID, Series Instance UID, Frame Of Reference, Image Reference ...
    /// Si no es respecta aquest requisit passarà que imatges d'un mateix estudi després de ser anonimitzades tindran Study Instance UID diferents.
    bool anonymizeDICOMFile(const QString &inputPathFile, const QString &outputPathFile);

    /// Ens indica quin nom de pacient han de tenir els estudis anonimitzats. El nom no pot tenir més de 64 caràcters seguint la normativa DICOM per a tags de
    /// tipus PN (Person Name) si es passa un nom de més de 64 caràcters es trunca.
    void setPatientNameAnonymized(const QString &patientNameAnonymized);
    QString getPatientNameAnonymized() const;

    /// En comptes d'eliminar el valor del StudyID tal com indica el Basic Profile el substitueix per un valor arbitrari
    /// Aquesta opció està pensada pel DICOMDIR en que és obligatori que els estudis tinguin PatientID, tots els estudis a anonimitzar que tinguin en comú
    /// mateix el Patient ID abans de ser anonimitzats, després de ser-ho tindran un nou Patient ID en comú, d'aquesta manera es podrà veure que aquells estudis
    /// són del mateix pacient.
    void setReplacePatientIDInsteadOfRemove(bool replace);
    bool getReplacePatientIDInsteadOfRemove();

    /// En comptes d'eliminar el valor del StudyID tal com indica el Basic Profile el substitueix per un valor arbitrari
    /// Aquesta opció està pensada pel DICOMDIR en que és obligatori que els estudis tinguin StudyID
    void setReplaceStudyIDInsteadOfRemove(bool replace);
    bool getReplaceStudyIDInsteadOfRemove();

    /// Indica si s'han de treure els tags privats de les imatges. Per defecte es treuen si no s'indica el contrari, ja que els tags privats poden
    /// contenir informació sensible del pacient que segons el Basic Application Level Confidentiality Profile de DICOM s'ha d'anonimitzar, com podria
    /// ser el nom del pacient, edat, ....
    void setRemovePrivateTags(bool removePritaveTags);
    bool getRemovePrivateTags();

private:
    /// Inicialitza les variables de gdcm necessàries per anonimitzar
    void initializeGDCM();

    /// Retorna el valor de PatientID anonimitzat a partir del PatientID original del fitxer. Aquest mètode és consistent de manera que si li passem
    /// una o més vegades el mateix PatientID sempre retornarà el mateix valor com a PatientID anonimitzat.
    QString getAnonimyzedPatientID(const QString &originalPatientID);

    /// Retorna el valor de StudyID anonimitzat a partir del Study Instance UID original del fitxer. Aquest mètode és consistent de manera que si li passem
    /// una o més vegades el mateix study Instance UID sempre retornarà el mateix valor com de Study ID anonimitzat.
    QString getAnonymizedStudyID(const QString &originalStudyInstanceUID);

    /// Retorna el valor d'un Tag en un string, si no troba el tag retorna un string buit
    QString readTagValue(gdcm::File *gdcmFile, gdcm::Tag) const;

private:
    QString m_patientNameAnonymized;
    bool m_replacePatientIDInsteadOfRemove;
    bool m_replaceStudyIDInsteadOfRemove;
    bool m_removePritaveTags;

    QHash<QString, QString> m_hashOriginalPatientIDToAnonimyzedPatientID;
    QHash<QString, QString> m_hashOriginalStudyInstanceUIDToAnonimyzedStudyID;

    gdcm::gdcmAnonymizerStarviewer *m_gdcmAnonymizer;
};

};

#endif
