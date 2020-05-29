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

#ifndef EXTENSIONMEDIATOR_H
#define EXTENSIONMEDIATOR_H

#include <QObject>
#include "displayableid.h"
#include "identifier.h"

namespace udg {

class ExtensionContext;

/**
    Classe abstracta que fa de pont entre l'starviewer i una extensió. D'aquesta classe hauran d'heredar tots
    els mediators de les diferents extensions. Hi ha d'haver un mediator per cada extensió.
    La vida d'un ExtensionMediator és la mateixa que la de la seva extensió. Per això queda lligada a ell
    mitjançants un parentiu.
    S'instancia un objecte Mediator per cada objecte Extension.
    Classe "mare" de l'Extensió. És l'única que enten a l'Extensió i sap on es troba, com tractar-la... Alhora
    "totes les mares són iguals". I els fills no coneixen a les mares.

    \TODO Cal revisar tot aquest esquema. Ara és temporal per poder separar en directoris a l'espera del "Nou Disseny(tm)"
  */
class ExtensionMediator : public QObject {
Q_OBJECT
public:
    ExtensionMediator(QObject *parent = 0);

    virtual ~ExtensionMediator();

    /// Mètode que ens serveix per, un cop creada l'extensió, inicialitzar-la amb els paràmetres necessàris a partir del seu contexte.
    /// Per poder tractar l'extensió, el primer que caldrà serà realitzar un cast de QWidget a la classe concreta
    /// del widget que se'ns passa.
    /// @return Retorna false en el supòsit que hi hagi alguna cosa que impedeixi inicialitzar-la, true en la resta de casos
    virtual bool initializeExtension(QWidget *extension, const ExtensionContext &extensionContext) = 0;

    /// Retorna l'identificador de la classe Extension amb qui dialoga.
    /// Aquest identificador també serveix per identificar els resources (.qrc) de l'extensió.
    /// Per exemple, si l'extensió té un getExtensionID().getID() com a "MyExtension" al fitxer resources s'hi haurà de posar
    ///  <qresource prefix="/extensions/MyExtension" >
    /// i per accedir-hi: QIcon *icon = new QIcon(":/extensions/MyExtension/images/icon.png");
    virtual DisplayableID getExtensionID() const = 0;

    /// Orders the extension to view newly loaded studies from the current patient.
    virtual void viewNewStudiesFromSamePatient(QWidget *extension, const QString &newStudyUID);

};

}

#endif
