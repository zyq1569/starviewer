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

#ifndef UDGSENDDICOMFILESTOPACSJOB_H
#define UDGSENDDICOMFILESTOPACSJOB_H

#include <QObject>

#include "pacsjob.h"
#include "pacsdevice.h"
#include "pacsrequeststatus.h"

namespace udg {

class Study;
class Image;
class SendDICOMFilesToPACS;

/**
    Job that is responsible for sending PACS files.
*/
class SendDICOMFilesToPACSJob : public PACSJob {
    Q_OBJECT
public:
    /// Attention, The list of images must contain the structure Patient, Study, Series, Images
    SendDICOMFilesToPACSJob(PacsDevice pacsDevice, QList<Image*>);
    ~SendDICOMFilesToPACSJob();

    ///Returns the type of PACSJob that is the object
    PACSJob::PACSJobType getPACSJobType();

    /// Code that will execute the job
    virtual void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread);

    /// Returns Job Status
    PACSRequestStatus::SendRequestStatus getStatus();

    /// Returns the Status described in a QString, this QString is
    /// intended to be displayed in QMessageBox to inform the user of the status returned
    /// the job in the same description indicates which study is affected
    QString getStatusDescription();

    /// Returns the list of images that were indicated to be saved
    QList<Image*> getFilesToSend();

    ///Returns the study of the images that were to be saved
    Study* getStudyOfDICOMFilesToSend();

signals:
    /// Signal that is emitted when an image is sent to the PACS
    void DICOMFileSent(PACSJobPointer pacsJob, int numberOfDICOMFilesSent);

    /// Signal that is emitted when a complete series has been sent to the PACS
    void DICOMSeriesSent(PACSJobPointer pacsJob, int numberOfSeriesSent);

private:
    /// Request that we cancel the job
    void requestCancelJob();

private slots:
    /// Slot that responds to the SendDICOMFilesToPACS DICOMFileSent signal
    void DICOMFileSent(Image *imageSent, int numberOfDICOMFilesSent);

private:
    QList<Image*> m_imagesToSend;
    PACSRequestStatus::SendRequestStatus m_sendRequestStatus;
    SendDICOMFilesToPACS *m_sendDICOMFilesToPACS;
    int m_numberOfSeriesSent;
    QString m_lastDICOMFileSeriesInstanceUID;
};

};

#endif
