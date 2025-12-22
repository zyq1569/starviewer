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

#ifndef UDGEXTENSIONFACTORYREGISTER_H
#define UDGEXTENSIONFACTORYREGISTER_H

#include <QWidget>
#include <QString>

#include "genericsingletonfactoryregister.h"
#include "extensionfactory.h"
#include "logging.h"

namespace udg {

/**
Class that allows us to register an Extension in the ExtensionFactory. In order to be able to register an extension we have to declare
an object of type ExtensionFactoryRegister.
Example:
@code
ExtensionFactoryRegister <ExtensionName> registerAs ("Extension Identifier");
@endcode
With this simple line of code we have already registered our extension in the factory.
**/
template <class FactoryType>
class ExtensionFactoryRegister : public GenericSingletonFactoryRegister<QWidget, FactoryType, QString, ExtensionFactory, QWidget> {
public:
    /// Methods
    ExtensionFactoryRegister(const QString &identifier)
        :GenericSingletonFactoryRegister<QWidget, FactoryType, QString, ExtensionFactory, QWidget>(identifier)
    {

    }
};

}

#endif
