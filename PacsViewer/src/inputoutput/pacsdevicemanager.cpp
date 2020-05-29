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

#include "pacsdevicemanager.h"

#include <QString>
#include "pacsdevice.h"
#include "logging.h"
#include "inputoutputsettings.h"

namespace udg {

PacsDeviceManager::PacsDeviceManager()
{
}

PacsDeviceManager::~PacsDeviceManager()
{
}

bool PacsDeviceManager::addPACS(PacsDevice &pacs)
{
    // Si el PACS ja existeix no l'afegim
    bool ok = !this->isPACSConfigured(pacs);
    if (ok)
    {
        // En cas que existeixi, li assignarem l'ID
        QList<PacsDevice> pacsList = getPACSList();
        if (pacsList.isEmpty())
        {
            // Si encara no teníem cap PACS, l'ID inicial serà 0
            pacs.setID(QString::number(0));
        }
        else
        {
            // En cas que ja en tinguem de configurats, l'ID serà
            // l'ID més alt dels configurats + 1
            int highestID = 0;
            foreach (PacsDevice pacs, pacsList)
            {
                if (pacs.getID().toInt() > highestID)
                {
                    highestID = pacs.getID().toInt();
                }
            }
            pacs.setID(QString::number(highestID + 1));
        }

        Settings settings;
        settings.addListItem(InputOutputSettings::PacsListConfigurationSectionName, pacsDeviceToSettingsListItem(pacs));
    }

    return ok;
}

void PacsDeviceManager::updatePACS(PacsDevice &pacsToUpdate)
{
    // Obtenim la llista completa de PACS
    QList<PacsDevice> pacsList = getPACSList();
    // Eliminem tots els PACS que tinguem guardats a disc
    Settings settings;
    settings.remove(InputOutputSettings::PacsListConfigurationSectionName);

    // Recorrem tota la llista de PACS i els afegim de nou
    // Si trobem el que volem fer update, afegim l'actualitzat
    foreach (PacsDevice device, pacsList)
    {
        if (pacsToUpdate.getID() == device.getID())
        {
            addPACS(pacsToUpdate);
        }
        else
        {
            addPACS(device);
        }
    }
}

bool PacsDeviceManager::deletePACS(const QString &pacsIDString)
{
    // Obtenim la llista completa de PACS
    QList<PacsDevice> pacsList = getPACSList();
    // Eliminem tots els PACS que tinguem guardats a disc
    Settings settings;
    settings.remove(InputOutputSettings::PacsListConfigurationSectionName);

    // Recorrem tota la llista de PACS i els afegim de nou
    // excepte el que volem esborrar
    foreach (PacsDevice device, pacsList)
    {
        if (pacsIDString != device.getID())
        {
            addPACS(device);
        }
    }
    return true;
}

QList<PacsDevice> PacsDeviceManager::getPACSList(FilterPACSByService filter, bool onlyDefault)
{
    QList<PacsDevice> configuredPacsList;
    Settings settings;
    Settings::SettingListType list = settings.getList(InputOutputSettings::PacsListConfigurationSectionName);
    foreach (Settings::SettingsListItemType item, list)
    {
        PacsDevice pacs;
        pacs = settingsListItemToPacsDevice(item);
        // Depenent del paràmetre "onlyDefault" afegirem o no els pacs
        if ((onlyDefault && pacs.isDefault()) || !onlyDefault)
        {
            // Filtrem per servei si ens ho han demanat
            if (filter == PacsDeviceManager::AllPacs ||
                (filter == PacsDeviceManager::PacsWithQueryRetrieveServiceEnabled && pacs.isQueryRetrieveServiceEnabled()) ||
                (filter == PacsDeviceManager::PacsWithStoreServiceEnabled && pacs.isStoreServiceEnabled()))
            {
                configuredPacsList << pacs;
            }
        }
    }

    return configuredPacsList;
}

PacsDevice PacsDeviceManager::getPACSDeviceByID(const QString &pacsIDString)
{
    QList<PacsDevice> pacsList = getPACSList();
    PacsDevice pacs;

    bool found = false;
    int i = 0;
    int count = pacsList.count();
    while (!found && i < count)
    {
        if (pacsIDString == pacsList.at(i).getID())
        {
            found = true;
            pacs = pacsList.at(i);
        }
        i++;
    }

    if (!found)
    {
        DEBUG_LOG("No existeix cap PACS amb aquest ID: " + pacsIDString);
        ERROR_LOG("No existeix cap PACS amb aquest ID: " + pacsIDString);
    }
    return pacs;
}

PacsDevice PacsDeviceManager::getPACSDeviceByAddressAndQueryPort(QString address, int queryRetrieveServicePort)
{
    PacsDevice pacsDeviceToReturn;

    foreach(PacsDevice pacs, getPACSList())
    {
        if (address == pacs.getAddress() && queryRetrieveServicePort == pacs.getQueryRetrieveServicePort())
        {
            pacsDeviceToReturn = pacs;
            break;
        }
    }

    if (pacsDeviceToReturn.isEmpty())
    {
        ERROR_LOG(QString("No existeix cap PACS amb aquest adreca: %1, port query: %2").arg(address, queryRetrieveServicePort));
    }

    return pacsDeviceToReturn;
}

QList<PacsDevice> PacsDeviceManager::removeDuplicateSamePACS(QList<PacsDevice> pacsDeviceList)
{
    QList<PacsDevice> pacsDeviceListWithoutDuplicates;

    foreach (PacsDevice pacsDevice, pacsDeviceList)
    {
        if (!isAddedSamePacsDeviceInList(pacsDeviceListWithoutDuplicates, pacsDevice))
        {
            pacsDeviceListWithoutDuplicates.append(pacsDevice);
        }
    }

    return pacsDeviceListWithoutDuplicates;
}

bool PacsDeviceManager::isAddedSamePacsDeviceInList(QList<PacsDevice> pacsDeviceList, PacsDevice pacsDevice)
{
    foreach (PacsDevice pacsDeviceInList, pacsDeviceList)
    {
        if (pacsDeviceInList.isSamePacsDevice(pacsDevice))
        {
            return true;
        }
    }

    return false;
}

bool PacsDeviceManager::isPACSConfigured(const PacsDevice &pacs)
{
    QList<PacsDevice> pacsList = getPACSList();

    foreach (PacsDevice pacsDevice, pacsList)
    {
        if (pacsDevice.getAETitle() == pacs.getAETitle() &&
            pacsDevice.getQueryRetrieveServicePort() == pacs.getQueryRetrieveServicePort() &&
            pacsDevice.getAddress() == pacs.getAddress())
        {
            return true;
        }
    }

    return false;
}

Settings::SettingsListItemType PacsDeviceManager::pacsDeviceToSettingsListItem(const PacsDevice &pacsDevice)
{
    Settings::SettingsListItemType item;

    item["ID"] = pacsDevice.getID();
    item["AETitle"] = pacsDevice.getAETitle();
    item["PacsPort"] = QString::number(pacsDevice.getQueryRetrieveServicePort());
    item["Location"] = pacsDevice.getLocation();
    item["Institution"] = pacsDevice.getInstitution();
    item["PacsHostname"] = pacsDevice.getAddress();
    item["Description"] = pacsDevice.getDescription();
    item["QueryRetrieveServiceEnabled"] = pacsDevice.isQueryRetrieveServiceEnabled();
    item["StoreServiceEnabled"] = pacsDevice.isStoreServiceEnabled();
    item["StoreServicePort"] = QString::number(pacsDevice.getStoreServicePort());

    return item;
}

PacsDevice PacsDeviceManager::settingsListItemToPacsDevice(const Settings::SettingsListItemType &item)
{
    PacsDevice pacsDevice;
    // TODO cal comprovar que hi ha les claus que volem? sinó quedarà amb valors empty
    pacsDevice.setID(item.value("ID").toString());
    pacsDevice.setAETitle(item.value("AETitle").toString());
    pacsDevice.setLocation(item.value("Location").toString());
    pacsDevice.setInstitution(item.value("Institution").toString());
    pacsDevice.setAddress(item.value("PacsHostname").toString());
    pacsDevice.setDescription(item.value("Description").toString());

    // A partir d'Starviewer 0.9 s'ofereix la possibilitat d'enviar imatges a PACS, aquest canvi implicar que s'ha de poder indicar
    // a quin port del PACS s'han d'enviar les imatges per guardar-les, per mantenir la comptabilitat amb PACS que han estat guardats al
    // settings amb versions anteriors s'executa el codi que hi ha a continuació

    if (!item.contains("QueryRetrieveServiceEnabled"))
    {
        // Si no està guardat als settings si està activat el servei de Query/Retrieve per defecte li indiquem que està activat
        pacsDevice.setQueryRetrieveServiceEnabled(true);
        pacsDevice.setQueryRetrieveServicePort(item.value("PacsPort").toInt());
    }
    else
    {
        pacsDevice.setQueryRetrieveServiceEnabled(item.value("QueryRetrieveServiceEnabled").toBool());

        if (pacsDevice.isQueryRetrieveServiceEnabled())
        {
            pacsDevice.setQueryRetrieveServicePort(item.value("PacsPort").toInt());
        }
    }

    if (!item.contains("StoreServiceEnabled"))
    {
        // Si no està guardat als settings si està activat el servei de Store Query/Retrieve per defecte li indique, que està activat i li
        // donem el mateix port que el servei de Query/Retrieve
        pacsDevice.setStoreServiceEnabled(true);
        pacsDevice.setStoreServicePort(item.value("PacsPort").toInt());
    }
    else
    {
        pacsDevice.setStoreServiceEnabled (item.value("StoreServiceEnabled").toBool());

        if (pacsDevice.isStoreServiceEnabled())
        {
            pacsDevice.setStoreServicePort(item.value("StoreServicePort").toInt());
        }
    }

    return pacsDevice;
}
};
