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

#ifndef UDGPORTINUSEBYANOTHERAPPLICATION_H
#define UDGPORTINUSEBYANOTHERAPPLICATION_H

namespace udg {

/**
    Class that is responsible for checking if a port is in use by an application other than starviewer.
*/
class PortInUseByAnotherApplication {
public:
    virtual ~PortInUseByAnotherApplication();

    /// Creates a new instance of one of the subclasses that the interface implements
    static PortInUseByAnotherApplication* newInstance();

    ///Check if the port is in use by a non-Starviewer application
    virtual bool isPortInUseByAnotherApplication(int port, bool &error);

protected:
    // Private constructor to force to use the newInstance method
    PortInUseByAnotherApplication();
};

}  // end namespace udg

#endif
