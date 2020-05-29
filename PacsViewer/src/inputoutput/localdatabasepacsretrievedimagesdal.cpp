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

#include "localdatabasepacsretrievedimagesdal.h"

#include <sqlite3.h>
#include <QString>
#include "databaseconnection.h"
#include "pacsdevice.h"
#include "logging.h"

namespace udg {

LocalDatabasePACSRetrievedImagesDAL::LocalDatabasePACSRetrievedImagesDAL(DatabaseConnection *dbConnect):LocalDatabaseBaseDAL(dbConnect)
{
}

qlonglong LocalDatabasePACSRetrievedImagesDAL::insert(const PacsDevice &pacsDevice)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), buildSqlInsert(pacsDevice).toUtf8().constData(), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlInsert(pacsDevice));
        return -1;
    }
    else
    {
        // El mètode retorna un tipus sqlite3_int64 aquest en funció de l'entorn de compilació equival a un determinat tipus
        // http://www.sqlite.org/c3ref/int64.html __int64 per windows i long long int per la resta, qlonglong de qt
        // http://doc.qt.nokia.com/4.1/qtglobal.html#qlonglong-typedef equival als mateixos tipus pel mateix entorn de compilació per això retornem el
        // ID com un qlonglong.
        return sqlite3_last_insert_rowid(m_dbConnection->getConnection());
    }
}

PacsDevice LocalDatabasePACSRetrievedImagesDAL::query(const qlonglong &IDPacsInDatabase)
{
    return query(buildSqlSelect(IDPacsInDatabase));
}

PacsDevice LocalDatabasePACSRetrievedImagesDAL::query(const QString AETitle, const QString address, int queryPort)
{
    return query(buildSqlSelect(AETitle, address, queryPort));
}

PacsDevice LocalDatabasePACSRetrievedImagesDAL::query(const QString &sqlQuerySentence)
{
    int columns;
    int rows;
    char **reply = NULL;
    char **error = NULL;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(), sqlQuerySentence.toUtf8().constData(), &reply, &rows, &columns, error);

    if (getLastError() != SQLITE_OK)
    {
        logError (sqlQuerySentence);
        return PacsDevice();
    }

    PacsDevice pacsDevice;

    if (rows >= 1)
    {
        // la primera columna és la capçalera
        pacsDevice = fillPACSDevice(reply, 1, columns);
    }

    sqlite3_free_table(reply);

    return pacsDevice;
}

PacsDevice LocalDatabasePACSRetrievedImagesDAL::fillPACSDevice(char **reply, int row, int columns)
{
    PacsDevice pacsDevice;

    pacsDevice.setID(reply[0 + row * columns]);
    pacsDevice.setAETitle(reply[1 + row * columns]);
    pacsDevice.setAddress(reply[2 + row * columns]);
    pacsDevice.setQueryRetrieveServicePort(QString(reply[3 + row * columns]).toInt());

    return pacsDevice;
}

QString LocalDatabasePACSRetrievedImagesDAL::buildSqlInsert(const PacsDevice &pacsDevice)
{
    QString insertSentence = QString ("Insert into PACSRetrievedImages  (AETitle, Address, QueryPort) "
                                                   "values ('%1', '%2', %3)")
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(pacsDevice.getAETitle()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(pacsDevice.getAddress()))
                                    .arg(pacsDevice.getQueryRetrieveServicePort());
    return insertSentence;
}

QString LocalDatabasePACSRetrievedImagesDAL::buildSqlSelect()
{
    QString selectSentence = "Select ID, AETitle, Address, QueryPort "
                             "From PACSRetrievedImages ";

    return selectSentence;
}

QString LocalDatabasePACSRetrievedImagesDAL::buildSqlSelect(const qlonglong &IDPACSInDatabase)
{
    QString whereSentence = QString(" Where ID = %1 ").arg(IDPACSInDatabase);

    return buildSqlSelect() + whereSentence;

}

QString LocalDatabasePACSRetrievedImagesDAL::buildSqlSelect(const QString AETitle, const QString address, int queryPort)
{
    QString whereSentence = QString(" Where AETitle = '%1' and "
                                   "Address = '%2' and "
                                   "QueryPort = %3")
            .arg(DatabaseConnection::formatTextToValidSQLSyntax(AETitle))
            .arg(DatabaseConnection::formatTextToValidSQLSyntax(address))
            .arg(queryPort);

    return buildSqlSelect() + whereSentence;
}
}
