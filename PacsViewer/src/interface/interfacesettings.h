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

#ifndef UDGINTERFACESETTINGS_H
#define UDGINTERFACESETTINGS_H

#include "defaultsettings.h"

namespace udg {

class InterfaceSettings : public DefaultSettings {
public:
    InterfaceSettings();
    ~InterfaceSettings();

    void init();

    /// Declaració de claus
    static const QString OpenFileLastPath;
    static const QString OpenDirectoryLastPath;
    static const QString OpenFileLastFileExtension;
    static const QString ApplicationMainWindowGeometry;
    // Indicate if we allow to have more than one instance of each extension (true) or only one (false)
    static const QString AllowMultipleInstancesPerExtension;
    // Defines which extension will open by default
    static const QString DefaultExtension;
};

} // end namespace udg

#endif
