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

#include "databaseconnection.h"

#include <sqlite3.h>
// Per les traduccions: tr()
#include <QObject>
#include <QSemaphore>
#include <QDir>
#include <QString>
#include "localdatabasemanager.h"
#include "logging.h"

namespace udg {

DatabaseConnection::DatabaseConnection()
    : m_databaseConnection(NULL)
{
    m_databasePath = LocalDatabaseManager::getDatabaseFilePath();
    m_transactionLock = new QSemaphore(1);
}

void DatabaseConnection::setDatabasePath(const QString &path)
{
    m_databasePath = path;
}

void DatabaseConnection::open()
{
    // Cal obrir amb UTF8 perquè l'sqlite3 nomes treballa amb aquesta codificació i sinó no troba la base de dades.
    sqlite3_open(qPrintable(QDir::toNativeSeparators(QString(m_databasePath.toUtf8()))), &m_databaseConnection);
    // En el moment que es fa el commit de les dades inserides o updates a la base de dades, sqlite bloqueja tota la base
    // de dades, per tant no es pot fer cap consulta. Indicant el busy_timeout a 10000 ms el que fem, és que si tenim una
    // setència contra sqlite que es troba la bd o una taula bloquejada, va fent intents cada x temps per mirar si continua
    // bloqueja fins a 15000ms una vegada passat aquest temps dona errora de taula o base de dades bloquejada

    sqlite3_busy_timeout(m_databaseConnection, 15000);
}

void DatabaseConnection::beginTransaction()
{
    if (!isConnected())
    {
        open();
    }

    m_transactionLock->acquire();
    sqlite3_exec(m_databaseConnection, "BEGIN IMMEDIATE", 0, 0, 0);
}

void DatabaseConnection::commitTransaction()
{
    sqlite3_exec(m_databaseConnection, "END", 0, 0, 0);
    m_transactionLock->release();
}

void DatabaseConnection::rollbackTransaction()
{
    sqlite3_exec(m_databaseConnection, "ROLLBACK", 0, 0, 0);
    m_transactionLock->release();
    INFO_LOG("S'ha cancel.lat transaccio de la BD");
}

QString DatabaseConnection::formatTextToValidSQLSyntax(QString string)
{
    return string.isNull() ? "" : string.replace("'", "''");
}

QString DatabaseConnection::formatTextToValidSQLSyntax(QChar qchar)
{
    // Retornem un QString perquè si retornem QChar('') si qchar és null al converti-lo a QString(QChar('')) el QString s'inicialitza incorrectament agafant
    // com a valor un caràcter estrany en comptes de QString("")
    return qchar.isNull() ? "" : QString(qchar);
}

sqlite3* DatabaseConnection::getConnection()
{
    if (!isConnected())
    {
        open();
    }

    return m_databaseConnection;
}

bool DatabaseConnection::isConnected()
{
    return m_databaseConnection != NULL;
}

void DatabaseConnection::close()
{
    if (isConnected())
    {
        sqlite3_close(m_databaseConnection);
        m_databaseConnection = NULL;
    }
}

QString DatabaseConnection::getLastErrorMessage()
{
    return sqlite3_errmsg(m_databaseConnection);
}

int DatabaseConnection::getLastErrorCode()
{
    return sqlite3_errcode(m_databaseConnection);
}

DatabaseConnection::~DatabaseConnection()
{
    close();
}

};
