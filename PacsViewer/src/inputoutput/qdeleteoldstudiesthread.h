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


#ifndef UDGDELETEOLDSTUDIESTHREAD_H
#define UDGDELETEOLDSTUDIESTHREAD_H

#include <QThread>

#include "localdatabasemanager.h"

namespace udg {

/**
    Class in charge of erasing old studies in a thread
 */
class QDeleteOldStudiesThread : public QThread {
    Q_OBJECT

public:
    ///Class builder
    QDeleteOldStudiesThread(QObject *parent = 0);

    ///Delete old studies by starting a thread
    void deleteOldStudies();

    /// Returns the status of the operation to delete old studies
    LocalDatabaseManager::LastError getLastError();

signals:
    /// Signal that is sent when the execution of this thread ends
    void finished();

private:
    /// Method that is executed by the thread created by Qt, which deletes the old studies
    void run();

    LocalDatabaseManager::LastError m_lastError;

};

}

#endif
