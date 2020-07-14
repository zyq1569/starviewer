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

#ifndef UDGWINDOWSFIREWALLACCESS_H
#define UDGWINDOWSFIREWALLACCESS_H

#include "firewallaccess.h"
#include <windows.h>
#include <netfw.h>
#include <QString>

namespace udg {

/**
    Classe que comprova si starviewer té accés a través del firewall de windows.
  */
class WindowsFirewallAccess : public FirewallAccess {
public:
    WindowsFirewallAccess();
    ~WindowsFirewallAccess();

    bool doesStarviewerHaveAccesThroughFirewall();

protected:
    /// Initialize the Windows Firewall so you can use it.
    /// @param firewallProfile is output and contains the firewall profile.
    virtual HRESULT initializeWindowsFirewall(INetFwProfile **firewallProfile);

    /// Check if the firewall is turned on.
    virtual HRESULT windowsFirewallIsOn(INetFwProfile *firewallProfile, bool *firewallOn);

    ///Check if the firewall allows exceptions.
    virtual HRESULT doesWindowsFirewallAllowExceptions(INetFwProfile *firewallProfile, bool *exceptionsAllowed);

    /// Given a firewall profile and path to an application executable, check if it is in the firewall exception list.
    /// The result you return is whether it went well or not.
    /// @param firewallApplicationEnabled is output and saves whether the application is in the exception list or not.
    virtual HRESULT isApplicationEnabledAtFirewall(INetFwProfile *firewallProfile, BSTR firewallProcessImageFileName, bool *firewallApplicationEnabled);

    /// Returns the path from where starviewer is running
    virtual BSTR getStarviewerExecutablePath();

    /// Transforms a QString into a 'Basic String' text string in Visual Basic. It is the user's responsibility to release the string
    /// when you no longer need it, using the SysFreeString function.
    virtual BSTR fromQStringToBSTR(const QString &string);

    /// Initialize the Windows Firewall library.
    /// Return true if everything went well.
    virtual HRESULT initializeWindowsFirewallLibrary(LPVOID pvReserved, DWORD dwCoInit);

    ///Finish the windows firewall library.
    virtual void UninitializeWindowsFirewallLibrary();

    /// Clear the firewall profile if it is not null
    virtual void CleanupWindowsFirewall(INetFwProfile *firewallProfile);

};

} // End namespace udg

#endif
