/**************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGRECTUMSEGMENTATIONEXTENSIONMEDIATOR_H
#define UDGRECTUMSEGMENTATIONEXTENSIONMEDIATOR_H

#include "extensionmediator.h"

#include "installextension.h"
#include "qrectumsegmentationextension.h"

namespace udg {

/**
    @author Girona Graphics Group (GGG) <vismed@ima.udg.es>
*/
class RectumSegmentationExtensionMediator : public ExtensionMediator
{
    //Q_OBJECT
public:
    RectumSegmentationExtensionMediator(QObject *parent = 0);

    ~RectumSegmentationExtensionMediator();

    virtual bool initializeExtension(QWidget* extension, const ExtensionContext &extensionContext);

    virtual DisplayableID getExtensionID()const;

    //---20200919---add
    virtual void executionCommand(QWidget *extension, Volume* volume, void *data = NULL, int command = 0)
    {

    }
};

static InstallExtension<QRectumSegmentationExtension, RectumSegmentationExtensionMediator> registerRectumSegmentationExtension;

}

#endif
