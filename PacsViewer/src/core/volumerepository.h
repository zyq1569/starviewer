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
    Aquesta classe és el repositori de volums. En aquesta classe es guarden tots els volums que hi ha oberts durant
    l'execució del programa. Només hi haura una sola instància en tota la vida del programa d'aquesta classe.
    Per fer-ho s'aplica el patró Singleton.

    Per poder obtenir una instància del repositori hem de fer un include del fitxer volumerepository.h i fer una crida
    al mètode VolumeRepository::getRepository(). Aquest ens retornarà un punter al repositori de volums.

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
    /// Afegeix un volum al repositori.
    /// Ens retorna l'id del volum afegit per poder-lo obtenir més endavant.
    Identifier addVolume(Volume *model);

    /// Ens retorna un volum del repositori amb l'identificador que especifiquem.
    Volume* getVolume(Identifier id);

    /// Esborra un Volume de memòria i el treu del repositori
    void deleteVolume(Identifier id);

    /// Retorna el nombre de volums que hi ha al repositori
    int getNumberOfVolumes();

    /// Ens retorna l'única instància del repositori.
    static VolumeRepository* getRepository()
    {
        static VolumeRepository repository;
        return &repository;
    }

    /// El destructor allibera l'espai ocupat pels volums
    ~VolumeRepository(){};

signals:
    void itemAdded(Identifier id);
    void itemRemoved(Identifier id);

private:
    /// Ha de quedar amagat perquè no poguem crear instàncies
    VolumeRepository();
};

}

#endif
