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

#include "customwindowlevelswriter.h"
#include "windowlevel.h"
#include "customwindowlevelsrepository.h"
#include "starviewerapplication.h"
#include "logging.h"
#include "coresettings.h"

#include <QFile>
#include <QFileInfo>
#include <QXmlStreamWriter>

namespace udg {

CustomWindowLevelsWriter::CustomWindowLevelsWriter()
{
}

CustomWindowLevelsWriter::~CustomWindowLevelsWriter()
{
}

void CustomWindowLevelsWriter::write()
{
    // Establir el path per defecte on es guardarà el fitxer
    QString path = getPath();

    if (path.isEmpty())
    {
        return;
    }

    QFile file(path);
    if (file.exists())
    {
        // Si ja existeix l'esborrem ja que si és més llarg que el nou, hi quedaran caràcters del vell
        QFile::remove(path);
    }
    if (!file.open(QFile::ReadWrite | QFile::Text))
    {
        return;
    }

    QXmlStreamWriter writer(&file);

    // Encara que no hi hagi res al repository s'esborra el fitxer i es crea de nou
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(QString("CustomWindowLevels"));
    // TODO: probablement s'hauria de posar l'esquema que fa servir
    // Per exemple com a atribut xmlns

    // Per cada custom window level
    foreach (WindowLevel *windowLevel, CustomWindowLevelsRepository::getRepository()->getItems())
    {
        writer.writeStartElement(QString("CustomWindowLevel"));
        writer.writeAttribute(QString("name"), QString("%1").arg(windowLevel->getName()));
        // Un tag nou per window i per level
        writer.writeTextElement(QString("width"), QString("%1").arg(windowLevel->getWidth()));
        writer.writeTextElement(QString("level"), QString("%1").arg(windowLevel->getCenter()));
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();

    INFO_LOG(QString("S'han guardat els custom window levels a: %1").arg(path));
}

QString CustomWindowLevelsWriter::getPath()
{
    Settings systemSettings;
    QString userPath = systemSettings.getValue(CoreSettings::UserCustomWindowLevelsPath).toString();

    if (userPath.isEmpty())
    {
        return "";
    }

    QFileInfo fileInfo(userPath);
    QDir dir;
    if (!dir.exists(fileInfo.absolutePath()))
    {
        if (!dir.mkpath(fileInfo.absolutePath()))
        {
            return "";
        }
    }

    if (fileInfo.isDir())
    {
        // Per si de cas al setting s'ha definit un directori.
        userPath += "customwindowlevels.xml";
    }

    return userPath;
}

}
