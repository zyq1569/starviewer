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

#ifndef UDGEDITORTOOLDATA_H
#define UDGEDITORTOOLDATA_H

#include "tooldata.h"

namespace udg {

class EditorToolData : public ToolData {
Q_OBJECT
public:
    EditorToolData(QObject *parent = 0);
    ~EditorToolData();

    void setVolumeVoxels(int vol);

    int getVolumeVoxels();

private:
    /// We store the position of the seed
    int m_volumeCont;

};

}

#endif
