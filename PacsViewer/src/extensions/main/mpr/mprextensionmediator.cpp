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

#include "mprextensionmediator.h"

#include "extensioncontext.h"

// QT
#include <QMessageBox>
//MPR-3D https://blog.csdn.net/seugxf/article/details/105201516
//https://blog.csdn.net/weixin_38500110/article/details/78807196
namespace udg {

MPRExtensionMediator::MPRExtensionMediator(QObject *parent)
 : ExtensionMediator(parent)
{
}

MPRExtensionMediator::~MPRExtensionMediator()
{
}

DisplayableID MPRExtensionMediator::getExtensionID() const
{
    return DisplayableID("MPRExtension", tr("MPR-2D"));
}

bool MPRExtensionMediator::initializeExtension(QWidget *extension, const ExtensionContext &extensionContext)
{
    QMPRExtension *mprExtension;

    if (!(mprExtension = qobject_cast<QMPRExtension*>(extension)))
    {
        return false;
    }

    Volume *input = extensionContext.getDefaultVolume();
	//Volume * selVolume = QViewer::selectVolume();
	//Volume *input = selVolume ? selVolume : extensionContext.getDefaultVolumeNoLocalizer();
    if (!input)
    {
        QMessageBox::information(0, tr("Starviewer"), tr("The selected item is not an image"));
    }
    else
    {
        mprExtension->setInput(input);
    }

    return true;
}

void  MPRExtensionMediator::executionCommand(QWidget *extension, Volume* volume, void *data , int command )
{

}

} // End udg namespace
