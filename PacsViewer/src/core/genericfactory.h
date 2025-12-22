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

#ifndef UDGGENERICFACTORY_H
#define UDGGENERICFACTORY_H

#include <QMap>
#include <QObject>
#include <QList>

namespace udg {

/**
Class that serves as the basis for implementing the Factory pattern.
The use of this class is internal to the platform and should not be used unless it is being developed by the core.
Its utility, along with GenericFactoryRegister, is to provide a generic implementation of the Factory pattern.

This class is useful when generating classes in the same class hierarchy. For example, if we had
the Vehicle class as an abstract class and the Truck, Car, Motorcycle classes as inheriting classes from Vehicle,
we could use the GenericFactory to instantiate objects such as Truck, Car or Motorcycle but that were returned
as Vehicles.

The current implementation assumes that all objects to be created will inherit from QObject and therefore its constructor.
has a parameter that is the relative of this one.

Usage example:
@code
// We create a Vehicle Factory that will be identified by a string. Vehicles and their children are subclasses of QObject
typedef GenericFactory <Vehicle, std :: string> VehicleFactory;

VehicleFactory vehicles;

// .. Here we should register the different classes with the Factory.
// See GenericFactoryRegister for a simple way

Vehicle * vehicle = vehicles-> create ("car");
Vehicle * vehicle2 = vehicles-> create ("motorcycle");

std :: cout << "Total wheels =" << vehicle-> getNumeroRodes () + vehicle2-> getNumeroRodes () << std :: endl;

// .. This would print "Total Wheels = 6" assuming car returns 4 and motorcycle 2.
@endcode
Although this is not done in the example, it would be necessary to look at whether the returned object is NULL or not.

This class will be used, most of the time, with a singleton to facilitate registration and access but it does not have to.

@TODO If this class is used in conjunction with a singleton, only one object of each type can be used.
@TODO If necessary, the most generic implementation should be made to allow 0 to n parameters in the constructor and not
do not force created objects to inherit from QObject.
@see GenericSingletonFactoryRegister
@see ExtensionFactory
*/
template <class BaseClass, typename ClassIdentifier, class ParentType = QObject>
class GenericFactory {
    typedef BaseClass* (*BaseClassCreateFunction)(ParentType*);
    typedef QMap<ClassIdentifier, BaseClassCreateFunction> FunctionRegistry;

public:
    ///Class builder
    GenericFactory(){}

    /// Method used to register a class creation function.
    /// This method is the one that must be used in order to be able to register a certain class in the factory.
    /// @param className Name of the class to register.
    /// @param function Function of type BaseClassCreateFunction that returns an object of class className.
    /// This method is automatically given by the GenericFactoryRegister class.
    void registerCreateFunction(const ClassIdentifier &className, BaseClassCreateFunction function)
    {
        m_registry.insert(className, function);
    }

    /// Method that creates the object that is defined by the identifier. It returns the BaseClass type.
    /// @param className Name of the class we want the object to do
    /// @param parent QObject parent of the object to be created.
    /// @return Returns the converted object to the base class BaseClass.
    /// In case it does not find the object or error it will return NULL.
    BaseClass* create(const ClassIdentifier &className, ParentType *parent = 0) const
    {
        BaseClass *theObject = NULL;

        typename FunctionRegistry::const_iterator regEntry = m_registry.find(className);

        if (regEntry != m_registry.end())
        {
            try
            {
                theObject = regEntry.value()(parent);
            }
            catch (const std::bad_alloc&)
            {
                theObject = NULL;
            }
        }
        return theObject;
    }

    QList<ClassIdentifier> getFactoryIdentifiersList() const
    {
        return m_registry.keys();
    }

private:
    FunctionRegistry m_registry;
};

}

#endif
