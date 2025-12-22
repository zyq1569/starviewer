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
        //If the requested tool exists, we check if it has persistent data
        if (tool->hasPersistentData())
        {
            //Let's see if we have them in the repository
            ToolData *persistentData = m_persistentToolDataRepository.value(tool->toolName());
            if (persistentData)
            {
                // They are there, so we assign them to the tool
                tool->setToolData(persistentData);
            }
            else
            {
                // They are not in the repository, so this is the first time they have asked for the tool
                // We obtain your data and register it in the repository
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
        //We remove it from the map and free its memory
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
    /// You can't do a foreach on a map because it
    /// returns a pair of elements, so we pass all the elements of the map to a QList.
    QList<Tool*> toolsList = m_toolsMap.values();

    foreach (Tool *tool, toolsList)
    {
        tool->handleEvent(eventID);
    }
}

}
