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

#include <sqlite3.h>
#include <QString>

#include "localdatabasepatientdal.h"
#include "databaseconnection.h"
#include "logging.h"
#include "dicommask.h"

namespace udg {

LocalDatabasePatientDAL::LocalDatabasePatientDAL(DatabaseConnection *dbConnection)
 : LocalDatabaseBaseDAL(dbConnection)
{
}

void LocalDatabasePatientDAL::insert(Patient *newPatient)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), buildSqlInsert(newPatient).toUtf8().constData(), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlInsert(newPatient));
    }
    else
    {
        // El mètode retorna un tipus sqlite3_int64 aquest en funció de l'entorn de compilació equival a un determinat tipus
        // http://www.sqlite.org/c3ref/int64.html __int64 per windows i long long int per la resta, qlonglong de qt
        // http://doc.qt.nokia.com/4.1/qtglobal.html#qlonglong-typedef equival als mateixos tipus pel mateix entorn de compilació per això retornem el
        // ID com un qlonglong.
        newPatient->setDatabaseID(sqlite3_last_insert_rowid(m_dbConnection->getConnection()));
    }
}

void LocalDatabasePatientDAL::update(Patient *patientToUpdate)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), buildSqlUpdate(patientToUpdate).toUtf8().constData(), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlUpdate(patientToUpdate));
    }
}

void LocalDatabasePatientDAL::del(qlonglong patientID)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), buildSqlDelete(patientID).toUtf8().constData(), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlDelete(patientID));
    }
}

QList<Patient*> LocalDatabasePatientDAL::query(const DicomMask &patientMask)
{
    int columns;
    int rows;
    char **reply = NULL;
    char **error = NULL;
    QList<Patient*> patientList;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(),
                                          buildSqlSelect(patientMask).toUtf8().constData(),
                                          &reply, &rows, &columns, error);

    if (getLastError() != SQLITE_OK)
    {
        logError (buildSqlSelect(patientMask));
        return patientList;
    }

    // index = 1 ignorem les capçaleres
    for (int index = 1; index <= rows; index++)
    {
        patientList.append(fillPatient(reply, index, columns));
    }

    sqlite3_free_table(reply);

    return patientList;
}

Patient* LocalDatabasePatientDAL::fillPatient(char **reply, int row, int columns)
{
    Patient *patient = new Patient();

    patient->setDatabaseID(QString(reply[0 + row * columns]).toLongLong());
    patient->setID(reply[1 + row * columns]);
    patient->setFullName(convertToQString(reply[2 + row * columns]));
    patient->setBirthDate(reply[3 + row * columns]);
    patient->setSex(reply[4 + row * columns]);

    return patient;
}

QString LocalDatabasePatientDAL::buildSqlSelect(const DicomMask &patientMaskToSelect)
{
    QString selectSentence = "Select ID, DICOMPatientID, Name, Birthdate, Sex "
                             "From Patient ";

    QString whereSentence;
    if (!patientMaskToSelect.getPatientID().isEmpty())
    {
        whereSentence = QString(" Where DICOMPatientID = '%1' ").arg(DatabaseConnection::formatTextToValidSQLSyntax(patientMaskToSelect.getPatientID()));
    }

    return selectSentence + whereSentence;
}

QString LocalDatabasePatientDAL::buildSqlInsert(Patient *newPatient)
{
    QString insertSentence = QString ("Insert into Patient  (DICOMPatientID, Name, Birthdate, Sex) "
                                                   "values ('%1', '%2', '%3', '%4')")
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newPatient->getID()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newPatient->getFullName()))
                                    .arg(newPatient->getBirthDate().toString("yyyyMMdd"))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newPatient->getSex()));

    return insertSentence;
}

QString LocalDatabasePatientDAL::buildSqlUpdate(Patient *patientToUpdate)
{
    QString updateSentence = QString ("Update Patient Set  DICOMPatientID = '%1', "
                                                           "Name = '%2', "
                                                           "Birthdate = '%3', "
                                                           "Sex = '%4' "
                                                    " Where ID = %5")
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(patientToUpdate->getID()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(patientToUpdate->getFullName()))
                                    .arg(patientToUpdate->getBirthDate().toString("yyyyMMdd"))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(patientToUpdate->getSex()))
                                    .arg(patientToUpdate->getDatabaseID());
    return updateSentence;
}

QString LocalDatabasePatientDAL::buildSqlDelete(qlonglong patientID)
{
    QString deleteSentence = "Delete From Patient ";
    QString whereSentence = QString(" Where ID = %1").arg(patientID);

    return deleteSentence + whereSentence;
}
}
