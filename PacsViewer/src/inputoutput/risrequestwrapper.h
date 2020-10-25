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

#ifndef UDGRISREQUESTWRAPPER_H
#define UDGRISREQUESTWRAPPER_H

#include <QString>

namespace udg {

/**
This class is responsible for integrating with the SAP of the ICS for downloading studies, to do so
we take advantage of the part done in the Starviewer of integration with the RIS PIER.
This class sends via TCP / IP a request to the Starviewer Local to download a study in the same XML format as
makes the RIS PIER, thus taking advantage of the implemented RIS PIER integration part of the Starviewer we have done
integration with SAP.
*/
class RISRequestWrapper {

public:
    /// Send via Tcp / IP a request to download the study
    /// with the accession number passed by parameter to the Starviewer of the local machine
    void sendRequestToLocalStarviewer(QString accessionNumber);

private:
    /// Giving the accession number returns an xml that the Starviewer
    /// once received it is able to automatically download the studio with accession
    /// number passed by parameter
    QString getXmlPier(QString accessionNumber);

    /// Print by screen and log in error when connecting
    void errorConnecting(int port, QString errorDescription);

    /// Print by screen and log in error when sending the request for the Tcp / Ip connection
    void errorWriting(QString errorDescription);

    /// Print by screen and log in error when closing connection
    void errorClosing(QString errorDescription);
};

}

#endif
