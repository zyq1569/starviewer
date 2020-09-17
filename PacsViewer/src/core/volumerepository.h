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

#ifndef UDGVOLUMEREPOSITORY_H
#define UDGVOLUMEREPOSITORY_H

#include "repository.h"
#include "volume.h"
#include "identifier.h"

#include <QObject>

namespace udg {

/**
This class is the volume repository. In this class are kept all the volumes that are open during
program execution. There will be only one instance in the entire life of the program in this class.
To do this the Singleton pattern is applied.

In order to get an instance of the repository we need to make an include of the volumerepository.h file and make a call
to the VolumeRepository :: getRepository () method. This will return a pointer to the volume repository.

Exemple:

\code
#include "volumerepository.h"
...
udg::VolumeRepository* m_volumeRepository;
m_volumeRepository = VolumeRepository::getRepository();
...
Volume* m_volume = m_volumeRepository->getVolume(id);
\endcode
  */
class VolumeRepository : public Repository<Volume> {
Q_OBJECT
public:
    /// Add a volume to the repository.
    /// Returns the id of the added volume so we can get it later.
    Identifier addVolume(Volume *model);

    /// Returns a volume from the repository with the identifier we specify.
    Volume* getVolume(Identifier id);

    /// Delete a Memory Volume and remove it from the repository
    void deleteVolume(Identifier id);

    /// Returns the number of volumes in the repository
    int getNumberOfVolumes();

    /// Returns us the only instance of the repository.
    static VolumeRepository* getRepository()
    {
        static VolumeRepository repository;
        return &repository;
    }

    /// The shredder frees up space occupied by volumes
    ~VolumeRepository(){};

signals:
    void itemAdded(Identifier id);
    void itemRemoved(Identifier id);

private:
    /// It must be hidden so that we cannot create instances
    VolumeRepository();
};

}

#endif
