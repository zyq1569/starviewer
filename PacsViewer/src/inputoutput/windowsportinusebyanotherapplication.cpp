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

#include "windowsportinusebyanotherapplication.h"
#include "logging.h"

// Windows
#include <windows.h>
/// For TCP / IP Tables
#include <iphlpapi.h>
/// For bstr_t
#include <comdef.h>
/// To get the pid
#include <process.h>

namespace udg {

WindowsPortInUseByAnotherApplication::WindowsPortInUseByAnotherApplication()
{
}

WindowsPortInUseByAnotherApplication::~WindowsPortInUseByAnotherApplication()
{
}

bool WindowsPortInUseByAnotherApplication::isPortInUseByAnotherApplication(int port, bool &error)
{
    int starviewerPId = _getpid();
    bool inUseByAnotherApplication = false;
    error = false;

    PMIB_TCPTABLE_OWNER_MODULE tcpTable = getTCPTables();
    if (!tcpTable)
    {
        error = true;
        return false;
    }
    DWORD numberOfEntries = tcpTable->dwNumEntries;
    DWORD index = 0;
    bool found = false;

    while (!found && index < numberOfEntries)
    {
        MIB_TCPROW_OWNER_MODULE module = tcpTable->table[index++];
        int pId = module.dwOwningPid;
        int localPort = htons((short)module.dwLocalPort);
        /// Only check that the port is the same. It is not checked if it is in use or not,
        /// as it is already looked at before calling this method
        if (localPort == port)
        {
            found = true;
            if (pId == starviewerPId)
            {
                inUseByAnotherApplication = false;
            }
            else
            {
                inUseByAnotherApplication = true;
            }
        }
    }

    free(tcpTable);
    return inUseByAnotherApplication;
}

PMIB_TCPTABLE_OWNER_MODULE WindowsPortInUseByAnotherApplication::getTCPTables()
{
    PVOID tcpTable = NULL;
    DWORD size = 0;
    DWORD result = 0;
    /// First of all, we need to find the size of the TCP table and save it to size,
    ///  once we have it, we can do the malloc and go back
    /// to call the function to retrieve the table correctly.
    /// The problem is because the function does not return the
    /// size carefully (it always returns a different value),
    /// therefore we can look for it with a while and when the
    /// variable pTCPTable is met it will be instantiated correctly.
    bool found = false;
    while (!found)
    {
        //Find the size
        GetExtendedTcpTable(NULL, &size, true, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0);
        // Reserve space for the TCP table now that we know the size
        tcpTable = malloc(size);
        //The TCP table is obtained
        result = GetExtendedTcpTable(tcpTable, &size, true, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0);
        // If it gives us a size error, we try again
        if (result == ERROR_INSUFFICIENT_BUFFER)
        {
            free(tcpTable);
        }
        else if (result == NO_ERROR)
        {
            // If you don't make any mistakes, we can fold
            found = true;
        }
        else
        {
            //  If it gives us a different error, we log it in and return null
            found = true;
            ERROR_LOG(QString("Error getting TCP table: %1").arg(GetLastError()));
            free(tcpTable);
            tcpTable = NULL;
        }
    }

    return (PMIB_TCPTABLE_OWNER_MODULE)tcpTable;
}

} // End udg namespace
