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

#ifndef UDGGENERICSINGLETONFACTORYREGISTER_H
#define UDGGENERICSINGLETONFACTORYREGISTER_H

#include <QObject>

namespace udg {

/**
     Class that makes it easier for us to register a class in a GenericFactory that is singleton. This template makes our job easier
     to register a class in a GenericFactory. The only condition is that the GenericFactory be Singleton. Instead of creating
     a macro has been chosen to do this with a template as this saves us the trouble of using macros.
     The way to register a class is to declare a variable of type GenericSingletonFactoryRegister.

     However, this class should be inherited and only use its inheritances. An example is ExtensionFactoryRegister.
     Therefore, this class is only reserved for when you want to implement a new factory and not to use it directly in the
     factory's register.
*/
template <class AncestorType, class BaseClass, typename ClassIdentifier, class SingletonRegistry, class ParentType = QObject>
class GenericSingletonFactoryRegister {
public:
    /// Constructor method that helps us to register a class with a specific id in a Factory.
    /// @param id Identifier of the class to be registered
    /// @return
    GenericSingletonFactoryRegister(const ClassIdentifier &id)
    {
        SingletonRegistry::instance()->registerCreateFunction(id, createInstance);
    }

    /// Auxiliary method and should not be used directly
    static AncestorType* createInstance(ParentType *parent)
    {
        return dynamic_cast<AncestorType*>(new BaseClass(parent));
    }
};

}

#endif
