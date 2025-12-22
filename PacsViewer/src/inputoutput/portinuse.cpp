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

#include "portinuse.h"
#include "portinusebyanotherapplication.h"

#include <QTcpServer>

#include "logging.h"

namespace udg {

PortInUse::PortInUse()
{
    m_status = PortInUse::PortUnknownStatus;
    m_errorString = QObject::tr("No port checked yet");
}

PortInUse::~PortInUse()
{
}

bool PortInUse::isPortInUse(int port)
{
    bool portInUse;
    QAbstractSocket::SocketError serverError;
    QString errorString;

    portInUse = !isPortAvailable(port, serverError, errorString);

    if (!portInUse)
    {
        m_status = PortInUse::PortIsAvailable;
    }
    else if (serverError == QAbstractSocket::AddressInUseError)
    {
        m_status = PortInUse::PortIsInUse;
    }
    else
    {
        /// AddressInUseError should not be given a different error, anyway for security logging
        ERROR_LOG("No s'ha pogut comprovat correctament si el port " + QString().setNum(port) + " està en ús, per error: " + errorString);
        m_errorString = errorString;
        m_status = PortInUse::PortCheckError;
    }

    m_lastPortChecked = port;
    return portInUse;
}

PortInUse::PortInUseOwner PortInUse::getOwner()
{
    PortInUse::PortInUseOwner owner = PortInUse::PortUsedByUnknown;

    // In case the port is in use, you need to see if it has been opened by Starviewer or another application
    if (m_status == PortInUse::PortIsInUse)
    {
        //Instantiate an object of the class according to the operating system
        PortInUseByAnotherApplication *portInUse = createPortInUseByAnotherApplication();
        bool error;
        bool inUse = portInUse->isPortInUseByAnotherApplication(m_lastPortChecked, error);
        if (!error)
        {
            if (inUse)
            {
                // Open port for another application
                owner = PortInUse::PortUsedByOther;
            }
            else
            {
                // port opened by Starviewer
                owner = PortInUse::PortUsedByStarviewer;
            }
        }

        delete portInUse;
    }

    return owner;
}

bool PortInUse::isPortAvailable(int port, QAbstractSocket::SocketError &serverError, QString &errorString)
{
    QTcpServer tcpServer;
    bool result;

    /// Result will be true if the port is free, so the opposite must be returned
    result = tcpServer.listen(QHostAddress::Any, port);
    serverError = tcpServer.serverError();
    errorString = tcpServer.errorString();

    tcpServer.close();

    return result;
}

PortInUse::PortInUseStatus PortInUse::getStatus()
{
    return m_status;
}

QString PortInUse::getErrorString()
{
    return m_errorString;
}

udg::PortInUseByAnotherApplication* PortInUse::createPortInUseByAnotherApplication()
{
    return PortInUseByAnotherApplication::newInstance();
}

} // End udg namespace
