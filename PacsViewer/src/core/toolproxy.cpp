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

#include "toolproxy.h"
#include "tool.h"
#include "tooldata.h"
#include <QStringList>

namespace udg {

ToolProxy::ToolProxy(QObject *parent)
 : QObject(parent)
{
}

ToolProxy::~ToolProxy()
{
    removeAllTools();
    m_persistentToolDataRepository.clear();
}

void ToolProxy::addTool(Tool *tool)
{
    if (!m_toolsMap.contains(tool->toolName()))
    {
        m_toolsMap.insert(tool->toolName(), tool);
        // Si la tool demanada existeix, comprovem si té dades persistents
        if (tool->hasPersistentData())
        {
            // Mirem si les tenim al repositori
            ToolData *persistentData = m_persistentToolDataRepository.value(tool->toolName());
            if (persistentData)
            {
                // Hi són, per tant li assignem a la tool
                tool->setToolData(persistentData);
            }
            else
            {
                // No hi són al respositori, per tant és el primer cop que demanen la tool
                // Obtenim les seves dades i les registrem al repositori
                m_persistentToolDataRepository[tool->toolName()] = tool->getToolData();
            }
        }
    }
    else
    {
        delete tool;
    }
}

bool ToolProxy::removeTool(const QString &toolName)
{
    bool ok = false;
    if (m_toolsMap.contains(toolName))
    {
        // L'eliminem del mapa i alliberem la seva memòria
        Tool *tool = m_toolsMap.take(toolName);
        delete tool;
        ok = true;
    }
    return ok;
}

void ToolProxy::removeAllTools()
{
    QStringList toolsList = m_toolsMap.keys();
    foreach (const QString &toolName, toolsList)
    {
        Tool *tool = m_toolsMap.take(toolName);
        delete tool;
    }
}

bool ToolProxy::isToolActive(const QString &toolName)
{
    return m_toolsMap.contains(toolName);
}

Tool* ToolProxy::getTool(const QString &toolName)
{
    Tool *tool = 0;
    if (m_toolsMap.contains(toolName))
    {
        tool = m_toolsMap.value(toolName);
    }

    return tool;
}

void ToolProxy::forwardEvent(unsigned long eventID)
{
    // No es pot fer un foreach sobre un map perquè retorna parella d'elements, per això passem tots els elements del map a una QList.
    QList<Tool*> toolsList = m_toolsMap.values();

    foreach (Tool *tool, toolsList)
    {
        tool->handleEvent(eventID);
    }
}

}
