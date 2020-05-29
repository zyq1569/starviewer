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

#include "settings.h"

#include "logging.h"
#include "starviewerapplication.h"
#include "settingsregistry.h"
#include "settingsparser.h"

#include <QTreeWidget>
// Pel restoreColumnsWidths
#include <QHeaderView>
// Pels saveGeometry(),restoreGeometry() de QSplitter
#include <QSplitter>

namespace udg {

Settings::Settings()
{
    QSettings *userSettings = new QSettings(QSettings::UserScope, OrganizationNameString, ApplicationNameString);
    QSettings *systemSettings = new QSettings(QSettings::SystemScope, OrganizationNameString, ApplicationNameString);

    m_qsettingsObjectsMap.insert(UserLevel, userSettings);
    m_qsettingsObjectsMap.insert(SystemLevel, systemSettings);
}

Settings::~Settings()
{
    // Alliberem la memòria ocupada pels objectes
    foreach (QSettings *setting, m_qsettingsObjectsMap)
    {
        delete setting;
    }
}

QVariant Settings::getValue(const QString &key) const
{
    QVariant value;
    // Primer mirem si tenim valor als settings
    // Si estigués buit, llavors agafem el valor per defecte que tinguem al registre
    // TODO hauríem d'obtenir l'objecte de settings amb getSettingsObject(key)
    // però cal resoldre abans un problema de linkatge produit per projectes externs (crashreporter/sapwrapper)
    value = m_qsettingsObjectsMap.value(SettingsRegistry::instance()->getAccessLevel(key))->value(key);
    if (value == QVariant())
    {
        value = SettingsRegistry::instance()->getDefaultValue(key);
    }

    // Obtenir les propietats del setting
    // TODO de moment només tractem "Parseable"
    // si és Parseable, mirem de parsejar el valor, altrament el retornem tal qual
    Settings::Properties properties = SettingsRegistry::instance()->getProperties(key);
    if (properties & Settings::Parseable)
    {
        value = SettingsParser::instance()->parse(value.toString());
    }
    return value;
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    getSettingsObject(key)->setValue(key, value);
}

bool Settings::contains(const QString &key) const
{
    // TODO hauríem d'obtenir l'objecte de settings amb getSettingsObject(key)
    // però cal resoldre abans un problema de linkatge produit per projectes externs (crashreporter/sapwrapper)
    return m_qsettingsObjectsMap.value(SettingsRegistry::instance()->getAccessLevel(key))->contains(key);
}

void Settings::remove(const QString &key)
{
    getSettingsObject(key)->remove(key);
}

QStringList Settings::getValueAsQStringList(const QString &key, const QString &separator) const
{
    return getValue(key).toString().split(separator, QString::SkipEmptyParts);
}

Settings::SettingsListItemType Settings::getListItem(const QString &key, int index)
{
    SettingsListItemType item;
    QSettings *qsettings = getSettingsObject(key);

    int size = qsettings->beginReadArray(key);
    if (index < size && index >= 0)
    {
        qsettings->setArrayIndex(index);
        // Omplim el conjunt de claus-valor a partir de les claus de l'índex de la llista
        item = fillSettingsListItemFromKeysList(qsettings->allKeys(), qsettings);
    }
    else
    {
        DEBUG_LOG("Índex fora de rang. L'element retornat serà buit");
    }
    qsettings->endArray();

    return item;
}

Settings::SettingListType Settings::getList(const QString &key)
{
    SettingListType list;
    QSettings *qsettings = getSettingsObject(key);

    int size = qsettings->beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        qsettings->setArrayIndex(i);

        SettingsListItemType item;
        // Omplim el conjunt de claus-valor a partir de les claus de l'índex de la llista
        item = fillSettingsListItemFromKeysList(qsettings->allKeys(), qsettings);
        // Afegim el nou conjunts de valors a la llista
        list << item;
    }
    qsettings->endArray();

    return list;
}

void Settings::addListItem(const QString &key, const SettingsListItemType &item)
{
    QSettings *qsettings = getSettingsObject(key);

    int arraySize = qsettings->beginReadArray(key);
    qsettings->endArray();

    qsettings->beginWriteArray(key);
    qsettings->setArrayIndex(arraySize);
    // Omplim
    dumpSettingsListItem(item, qsettings);
    qsettings->endArray();
}

void Settings::setListItem(int index, const QString &key, const SettingsListItemType &item)
{
    Settings::SettingListType list = getList(key);
    int listSize = list.count();
    if (index >= 0 && index < listSize)
    {
        list[index] = item;
        setList(key, list);
    }
    else
    {
        DEBUG_LOG("L'index està fora de rang més gran");
    }
}

void Settings::removeListItem(const QString &key, int index)
{
    Settings::SettingListType list = getList(key);
    int listSize = list.count();
    if (index >= 0 && index < listSize)
    {
        list.removeAt(index);
        setList(key, list);
    }
    else
    {
        DEBUG_LOG("L'index està fora de rang més gran");
    }
}

void Settings::setList(const QString &key, const SettingListType &list)
{
    QSettings *qsettings = getSettingsObject(key);
    // Eliminem tot el que pogués haver d'aquella llista anteriorment
    remove(key);
    int index = 0;
    // Escrivim la llista
    qsettings->beginWriteArray(key);
    foreach (const SettingsListItemType &item, list)
    {
        qsettings->setArrayIndex(index);
        dumpSettingsListItem(item, qsettings);
        index++;
    }
    qsettings->endArray();
}

void Settings::saveColumnsWidths(const QString &key, QTreeWidget *treeWidget)
{
    Q_ASSERT(treeWidget);

    int columnCount = treeWidget->columnCount();
    QString columnKey;
    for (int column = 0; column < columnCount; column++)
    {
        columnKey = key + "/columnWidth" + QString::number(column);
        this->setValue(columnKey, treeWidget->columnWidth(column));
    }
}

void Settings::restoreColumnsWidths(const QString &key, QTreeWidget *treeWidget)
{
    Q_ASSERT(treeWidget);

    int columnCount = treeWidget->columnCount();
    QString columnKey;
    for (int column = 0; column < columnCount; column++)
    {
        columnKey = key + "/columnWidth" + QString::number(column);
        if (!this->contains(columnKey))
        {
            treeWidget->resizeColumnToContents(column);
        }
        else
        {
            treeWidget->header()->resizeSection(column, this->getValue(columnKey).toInt());
        }
    }
}

void Settings::saveGeometry(const QString &key, QWidget *widget)
{
    Q_ASSERT(widget);
    this->setValue(key, widget->saveGeometry());
}

void Settings::restoreGeometry(const QString &key, QWidget *widget)
{
    Q_ASSERT(widget);
    widget->restoreGeometry(this->getValue(key).toByteArray());
}

void Settings::saveGeometry(const QString &key, QSplitter *splitter)
{
    Q_ASSERT(splitter);
    this->setValue(key, splitter->saveState());
}

void Settings::restoreGeometry(const QString &key, QSplitter *splitter)
{
    Q_ASSERT(splitter);
    splitter->restoreState(this->getValue(key).toByteArray());
}

Settings::SettingsListItemType Settings::fillSettingsListItemFromKeysList(const QStringList &keysList, QSettings *qsettings)
{
    Q_ASSERT(qsettings);

    SettingsListItemType item;

    foreach (const QString &key, keysList)
    {
        item[key] = qsettings->value(key);
    }
    return item;
}

void Settings::dumpSettingsListItem(const SettingsListItemType &item, QSettings *qsettings)
{
    Q_ASSERT(qsettings);

    QStringList keysList = item.keys();
    foreach (const QString &key, keysList)
    {
        qsettings->setValue(key, item.value(key));
    }
}

QSettings *Settings::getSettingsObject(const QString &key)
{
    return m_qsettingsObjectsMap.value(SettingsRegistry::instance()->getAccessLevel(key));
}

}  // End namespace udg
