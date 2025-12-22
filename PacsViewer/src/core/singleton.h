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

#ifndef UDGSINGLETON_H
#define UDGSINGLETON_H

#include <QReadWriteLock>
#include <QThread>
#include <QWriteLocker>
#include <QCoreApplication>

namespace udg {

/**
Class that implements the singleton pattern. The implementation is not thread-safe, so it should not be used from different
threads. Any class that wants to convert to Singleton will need to have a parameterless constructor.
The way to use it would be as follows:
\ code
typedef Singleton <TestingClass> TestingClassSingleton;
...
TestingClass * instance = TestingClassSingleton :: instance ();
instance-> method ();
...
\ endcode
As you can see, it is highly recommended to use a typedef so that it does not decrease the readability of the code. This way
using singleton is dangerous if there is no other instance of the TestingClass class. This way
what is being done is to ensure that of the TestingClassSingleton type we will have a globally accessible instance. But nothing stops us
create separate TestingClass classes.
Another way to declare a class as a singleton would be as follows:
\ code
class OnlyOne: public Singleton <OnlyOne>
{
    //..rest of the code
    protected:
        friend Singleton <OnlyOne>;
        OnlyOne ();
        ~ OnlyOne ();
};
\ endcode
In this way we are ensuring that of the OnlyOne class, throughout the life of the program, there will be only one and will be the
same for the whole execution.
Singleton <OnlyOne> must be declared a friend because otherwise we would be forced to declare a builder and destroyer
audiences, thus breaking the philosophy of a Singleton.
\ all Make it thread-safe.
*/
template <typename T>
class Singleton {

public:
    /// It serves us to access the only instance of the T class
    static T* instance()
    {
        static T theSingleInstance;
        return &theSingleInstance;
    }

protected:
    //  It is not implemented
    Singleton(){};
    //  It is not implemented
    Singleton(const Singleton& ){};
    // It is not implemented
    Singleton &operator=(const Singleton &){};
};

/**
Class that implements the Singleton pattern but at the pointer level. It is used in the same way as Singleton but with
 the peculiarity that the singleton is made at the level of a pointer instead of an object.
 This singleton should only be used when there are problems that prevent the use of the above. Some of these problems
 they would be problems in the order of destruction of static objects (as in the case of the DcmDatasetCache because of dcmtk).
 The problem with this implementation is that the T class must be a Qt class and derived from QObject.
 The singleton will be destroyed when the main application is destroyed, that is, when the QCoreApplication :: aboutToQuit signal is
 released.
 This implementation is thread-safe.
*/
template<typename T>
class SingletonPointer {

public:
    static T* instance()
    {
        if (m_theSinglePointer == NULL)
        {
            QWriteLocker locker(&m_pointerLock);
            //We double check to avoid unnecessary blockages
            if (m_theSinglePointer == NULL)
            {
                m_theSinglePointer = new T();
                m_theSinglePointer->connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(deleteLater()));
            }
        }
        return m_theSinglePointer;
    }

protected:
    // It is not implemented
    SingletonPointer(){};
    // It is not implemented
    SingletonPointer(const SingletonPointer &){};
    // It is not implemented
    SingletonPointer &operator=(const SingletonPointer &){};

private:
    static QReadWriteLock m_pointerLock;
    static T* m_theSinglePointer;
};

template<typename T> T* SingletonPointer<T>::m_theSinglePointer = NULL;
template<typename T> QReadWriteLock SingletonPointer<T>::m_pointerLock;
}

#endif
