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
class Volume;
/**
Abstract class that acts as a bridge between the starviewer and an extension. From this class they will all have to inherit
the mediators of the different extensions. There must be a mediator for each extension.
The life of an ExtensionMediator is the same as that of its extension. That is why it is tied to him
by means of a kinship.
A Mediator object is instantiated for each Extension object.
"Mother" class of the Extension. She is the only one who understands the Extension and knows where it is, how to treat it ... At the same time
"all mothers are equal." And children don’t know mothers.
\ TODO This whole scheme needs to be reviewed. It is now temporary to be able to separate in directories waiting for "New Design (tm)"
*/
class ExtensionMediator : public QObject {
    //Q_OBJECT
public:
    ExtensionMediator(QObject *parent = 0);

    virtual ~ExtensionMediator();

    /// Method that helps us to, once the extension has been created,
    /// initialize it with the necessary parameters based on its context.
    /// In order to deal with the extension, the first thing you will
    ///  need to do is cast a QWidget in the specific class
    /// of the widget that is passed to us.
    /// @return Returns false in the event that something prevents
    /// it from being initialized, true in all other cases
    virtual bool initializeExtension(QWidget *extension, const ExtensionContext &extensionContext) = 0;

    /// Returns the identifier of the Extension class with which it dialogs.
    /// This identifier is also used to identify the resources (.qrc) of the extension.
    /// For example, if the extension has a getExtensionID (). GetID ()
    /// as "MyExtension" in the resources file it should be put
    /// <qresource prefix = "/ extensions / MyExtension">
    /// and to access it: QIcon * icon = new QIcon (": / extensions / MyExtension / images / icon.png");
    virtual DisplayableID getExtensionID() const = 0;

    /// Orders the extension to view newly loaded studies from the current patient.
    virtual void viewNewStudiesFromSamePatient(QWidget *extension, const QString &newStudyUID);


    //---20200919---add
    virtual void executionCommand(QWidget *extension, Volume* volume, int command=0) = 0;
};

}

#endif
