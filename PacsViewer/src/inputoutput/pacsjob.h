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

#ifndef UDGPACSJOB_H
#define UDGPACSJOB_H

#include <QObject>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/Thread>

#include "pacsdevice.h"

namespace ThreadWeaver {

class WeaverInterface;

}

namespace udg {

class PACSJob;
typedef QSharedPointer<PACSJob> PACSJobPointer;

/**
Base class from which they will inherit all the operations that
are done with the PACS. This class contains the basic methods to be inherited.
This class inherits from ThreadWeaver :: Job in order
to automatically have the queue management it implements, and allow operations
with the PACS run in a separate thread.
*/
class PACSJob : public QObject, public ThreadWeaver::Job {
    Q_OBJECT
public:
    enum PACSJobType { SendDICOMFilesToPACSJobType, RetrieveDICOMFilesFromPACSJobType, QueryPACS };

    /// Class builder
    PACSJob(PacsDevice pacsDevice);

    /// Returns the PACSJob identifier this identifier is unique for all PACSJob
    int getPACSJobID();

    /// Returns the PacsDevice with which the PACSJob was built
    PacsDevice getPacsDevice();

    /// Indicates what type of PACSJob the object is
    virtual PACSJob::PACSJobType getPACSJobType() = 0;

    /// Inherited Job method that is used to cancel the execution of the current job.
    /// If the job is not running and is still running
    /// this method will do nothing so it is advised not to use this method,
    /// instead use requestCancelPACSJob from PACSManager than in the case
    /// that the job is executed requests that the execution be stopped
    ///  and if it is blocked it disengages it so that it is not executed.
    void requestAbort();

    /// Returns if the job abort was requested
    bool isAbortRequested();

    /// Inherited Job method, runs just before unsetting the job,
    /// if they thicken us it means that the job will not run therefore
    /// from this method we emit the PACSJobCancelled signal
    void aboutToBeDequeued(ThreadWeaver::QueueAPI *weaver);

    /// Sets the self pointer reference of this job.
    void setSelfPointer(const PACSJobPointer &self);

signals:
    /// Signal that is emitted when a PACSJob has started running
    void PACSJobStarted(PACSJobPointer);

    ///Signal that is emitted when a PACSJob has finished running
    void PACSJobFinished(PACSJobPointer);

    ///Signal that is emitted when a PACSJob has been canceled
    void PACSJobCancelled(PACSJobPointer);

protected:
    virtual void defaultBegin(const ThreadWeaver::JobPointer &job, ThreadWeaver::Thread *thread);
    virtual void defaultEnd(const ThreadWeaver::JobPointer &job, ThreadWeaver::Thread *thread);

protected:
    /// Weak reference to a shared pointer of the job itself.
    /// It is needed to emit the PACSJobCancelled() signal with a shared pointer from aboutToBeDequeued().
    /// Since it's a weak pointer it won't keep the job alive.
    /// TODO This should be removed by redesigning the PACS jobs architecture.
    QWeakPointer<PACSJob> m_selfPointer;

private:
    /// Method that the child classes must reimplement to cancel the execution of the current job
    virtual void requestCancelJob() = 0;

private:
    static int m_jobIDCounter;
    int m_jobID;
    PacsDevice m_pacsDevice;
    bool m_abortIsRequested;
};

};

Q_DECLARE_METATYPE(QSharedPointer<udg::PACSJob>)

#endif
