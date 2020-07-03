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
Class that encapsulates the parameters needed to define a PACS server to connect to
     These parameters are:

     Connection details (required)
        - AETitle of the PACS
        - PACS server IP
        - PACS connection port

     Descriptive Data (optional)
        - Institution to which the PACS belongs
        - Description of the PACS
        - Location of the PACS

     Other data
        - AETitle of the local machine
        - Maximum number of simultaneous connections
        - Connection timeout
        - Local Port of Query / Retrieve
     ALL of this data should be relocated to a more appropriate location as it is
     the same for any connection to a PACS and therefore should not be part of this class
  */
class PacsDevice {
public:
    PacsDevice();

    /// Assign / Return the address to the PACS to which we want to connect
    void setAddress (const QString & address);
    QString getAddress () const;

    /// Assign / Return the port of the PACS to which we want to connect
    void setQueryRetrieveServicePort (int port);
    int getQueryRetrieveServicePort () const;

    /// Assign / Return the AETitle of the PACS to which we want to connect
    void setAETitle (const QString & AETitle);
    QString getAETitle () const;

    /// Assign / Return the PACS description
    void setDescription (const QString & description);
    QString getDescription () const;

    /// Assigns / Returns the institution to which the PACS belongs
    void setInstitution (const QString & institution);
    QString getInstitution () const;

    /// Assign / Return the location of the PACS
    void setLocation (const QString & location);
    QString getLocation () const;

    /// Assign / Return if this PACS is the default for queries.
    void setDefault (bool isDefault);
    bool isDefault () const;

    /// Assign / Return the PACS ID. Key field assigned by the application.
    void setID (const QString & id);
    QString getID () const;

    /// Assign / Return if we can make queries / downloads to the PACS
    void setQueryRetrieveServiceEnabled (bool isQueryRetrieveServiceEnabled);
    bool isQueryRetrieveServiceEnabled () const;

    /// Assign / Return if we can send images to the PACS
    void setStoreServiceEnabled (bool isStoreServiceEnabled);
    bool isStoreServiceEnabled () const;

    /// Assign / Return the port through which we have to send images to the PACS
    void setStoreServicePort (int storeServicePort);
    int getStoreServicePort () const;

    /// Tells us if this object contains data or not
    bool isEmpty () const;

    /// Tells us if the past PACS is the same as the current object. It will be when they have the same AETitle, Address and QueryPort
    bool isSamePacsDevice (const PacsDevice & pacsDevice) const;
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
