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
    Classe que ens permet registrar una Extension en el ExtensionFactory. Per tal de poder registrar una extensió hem de declarar
    un objecte del tipus ExtensionFactoryRegister.
    Exemple:
    @code
    ExtensionFactoryRegister<ExtensionName> registerAs("Extension Identifier");
    @endcode
    Amb aquesta simple línia de codi ja tenim registrada la nostra extensió en el factory.
  */
template <class FactoryType>
class ExtensionFactoryRegister : public GenericSingletonFactoryRegister<QWidget, FactoryType, QString, ExtensionFactory, QWidget> {
public:
    /// Mètode
    ExtensionFactoryRegister(const QString &identifier)
        :GenericSingletonFactoryRegister<QWidget, FactoryType, QString, ExtensionFactory, QWidget>(identifier)
    {
        DEBUG_LOG("ExtensionFactoryRegister" + identifier);
    }
};

}

#endif
