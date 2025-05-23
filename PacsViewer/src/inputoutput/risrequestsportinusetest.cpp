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

#include "risrequestsportinusetest.h"
#include "inputoutputsettings.h"

namespace udg {

RISRequestsPortInUseTest::RISRequestsPortInUseTest(QObject *parent)
 : PortInUseTest(parent)
{
    m_port = Settings().getValue(InputOutputSettings::RISRequestsPort).toInt();
}

RISRequestsPortInUseTest::~RISRequestsPortInUseTest()
{
}

QString RISRequestsPortInUseTest::getDescription() const
{
    return tr("RIS request port is available");
}

} // End udg namespace
