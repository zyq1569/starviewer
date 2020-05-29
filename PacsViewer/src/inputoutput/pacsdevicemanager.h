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

#ifndef UDGPACSDEVICEMANAGER_H
#define UDGPACSDEVICEMANAGER_H

#include <QList>
#include "pacsdevice.h"
#include "settings.h"

namespace udg {

/**
    Aquesta classe gestiona els servidors PACS configurats per l'aplicació. Permet afegir, eliminar, modificar i consultar
    les dades dels PACS configurats, que es guardaran com a Settings de l'aplicació.
  */
class PacsDeviceManager {

public:
    enum FilterPACSByService { AllPacs, PacsWithQueryRetrieveServiceEnabled, PacsWithStoreServiceEnabled };

    PacsDeviceManager();
    ~PacsDeviceManager();

    /// Afegeix un nou servidor PACS. En un alta, el camp PacsID, s'assigna automàticament per l'aplicació.
    /// @param Objecte PacsDevice amb les dades del PACS
    /// @return True en el cas que s'hagi afegit correctament, false si el PACS ja existia.
    bool addPACS(PacsDevice &pacs);

    /// Actualitza les dades del PACS passat per paràmetre. TODO què fem amb el camp ID?
    /// @param pacs Objecte PacsDevice amb les noves dades del PACS
    void updatePACS(PacsDevice &pacs);

    /// Elimina de la llista de PACS configurats el PACS amb l'ID passat per paràmetre.
    /// @param pacsID Identificador del PACS a donar de baixa
    /// @return True en cas d'eliminar-se el PACS amb èxit, false, si no existeix cap PACS a eliminar amb tal ID
    bool deletePACS(const QString &pacsID);

    /// Ens retorna la llista de PACS configurats
    /// @param onlyDefault Amb valor true, només inclou els que estiguin marcats a consultar per defecte
    /// @return Llista de PACS configurats
    QList<PacsDevice> getPACSList(FilterPACSByService filter = AllPacs, bool onlyDefault = false);

    /// Donat un ID de PACS, ens retorna el corresponent PacsDevice amb la seva informació
    /// @param pacsID ID del PACS a cercar
    /// @return Les dades del PACS si existeix algun amb aquest ID, sinó tindrem un objecte buit
    PacsDevice getPACSDeviceByID(const QString &pacsID);

    /// Retorna un PACS a partir de la seva adreça i port.
    /// Si en trobar més d'un retorna el primer que troba. En teoria no s'hauria de donar aquest cas
    PacsDevice getPACSDeviceByAddressAndQueryPort(QString address, int queryPort);

    /// Retorna la llista sense PACS duplicats
    static QList<PacsDevice> removeDuplicateSamePACS(QList<PacsDevice> pacsDeviceList);

    /// Mètode helper que indica si el mateix pacs està insertat (té el mateix AETitle, Address i QueryPort) a la llista.
    //TODO: Aquest codi està duplicat a DICOMSource, però com DICOMSource està al core no pot utilitzar aquest mètode, sinó tindríem dependència ciclica
    static bool isAddedSamePacsDeviceInList(QList<PacsDevice> pacsDeviceList, PacsDevice pacsDevice);

private:
    /// Comprova si el PACS passat per paràmetre es troba o no dins de la llista de PACS configurats
    /// @param pacs PACS a comprovar
    /// @return True si existeix, false en cas contrari
    bool isPACSConfigured(const PacsDevice &pacs);

    /// Donat un objecte PacsDevice el transformem en un conjunt de claus-valor per una manipulació de settings més còmode
    Settings::SettingsListItemType pacsDeviceToSettingsListItem(const PacsDevice &parameters);

    /// Donat un conjunt de claus-valor omple i retorna un objecte PacsDevice
    PacsDevice settingsListItemToPacsDevice(const Settings::SettingsListItemType &item);
};

};

#endif
