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

#include "toolmanager.h"
#include "toolregistry.h"
#include "tooldata.h"
#include "toolproxy.h"
//For delete
#include "toolconfiguration.h"
#include "tool.h"
#include "qviewer.h"
#include "logging.h"

#include <QStringList>
#include <QSignalMapper>
#include <QAction>
#include <QActionGroup>

namespace udg {

ToolManager::ToolManager(QObject *parent)
    : QObject(parent)
{
    // TODO For now ToolRegistry is a normal class, but if you switch to singleton
    // The "new" must be changed to an :: instance ()
    m_toolRegistry = new ToolRegistry(this);
    m_toolsActionSignalMapper = new QSignalMapper(this);
    connect(m_toolsActionSignalMapper, SIGNAL(mapped(const QString&)), SLOT(triggeredToolAction(const QString&)));
}

ToolManager::~ToolManager()
{
}

void ToolManager::setViewerTools(QViewer *viewer, const QStringList &toolsList)
{
    /// Every time we remove a viewer we are managing, we will have to "unregister" it from the ToolManager
    connect(viewer, SIGNAL(destroyed(QObject*)), SLOT(unregisterViewer(QObject*)));
    ViewerToolConfigurationPairType pair;
    pair.first = viewer;
    pair.second = NULL;
    foreach (const QString &toolName, toolsList)
    {
        m_toolViewerMap.insert(toolName, pair);
    }
    // TODO Txapussilla, should be done with better internal treatment, with active tool lists
    refreshConnections();
}

void ToolManager::setupRegisteredTools(QViewer *viewer)
{
    setViewerTools(viewer, getRegisteredToolsList());
}

void ToolManager::setViewerTool(QViewer *viewer, const QString &toolName, ToolConfiguration *configuration)
{
    // Every time we remove a viewer we are managing, we will have to "unregister" it from the ToolManager
    connect(viewer, SIGNAL(destroyed(QObject*)), SLOT(unregisterViewer(QObject*)));
    ViewerToolConfigurationPairType pair;
    pair.first = viewer;
    pair.second = configuration;
    m_toolViewerMap.insert(toolName, pair);
    // TODO -Txapussilla, should be done with better internal treatment, with active tool lists
    refreshConnections();
}

void ToolManager::removeViewerTool(QViewer *viewer, const QString &toolName)
{
    QMutableMapIterator<QString, ViewerToolConfigurationPairType> mapIterator(m_toolViewerMap);
    bool found = false;
    while (mapIterator.hasNext() && !found)
    {
        mapIterator.next();
        if (mapIterator.key() == toolName)
        {
            ViewerToolConfigurationPairType pair = mapIterator.value();
            if (pair.first == viewer)
            {
                found = true;
                //We remove the proxy tool
                viewer->getToolProxy()->removeTool(toolName);
                // This means that for a tool name, we can only have a viewer-config pair
                // otherwise we would have to "kick" the whole map if we have the same tool,
                // for the same viewer and with a different configuration
                if (pair.second)
                {
                    // We remove the settings
                    delete pair.second;
                }
                // We remove the map item
                mapIterator.remove();
            }
        }
    }
}

void ToolManager::addExclusiveToolsGroup(const QString &groupName, const QStringList &tools)
{
    QActionGroup *actionGroup = 0;
    if (m_toolsGroups.find(groupName) != m_toolsGroups.end())
    {
        //If the group already exists, you do not need to create the entry, just modify it
        actionGroup = m_toolsGroups.take(groupName);
    }
    else
    {
        actionGroup = new QActionGroup(this);
        actionGroup->setExclusive(true);
    }
    // We add the group to the map
    m_toolsGroups.insert(groupName, actionGroup);
    //For each tool we will get its action and we will make it enter the exclusivity group
    foreach (const QString &toolName, tools)
    {
        // We get the action corresponding to that tool
        QAction *toolAction = getRegisteredToolAction(toolName);
        if (toolAction)
        {
            actionGroup->addAction(toolAction);
        }
        else
        {
            DEBUG_LOG(QString("No tenim registrada cap Action per la tool ") + toolName);
        }
    }
    // Guarrada! TODO This is a workaround to be able to "automatically" disable tools within the same group
    // The correct thing would be to raise the issue of signal mappers or make a more elaborate implementation
    connect(actionGroup, SIGNAL(triggered(QAction*)), SLOT(refreshConnections()));
}

QAction* ToolManager::registerActionTool(const QString &actionToolName)
{
    QPair<QAction*, QString> pair;
    //If it is not registered, we obtain it from the register and add it to our map
    if (!m_actionToolRegistry.contains(actionToolName))
    {
        pair = m_toolRegistry->getActionToolPair(actionToolName);
        m_actionToolRegistry.insert(actionToolName, pair);
    }
    // Otherwise, we take it from our map and do not recreate the action
    else
    {
        pair = m_actionToolRegistry.value(actionToolName);
    }

    return pair.first;
}

void ToolManager::triggerTools(const QStringList &toolsList)
{
    foreach (const QString &toolName, toolsList)
    {
        triggerTool(toolName);
    }
}

void ToolManager::triggerTool(const QString &toolName)
{
    if (m_toolsActionsRegistry.contains(toolName))
    {
        m_toolsActionsRegistry.value(toolName)->trigger();
    }
}

void ToolManager::enableActionTools(QViewer *viewer, const QStringList &actionToolsList)
{
    QPair<QAction*, QString> pair;
    foreach (const QString &actionToolName, actionToolsList)
    {
        pair = m_actionToolRegistry.value(actionToolName);
        connect(pair.first, SIGNAL(triggered()), viewer, qPrintable(pair.second));
    }
}

void ToolManager::disableActionTools(QViewer *viewer, const QStringList &actionToolsList)
{
    QPair<QAction*, QString> pair;
    foreach (const QString &actionToolName, actionToolsList)
    {
        pair = m_actionToolRegistry.value(actionToolName);
        disconnect(pair.first, SIGNAL(triggered()), viewer, 0);
    }
}

void ToolManager::enableRegisteredActionTools(QViewer *viewer)
{
    enableActionTools(viewer, m_actionToolRegistry.keys());
}

void ToolManager::disableRegisteredActionTools(QViewer *viewer)
{
    disableActionTools(viewer, m_actionToolRegistry.keys());
}

void ToolManager::disableAllToolsTemporarily()
{
    QStringList toolsList = m_toolViewerMap.uniqueKeys();
    foreach (const QString &toolName, toolsList)
    {
        deactivateTool(toolName);
    }
}

void ToolManager::undoDisableAllToolsTemporarily()
{
    refreshConnections();
}

void ToolManager::activateTool(const QString &toolName)
{
    //// It would be necessary to check if the tool is in an exclusive group to "take out" the other tools
    // in case we do without the mechanism we use with QActionToolGroup
    QList<ViewerToolConfigurationPairType> viewerConfigList = m_toolViewerMap.values(toolName);

    ToolData *data = m_sharedToolDataRepository.value(toolName);
    // We declare these variables to make it more readable
    QViewer *viewer;
    ToolConfiguration *configuration;
    foreach (const ViewerToolConfigurationPairType &pair, viewerConfigList)
    {
        viewer = pair.first;
        configuration = pair.second;
        Tool *tool = 0;

        //We need to check if the proxy already has the tool or not
        if (!viewer->getToolProxy()->isToolActive(toolName))
        {
            // Because the proxy does not have this tool
            // we produce it and fine-tune it with the data and configuration
            tool = m_toolRegistry->getTool(toolName, viewer);
            /// If we don't have any saved settings, nothing needs to be done, it stays with the default one
            if (configuration)
            {
                tool->setConfiguration(configuration);
            }
            //  We add the tool to the proxy
            viewer->getToolProxy()->addTool(tool);
            //We check the data in case it needs to be given
            if (tool->hasSharedData())
            {
                //They are not in the repository, we will get them from the tool itself and register them in the repository
                if (!data)
                {
                    data = tool->getToolData();
                    m_sharedToolDataRepository[toolName] = data;
                }
                else
                {
                    /// If we have already created them before, we assign them the ones from the first tool created
                    tool->setToolData(data);
                }
            }
        }
    }
}

void ToolManager::deactivateTool(const QString &toolName)
{
    QList<ViewerToolConfigurationPairType> viewerConfigList = m_toolViewerMap.values(toolName);

    foreach (const ViewerToolConfigurationPairType &pair, viewerConfigList)
    {
        // We declare this variable to make it more readable
        QViewer *viewer = pair.first;
        // Remove the proxy tool
        viewer->getToolProxy()->removeTool(toolName);
    }
    /// Remove Shared Data from this tool
    m_sharedToolDataRepository.remove(toolName);
}

void ToolManager::triggeredToolAction(const QString &toolName)
{
    // TODO All this needs to be reviewed. It should go with internal lists of tools enabled / disabled
    // we get the action that caused it
    QAction *toolAction = getRegisteredToolAction(toolName);
    if (toolAction)
    {
        // // If checked it must be activated, otherwise deactivated
        if (toolAction->isChecked())
        {
            activateTool(toolName);
        }
        else
        {
            deactivateTool(toolName);
        }
    }
    else
    {
        DEBUG_LOG(QString("There is no Action tool for the called tool: ") + toolName);
    }
}

QAction* ToolManager::getRegisteredToolAction(const QString &toolName)
{
    QAction *toolAction = 0;
    if (m_toolsActionsRegistry.contains(toolName))
    {
        // If it already exists we get it from the tools / action log
        toolAction = m_toolsActionsRegistry.value(toolName);
    }
    return toolAction;
}

QAction* ToolManager::registerTool(const QString &toolName)
{
    QAction *toolAction;
    if (m_toolsActionsRegistry.contains(toolName))
    {
        //If it already exists we get it from the tools / action register
        toolAction = m_toolsActionsRegistry.value(toolName);
    }
    else
    {
        /// Otherwise, we create the associated action, connect it to the internal structure, and record it
        toolAction = m_toolRegistry->getToolAction(toolName);
        m_toolsActionSignalMapper->setMapping(toolAction, toolName);
        connect(toolAction, SIGNAL(triggered()), m_toolsActionSignalMapper, SLOT(map()));
        m_toolsActionsRegistry.insert(toolName, toolAction);
    }
    //// Return the associated action
    return toolAction;
}

QStringList ToolManager::getRegisteredToolsList() const
{
    return m_toolsActionsRegistry.uniqueKeys();
}

void ToolManager::refreshConnections()
{
    QStringList toolsList = getRegisteredToolsList();
    foreach (const QString &tool, toolsList)
    {
        triggeredToolAction(tool);
    }
}

void ToolManager::unregisterViewer(QObject *viewer)
{
    /// We go through the whole map, removing all the entries where the viewer in question appears
    QMutableMapIterator<QString, ViewerToolConfigurationPairType> mapIterator(m_toolViewerMap);
    while (mapIterator.hasNext())
    {
        mapIterator.next();
        ViewerToolConfigurationPairType pair = mapIterator.value();
        if (pair.first == viewer)
        {
            /// We remove the map element
            mapIterator.remove();
        }
    }
}

}
