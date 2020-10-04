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

#ifndef UDGPORTINUSE_H
#define UDGPORTINUSE_H

#include <QString>
#include <QAbstractSocket>

namespace udg {

class PortInUseByAnotherApplication;

/**
    Class that is responsible for checking if a port is in use.
*/
class PortInUse {
public:
    enum PortInUseStatus { PortIsAvailable, PortIsInUse, PortCheckError, PortUnknownStatus };
    enum PortInUseOwner { PortUsedByUnknown, PortUsedByStarviewer, PortUsedByOther };

    PortInUse();
    virtual ~PortInUse();

    /// Indicates whether the port passed by parameter is in use (true) or free (false)
    bool isPortInUse(int port);

    /// Indicates whether the port passed by parameter is in use by a different starviewer application
    PortInUse::PortInUseOwner getOwner();

    /// Returns the status of the last port that was checked
    PortInUse::PortInUseStatus getStatus();

    /// Returns the string corresponding to the error. Its value will only be valid when m_status is worth PortCheckError.
    QString getErrorString();

protected:
    /// Returns if the port passed by parameter is free (true) or in use (false)
         /// @param serverError: Indicates server error
         /// @param errorString: description of the error.
    virtual bool isPortAvailable(int port, QAbstractSocket::SocketError &serverError, QString &errorString);
    ///Method for applying testing, creates a portInUseByAnotherApplication object
    virtual PortInUseByAnotherApplication* createPortInUseByAnotherApplication();

protected:
    ///Last port checked, when we do a getOwner, it will be made from that port
    int m_lastPortChecked;
    /// State of the port
    PortInUse::PortInUseStatus m_status;
    /// String with the description of the error in case any has occurred
    QString m_errorString;

};

}  // end namespace udg

#endif
