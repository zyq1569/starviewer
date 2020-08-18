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

#ifndef PACSDEVICE
#define PACSDEVICE

#include <QString>

namespace udg {

/**
    Classe que encapsula els paràmetres necessaris per definir un servidor PACS al qual connectar-nos
    Aquests paràmetres són:

    Dades de connexió (imprescindibles)
       - AETitle del PACS
       - IP del servidor PACS
       - Port de connexió del PACS

    Dades Descriptives (opcionals)
       - Institució a la que pertany el PACS
       - Descripció del PACS
       - Ubicació del PACS

    Altres dades
       - AETitle de la màquina local
       - Nombre màxim de connexions simultànies
       - Timeout de la connexió
       - Port Local de Query/Retrieve
    TODO aquestes dades s'haurien de reubicar en un lloc més adient ja que són
    les mateixes per a qualsevol connexió amb un PACS i per tant no haurien de formar part d'aquesta classe
  */
class PacsDevice {
public:
    PacsDevice();

    /// Assigna/Retorna l'adreça al PACS al qual ens volem connectar
    void setAddress(const QString &address);
    QString getAddress() const;

    /// Assigna/Retorna el port del PACS al qual ens volem connectar
    void setQueryRetrieveServicePort(int port);
    int getQueryRetrieveServicePort() const;

    /// Assigna/Retorna l'AETitle del PACS al qual ens volem connectar
    void setAETitle(const QString &AETitle);
    QString getAETitle() const;

    /// Assigna/Retorna la descripció del PACS
    void setDescription(const QString &description);
    QString getDescription() const;

    /// Assigna/Retorna la institucio a la qual pertany el PACS
    void setInstitution(const QString &institution);
    QString getInstitution() const;

    /// Assigna/Retorna la ubicació del PACS
    void setLocation(const QString &location);
    QString getLocation() const;

    /// Assigna/Retorna si aquest PACS és un de predeterminat per fer les consultes.
    void setDefault(bool isDefault);
    bool isDefault() const;

    /// Assigna/Retorna l'ID del PACS. Camp clau assignat per l'aplicació.
    void setID(const QString &id);
    QString getID() const;

    /// Assigna/Retorna si podem fer consultes/descarregues al PACS
    void setQueryRetrieveServiceEnabled(bool isQueryRetrieveServiceEnabled);
    bool isQueryRetrieveServiceEnabled() const;

    /// Assigna/Retorna si podem enviar imatges al PACS
    void setStoreServiceEnabled(bool isStoreServiceEnabled);
    bool isStoreServiceEnabled() const;

    /// Assigna/Retorna el port pel qual hem d'enviar imatges al PACS
    void setStoreServicePort(int storeServicePort);
    int getStoreServicePort() const;

    /// Ens diu si aquest objecte conté dades o no
    bool isEmpty() const;

    /// Ens indica si el PACS passat és el mateix que l'objecte actual. Ho serà quan tinguin el mateix AETitle, Address i QueryPort
    bool isSamePacsDevice(const PacsDevice &pacsDevice) const;

    bool operator ==(const PacsDevice &device) const;

private:
    /// Ens retorna el KeyName que identifica el PACS
    QString getKeyName() const;

    /// Ens retorna la llista de noms claus de PACS seleccionats per defecte
    QStringList getDefaultPACSKeyNamesList() const;

private:
    QString m_AETitle;
    int m_queryRetrieveServicePort;
    QString m_address;
    QString m_description;
    QString m_institution;
    QString m_location;
    QString m_id;
    bool m_isQueryRetrieveServiceEnabled;
    bool m_isStoreServiceEnabled;
    int m_storeServicePort;
};

}
#endif
