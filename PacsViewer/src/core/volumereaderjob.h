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

#ifndef UDGVOLUMEREADERJOB_H
#define UDGVOLUMEREADERJOB_H

#include <ThreadWeaver/Job>

#include "identifier.h"

namespace udg {

class Volume;
class VolumeReader;

/**
    Classe que s'encarrega de llegir el pixel data d'un Volume en forma de job de ThreadWeaver, és a dir
    asíncronament.

    Class that is responsible for reading the pixel data of a Volume in the form of a ThreadWeaver job, that is,
    asynchronously.
  */
class VolumeReaderJob : public QObject, public ThreadWeaver::Job {
Q_OBJECT
public:
    /// Constructor, cal passar-li el volume del que es vol llegir el pixel data.
    VolumeReaderJob(Volume *volume, QObject *parent = 0);
    virtual ~VolumeReaderJob();

    /// Ens permet demanar, de manera asíncrona, que es cancel·li el job.
    /// El mètode retornarà inmediatament però el job no es cancel·larà fins al cap d'una estona.
    virtual void requestAbort();

    /// Ens indica si el volume s'ha llegit correctament. Si es fa un request abort, es retornarà que no s'ha llegit correctament.
    bool success() const;

    /// Ens retorna l'error en format visible per l'usuari de la última execució del job.
    /// Si no hi ha error, retorna cadena buida.
    /// TODO: Ens veiem obligats a fer-ho així de moment, per com està a VolumeReader. Idealment aquí només es retornaria
    /// el codi d'error i és des de la interfície que es converteix en missatge a l'usuari.
    QString getLastErrorMessageToUser() const;

    /// Retorna el volume
    Volume* getVolume() const;
    /// Returns the identifier of the volume, even if the volume is destructed.
    const Identifier& getVolumeIdentifier() const;

signals:
    /// Signal que s'emet amb el progrés de lectura
    void progress(ThreadWeaver::JobPointer, int progress);
    void done(ThreadWeaver::JobPointer);

protected:
    /// Mètode on realment es fa la càrrega. S'executa en un thread de threadweaver.
    virtual void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread);
    virtual void defaultEnd(const ThreadWeaver::JobPointer &job, ThreadWeaver::Thread *thread);

private:
    Volume *m_volumeToRead;
    VolumeReader *m_volumeReader;

    /// Keeps the identifier of the volume to have access to it even if the volume is deleted.
    Identifier m_volumeIdentifier;
    bool m_volumeReadSuccessfully;
    QString m_lastErrorMessageToUser;

    /// Ens indica si s'ha fet o no un requestAbort
    bool m_abortRequested;

};

} // End namespace udg

#endif // VOLUMEREADERJOB_H
