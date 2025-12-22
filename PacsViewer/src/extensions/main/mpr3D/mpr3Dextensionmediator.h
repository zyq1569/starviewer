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

#ifndef MPR3DExtensionMediator_H
#define MPR3DExtensionMediator_H

#include "extensionmediator.h"

#include "installextension.h"
#include "qmpr3Dextension.h"

namespace udg {

class MPR3DExtensionMediator : public ExtensionMediator {
//Q_OBJECT
public:
    MPR3DExtensionMediator(QObject *parent = 0);

    ~MPR3DExtensionMediator();

    virtual bool initializeExtension(QWidget *extension, const ExtensionContext &extensionContext);

    virtual DisplayableID getExtensionID() const;

    //---20200919---add
	virtual void executionCommand(QWidget *extension, Volume* volume, void *data = NULL, int command = 0);

};

static InstallExtension<QMPR3DExtension, MPR3DExtensionMediator> registerMPR3DExtension;

} // End udg namespace

#endif
