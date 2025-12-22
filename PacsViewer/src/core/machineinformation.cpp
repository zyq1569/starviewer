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

#include "machineinformation.h"

#include <QNetworkInterface>
#include <QProcessEnvironment>

namespace udg {

MachineInformation::MachineInformation()
{
}

QString MachineInformation::getMACAddress()
{
    QString macAdress;
    ///First of all look for if there is local area network interface
    bool found = false;
    int index = 0;
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    while (!found && index < allInterfaces.count())
    {
        QNetworkInterface inter = allInterfaces[index++];

        bool isMainInterface = false;
#ifdef WIN32
        isMainInterface = inter.humanReadableName().contains("local");
#else
        isMainInterface = inter.humanReadableName().contains("eth");
#endif
        QNetworkInterface::InterfaceFlags f = inter.flags();
        bool flagsOk = f.testFlag(QNetworkInterface::IsUp) && f.testFlag(QNetworkInterface::IsRunning) && !f.testFlag(QNetworkInterface::IsLoopBack);

        if (isMainInterface && inter.isValid() && flagsOk)
        {
            macAdress += inter.hardwareAddress();
            found = true;
        }
    }

    ///I give priority to the local area interface and then look for the first valid interface
    if (macAdress == "")
    {
        found = false;
        index = 0;
        while (!found && index < allInterfaces.count())
        {
            QNetworkInterface interface = allInterfaces[index++];

            QNetworkInterface::InterfaceFlags flags = interface.flags();
            bool flagsOk = flags.testFlag(QNetworkInterface::IsUp) && flags.testFlag(QNetworkInterface::IsRunning) &&
                    !flags.testFlag(QNetworkInterface::IsLoopBack);

            /// Just in case the bluetooth is on and working, make sure it doesn't pick it up
            /// We rarely find a network connection that goes through a bluetooth device
            if (interface.isValid() && flagsOk && !interface.humanReadableName().toLower().contains("bluetooth"))
            {
                macAdress += interface.hardwareAddress();
                found = true;
            }
        }
    }

    return macAdress;
}

QString MachineInformation::getDomain()
{
    QString domain;
    //In case we are in windows, we look for the groupID
#ifdef WIN32
    domain = getSystemEnvironment().value(QString("USERDOMAIN"), QString(""));
#endif
    return domain;
}

QProcessEnvironment MachineInformation::getSystemEnvironment()
{
    return QProcessEnvironment::systemEnvironment();
}

} // namespace udg
