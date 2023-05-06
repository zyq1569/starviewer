/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGEDEMASEGMENTATIONEXTENSIONMEDIATOR_H
#define UDGEDEMASEGMENTATIONEXTENSIONMEDIATOR_H

#include "extensionmediator.h"

#include "installextension.h"
#include "qedemasegmentationextension.h"

namespace udg {

/**
    @author Grup de Gràfics de Girona (GGG) <vismed@ima.udg.es>
*/
//水肿分割
class EdemaSegmentationExtensionMediator : public ExtensionMediator
{
    Q_OBJECT
public:
    EdemaSegmentationExtensionMediator(QObject *parent = 0);

    ~EdemaSegmentationExtensionMediator();

    virtual bool initializeExtension(QWidget* extension, const ExtensionContext &extensionContext);

    virtual DisplayableID getExtensionID() const;

    //---20200919---add
    virtual void executionCommand(QWidget *extension,Volume* volume, int command=0)
    {

    }
};

static InstallExtension<QEdemaSegmentationExtension, EdemaSegmentationExtensionMediator> registerEdemaSegmentationExtension;

}

#endif
