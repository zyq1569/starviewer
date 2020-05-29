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

#ifndef UDGSETTINGS_H
#define UDGSETTINGS_H

#include "settingsinterface.h"

#include <QSettings>
#include <QMap>

// Forward declarations
class QString;
class QTreeWidget;
class QSplitter;

namespace udg {

/**
    Interfície per manipular settings.
    La manera correcta de fer servir la classe és declarant-la localment en el mètode en
    el que volem accedir o escriure al setting, mai es declararà com a variable membre d'una classe.
    L'escriptura d'un setting no es farà efectiva a disc fins que no es destrueixi la instància.
    Aquest mecanisme funciona d'aquesta manera per raons d'eficiència, així podem assignar diversos
    settings alhora sense penalitzar un accés a disc fins que no es destrueixi l'objecte de Settings.
  */
class Settings : public SettingsInterface {
public:
    enum AccessLevel { UserLevel, SystemLevel };
    enum PropertiesFlag { None = 0x0, Parseable = 0x1 };
    Q_DECLARE_FLAGS(Properties, PropertiesFlag)

    Settings();
    virtual ~Settings();

    /// Retorna el valor per la clau demanada. Si el setting no existeix, retorna el valor
    /// per defecte que aquesta clau tingui registrat
    virtual QVariant getValue(const QString &key) const;

    /// Assigna el valor al setting amb la clau indicada. Si la clau ja existeix, els valor anterior queda sobre-escrit.
    virtual void setValue(const QString &key, const QVariant &value);

    /// Retorna cert si existeix un setting anomenat key; returna fals altrament.
    virtual bool contains(const QString &key) const;

    /// Elimina els settings de key i totes les sub-settings que hi hagi per sota de la clau
    virtual void remove(const QString &key);

    /// Helper method to return a value as a QStringList in case values are convertible to string and the adequate values separator is used
    /// If no separator is used, the default value will be used
    QStringList getValueAsQStringList(const QString &key, const QString &separator = ";") const;
    
    // Mètodes per la manipulació de llistes de Settings

    // També existeix el tipus QSettings::SettingsMap que és QMap<QString, QVariant>
    // TODO podríem fer typedef QSettings::SettingsMap en comptes d'això
    typedef QMap<QString, QVariant> SettingsListItemType;
    typedef QList<SettingsListItemType> SettingListType;

    // Obtenció d'informació de llistes de settings

    /// Ens retorna l'i-éssim (index) conjunt de valors de la llista amb clau "key"
    SettingsListItemType getListItem(const QString &key, int index);

    /// Ens retorna tota la llista de settings que hi hagi sota key
    SettingListType getList(const QString &key);

    // Modificació de llistes de conjunts de valors

    /// Afegeix a la llista amb clau "key" un conjunt de valors
    void addListItem(const QString &key, const SettingsListItemType &item);

    /// Actualitza les dades del conjunt de valors "item" a l'índex index de la llista amb clau "key"
    void setListItem(int index, const QString &key, const SettingsListItemType &item);

    /// Elimina de la llista amb clau "key" l'element i-éssim (index)
    void removeListItem(const QString &key, int index);

    /// Afegeix una llista sencera de conjunts de valors amb clau "key".
    /// Si existia una llista anteriorment amb aquesta clau, els valors queden
    /// sobre-escrits per la llista proporcionada
    void setList(const QString &key, const SettingListType &list);

    // Mètodes per facilitar el guardar i/o restaurar la geometria de certs widgets

    /// Guarda/Restaura els amples de columna del widget dins de la clau donada.
    /// Sota la clau donada es guardaran els amples de cada columna amb nom columnWidthX on X serà el nombre de columna
    /// L'unica implementació de moment és per QTreeWidget (i classes que n'hereden).
    /// Es sobrecarregarà el mètode per tants widgets com calgui.
    void saveColumnsWidths(const QString &key, QTreeWidget *treeWidget);
    void restoreColumnsWidths(const QString &key, QTreeWidget *treeWidget);

    /// Guarda/Restaura la geometria d'un widget/splitter dins de la clau donada.
    void saveGeometry(const QString &key, QWidget *widget);
    void restoreGeometry(const QString &key, QWidget *widget);
    void saveGeometry(const QString &key, QSplitter *splitter);
    void restoreGeometry(const QString &key, QSplitter *splitter);

private:
    /// A partir d'una llista de claus, omplim un conjunt clau-valor.
    /// És necessari que li passem l'objecte qsettings (user/system) amb el que obtindrà els valors
    SettingsListItemType fillSettingsListItemFromKeysList(const QStringList &keysList, QSettings *qsettings);

    /// Traspassa el contingut del conjunt clau-valor a m_settings
    /// És necessari que li passem l'objecte qsettings (user/system) en el que volcarà els valors
    void dumpSettingsListItem(const SettingsListItemType &item, QSettings *qsettings);

    /// Ens retorna l'objecte adient de settings (usuari o sistema)
    /// segons com estigui configurada la clau en qüestió
    QSettings* getSettingsObject(const QString &key);

private:
    /// Objectes QSettings amb el que manipularem les configuracions
    QMap<int, QSettings*> m_qsettingsObjectsMap;
};
} // End namespace udg
Q_DECLARE_OPERATORS_FOR_FLAGS(udg::Settings::Properties)

#endif
