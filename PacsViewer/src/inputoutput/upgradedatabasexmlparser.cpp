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

#include "upgradedatabasexmlparser.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QXmlStreamAttributes>
#include <QScopedPointer>

#include "upgradedatabaserevisioncommands.h"
#include "logging.h"

namespace udg {

UpgradeDatabaseXMLParser::UpgradeDatabaseXMLParser(const QString &xmlData)
{
    m_xmlData = xmlData;
}

int UpgradeDatabaseXMLParser::getMinimumDatabaseRevisionRequiredToUpgrade() const
{
    QScopedPointer<QXmlStreamReader> reader(getXmlReader());

    if (reader.isNull())
    {
        ERROR_LOG("XML d'actualització de la base de dades buit");
        return -1;
    }
    if (!reader->readNextStartElement())
    {
        ERROR_LOG("XML d'actualitzacio de base de dades no valid");
        return -1;
    }
    else if (reader->name().toString() != "upgradeDatabase")
    {
        ERROR_LOG("S'esperava tag upgradeDatabase i s'ha trobat" + reader->name().toString());
        return -1;
    }

    return reader->attributes().value("minimumDatabaseRevisionRequired").toString().toInt();
}

UpgradeDatabaseRevisionCommands UpgradeDatabaseXMLParser::getUpgradeDatabaseRevisionCommands(int fromDatabaseRevision) const
{
    QScopedPointer<QXmlStreamReader> reader(getXmlReader());

    if (reader.isNull())
    {
        return UpgradeDatabaseRevisionCommands();
    }

    if (!reader->readNextStartElement())
    {
        ERROR_LOG("XML d'actualitzacio de base de dades no valid");
        return UpgradeDatabaseRevisionCommands();
    }
    else if (reader->name().toString() != "upgradeDatabase")
    {
        ERROR_LOG("S'esperava tag upgradeDatabase i s'ha trobat" + reader->name().toString());
        return UpgradeDatabaseRevisionCommands();
    }

    return parseUpgradeDatabaseTag(reader.data(), fromDatabaseRevision);
}

UpgradeDatabaseRevisionCommands UpgradeDatabaseXMLParser::parseUpgradeDatabaseTag(QXmlStreamReader *reader, int fromDatabaseRevision) const
{
    //Guardem les sentències per actualitzar la base de dade en un QMap, per així si el XML d'actualització no té les comandes
    //ordenades per revisió en ordre ascendent, amb el QMap les podem tornar ordenades correctament
    QMap<int, QStringList> sqlUpgradeCommandsGroupedByDatabaseRevision;

    while (!reader->atEnd())
    {
        if (reader->isStartElement() && reader->name() == "upgradeDatabaseToRevision")
        {
             int upgradeToRevisionParsed = reader->attributes().value("updateToRevision").toString().toInt();

            if (upgradeToRevisionParsed > fromDatabaseRevision)
            {
                reader->readNextStartElement();

                sqlUpgradeCommandsGroupedByDatabaseRevision.insertMulti(upgradeToRevisionParsed, parseUpgradeDatabaseToRevisionChildrenTags(reader));
            }
            else
            {
                //Aquesta canvis ja estant aplicats, saltem als següents
                reader->skipCurrentElement();
            }
        }
        else
        {
            reader->readNext();
        }
    }

    return fromQMapToUpgradeDatabaseRevisionCommands(sqlUpgradeCommandsGroupedByDatabaseRevision);
}

QStringList UpgradeDatabaseXMLParser::parseUpgradeDatabaseToRevisionChildrenTags(QXmlStreamReader *reader) const
{
    QStringList sqlUpgradeCommands;

    while (!reader->atEnd() && reader->name() == "upgradeCommand")
    {
        if (reader->isStartElement())
        {
           sqlUpgradeCommands.append(reader->readElementText());
        }

        reader->readNextStartElement();
    }

    return sqlUpgradeCommands;
}

UpgradeDatabaseRevisionCommands UpgradeDatabaseXMLParser::fromQMapToUpgradeDatabaseRevisionCommands(QMap<int, QStringList> m_sqlUpgradeCommandsGroupedByDatabaseRevision) const
{
    if (m_sqlUpgradeCommandsGroupedByDatabaseRevision.count() == 0)
    {
        return UpgradeDatabaseRevisionCommands();
    }

    //Obtenim les sentències sql a aplicar a la base de dades ordenades per la revisió ascendentment
    QStringList sqlUpgradeCommands;

    foreach(QStringList sqlUpgradeCommandsParsed, m_sqlUpgradeCommandsGroupedByDatabaseRevision.values())
    {
        sqlUpgradeCommands.append(sqlUpgradeCommandsParsed);
    }

    //Obtenim quin és el número de revisió més gran que hem parsejat, keys retorna els número de revisions ordenats ascendentment
    int upgradeDatbaseToRevision = m_sqlUpgradeCommandsGroupedByDatabaseRevision.keys().at(m_sqlUpgradeCommandsGroupedByDatabaseRevision.count() - 1);

    UpgradeDatabaseRevisionCommands upgradeDatabaseRevisionCommands;
    upgradeDatabaseRevisionCommands.setSqlUpgradeCommands(sqlUpgradeCommands);
    upgradeDatabaseRevisionCommands.setUpgradeToDatabaseRevision(upgradeDatbaseToRevision);

    return upgradeDatabaseRevisionCommands;

}

QXmlStreamReader* UpgradeDatabaseXMLParser::getXmlReader() const
{
    return new QXmlStreamReader(m_xmlData);
}

}
