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

#ifndef UDGSTUDYLAYOUTCONFIGSETTINGSMANAGER_H
#define UDGSTUDYLAYOUTCONFIGSETTINGSMANAGER_H

#include <QList>

namespace udg {

class StudyLayoutConfig;

/** 
Class that is responsible for obtaining, adding, updating, and deleting StudyLayoutConfig stored in settings.
  */
class StudyLayoutConfigSettingsManager {
public:
    StudyLayoutConfigSettingsManager();
    ~StudyLayoutConfigSettingsManager();

    /// Returns the list of configurations saved in settings
    QList <StudyLayoutConfig> getConfigList () const;

    /// Methods for adding a new configuration, updating it, or deleting it. Returns true if the operation is performed successfully, false otherwise.

    /// Add a new configuration if none exists for that mode
    bool addItem (const StudyLayoutConfig & config);

    /// Update the configuration in case there is one configured for that mode
    bool updateItem (const StudyLayoutConfig & config);

    /// Delete the configuration in case there is one configured for that mode
    bool deleteItem (const StudyLayoutConfig & config);
};

}

#endif
