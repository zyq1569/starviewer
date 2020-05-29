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

#ifndef UDGLOCALDATABASEPATIENTDAL_H
#define UDGLOCALDATABASEPATIENTDAL_H

#include <QList>

#include "localdatabasebasedal.h"
#include "patient.h"

namespace udg {

class DicomMask;

/**
    Aquesta classe s'encarrega de dur a terme les operacions relacionades amb l'objecte estudi de la cache de l'aplicació.
  */
class LocalDatabasePatientDAL : public LocalDatabaseBaseDAL {
public:
    LocalDatabasePatientDAL(DatabaseConnection *dbConnection);

    /// Insereix el nou pacient, i emplena el camp DatabaseID de Patient amb el ID de Pacient de la BD.
    void insert(Patient *newPatient);

    /// Updata el pacient
    void update(Patient *patientToUpdate);

    /// Esborra els estudis pacients que compleixen amb els criteris de la màscara, només té en compte el Patient Id
    void del(qlonglong patientID);

    /// Cerca els pacients que compleixen amb els criteris de la màscara de cerca, només té en compte el Patient Id
    QList<Patient*> query(const DicomMask &patientMaskToQuery);

private:
    /// Construeix la sentència sql per inserir el nou pacient
    QString buildSqlInsert(Patient *newPatient);

    /// Construeix la sentència updatar el pacient
    QString buildSqlUpdate(Patient *patientToUpdate);

    /// Construeix la setència per fer select de pacients a partir de la màscara, només té en compte el PatientID
    QString buildSqlSelect(const DicomMask &patientMaskToSelect);

    /// Construeix la setència per esborrar pacients a partir de la màscara, només té en compte el Patient Id
    QString buildSqlDelete(qlonglong patientID);

    /// Emplena un l'objecte series de la fila passada per paràmetre
    Patient* fillPatient(char **reply, int row, int columns);
};
}

#endif
