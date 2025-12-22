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

#ifndef UDGQLOGVIEWER_H
#define UDGQLOGVIEWER_H

#include "ui_qlogviewerbase.h"

namespace udg {

class QLogViewer : public QDialog, private Ui::QLogViewerBase {
Q_OBJECT
public:
    QLogViewer(QWidget *parent = 0);

    ~QLogViewer();

public slots:
    ///Opens a dialog to save the log file to another location
    void saveLogFileAs();

    ///Updates the data in the log file it displays
    void updateData();

private slots:
    /// Creates connections between signals and slots
    void createConnections();

private:
    void writeSettings();
    void readSettings();
};

}

#endif
