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
// Pel delete
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
    // TODO De moment ToolRegistry és una classe normal, però si es passa a singleton
    // El "new" s'haurà de canviar per un ::instance()
    m_toolRegistry = new ToolRegistry(this);
    m_toolsActionSignalMapper = new QSignalMapper(this);
    connect(m_toolsActionSignalMapper, SIGNAL(mapped(const QString&)), SLOT(triggeredToolAction(const QString&)));
}

ToolManager::~ToolManager()
{
}

void ToolManager::setViewerTools(QViewer *viewer, const QStringList &toolsList)
{
    // Cada cop que eliminem un viewer que estem gestionant, l'haurem de "desregistrar" del ToolManager
    connect(viewer, SIGNAL(destroyed(QObject*)), SLOT(unregisterViewer(QObject*)));
    ViewerToolConfigurationPairType pair;
    pair.first = viewer;
    pair.second = NULL;
    foreach (const QString &toolName, toolsList)
    {
        m_toolViewerMap.insert(toolName, pair);
    }
    // TODO Xapussilla, s'hauria de fer amb tractament més bo intern, amb llistes de tools actives
    refreshConnections();
}

void ToolManager::setupRegisteredTools(QViewer *viewer)
{
    setViewerTools(viewer, getRegisteredToolsList());
}

void ToolManager::setViewerTool(QViewer *viewer, const QString &toolName, ToolConfiguration *configuration)
{
    // Cada cop que eliminem un viewer que estem gestionant, l'haurem de "desregistrar" del ToolManager
    connect(viewer, SIGNAL(destroyed(QObject*)), SLOT(unregisterViewer(QObject*)));
    ViewerToolConfigurationPairType pair;
    pair.first = viewer;
    pair.second = configuration;
    m_toolViewerMap.insert(toolName, pair);
    // TODO Xapussilla, s'hauria de fer amb tractament més bo intern, amb llistes de tools actives
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
                // Eliminem la tool del proxy
                viewer->getToolProxy()->removeTool(toolName);
                // Això vol dir que per un nom de tool, nomé spodem tenir un parell viewer-config
                // del contrari, ens hauríem de "patejar" tot el map sencer per si tenim la mateixa tool,
                // pel mateix viewer i amb una configuració diferent
                if (pair.second)
                {
                    // Eliminem la configuració
                    delete pair.second;
                }
                // Eliminem l'element del mapa
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
        // Si el grup ja existeix, no cal crear l'entrada, simplement modificar-la
        actionGroup = m_toolsGroups.take(groupName);
    }
    else
    {
        actionGroup = new QActionGroup(this);
        actionGroup->setExclusive(true);
    }
    // Afegim el grup al mapa
    m_toolsGroups.insert(groupName, actionGroup);
    // Per cada tool obtindrem la seva acció i la farem entrar en el grup d'exclusivitat
    foreach (const QString &toolName, tools)
    {
        // Obtenim l'acció corresponent a aquella tool
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
    // Guarrada! TODO Aixo es un workaround per poder desactivar "automaticament" les tools dins d'un mateix grup
    // Lo correcte seria plantejar be el tema dels signals mappers o fer una implementacio propia mes elaborada
    connect(actionGroup, SIGNAL(triggered(QAction*)), SLOT(refreshConnections()));
}

QAction* ToolManager::registerActionTool(const QString &actionToolName)
{
    QPair<QAction*, QString> pair;
    // Si no està registrada la obtenim del registre i l'afegim al nostre map
    if (!m_actionToolRegistry.contains(actionToolName))
    {
        pair = m_toolRegistry->getActionToolPair(actionToolName);
        m_actionToolRegistry.insert(actionToolName, pair);
    }
    // Sinó, l'agafem del nostre map i no tornem a crear l'acció
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
    // TODO Caldria comprovar si la tool es troba en un grup exclusiu per "fer fora" les altres tools
    // en el cas que prescindíssim del mecanisme que fem servir amb QActionToolGroup
    QList<ViewerToolConfigurationPairType> viewerConfigList = m_toolViewerMap.values(toolName);

    ToolData *data = m_sharedToolDataRepository.value(toolName);
    // Declarem aquestes variables per fer-ho més llegible
    QViewer *viewer;
    ToolConfiguration *configuration;
    foreach (const ViewerToolConfigurationPairType &pair, viewerConfigList)
    {
        viewer = pair.first;
        configuration = pair.second;
        Tool *tool = 0;

        // Hem de comprovar si el proxy ja té o no la tool
        if (!viewer->getToolProxy()->isToolActive(toolName))
        {
            // Com que el proxy no té aquesta tool
            // la produim i la posem a punt amb les dades i la configuració
            tool = m_toolRegistry->getTool(toolName, viewer);
            // Si no tenim cap configuració guardada, no cal fer res, es queda amb la que té per defecte
            if (configuration)
            {
                tool->setConfiguration(configuration);
            }
            // Afegim la tool al proxy
            viewer->getToolProxy()->addTool(tool);
            // Comprovem les dades per si cal donar-n'hi
            if (tool->hasSharedData())
            {
                // No hi són al repositori, les obtindrem de la pròpia tool i les registrarem al repositori
                if (!data)
                {
                    data = tool->getToolData();
                    m_sharedToolDataRepository[toolName] = data;
                }
                else
                {
                    // Si ja les hem creat abans, li assignem les de la primera tool creada
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
        // Declarem aquesta variable per fer-ho més llegible
        QViewer *viewer = pair.first;
        // Eliminem la tool del proxy
        viewer->getToolProxy()->removeTool(toolName);
    }
    // Elinimen Shared Data d'aquesta tool
    m_sharedToolDataRepository.remove(toolName);
}

void ToolManager::triggeredToolAction(const QString &toolName)
{
    // TODO Cal repassar tot això. Hauria d'anar amb llistes internes de tools activades/desactivades
    // obtenim l'acció que l'ha provocat
    QAction *toolAction = getRegisteredToolAction(toolName);
    if (toolAction)
    {
        // Si està checked és que s'ha d'activar, altrament desactivar
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
        DEBUG_LOG(QString("No hi ha cap tool Action per la tool anomenada: ") + toolName);
    }
}

QAction* ToolManager::getRegisteredToolAction(const QString &toolName)
{
    QAction *toolAction = 0;
    if (m_toolsActionsRegistry.contains(toolName))
    {
        // Si ja existeix l'obtenim del registre de tools/acció
        toolAction = m_toolsActionsRegistry.value(toolName);
    }
    return toolAction;
}

QAction* ToolManager::registerTool(const QString &toolName)
{
    QAction *toolAction;
    if (m_toolsActionsRegistry.contains(toolName))
    {
        // Si ja existeix l'obtenim del registre de tools/acció
        toolAction = m_toolsActionsRegistry.value(toolName);
    }
    else
    {
        // Altrament, creem l'acció associada, la connectem amb l'estructura interna i la registrem
        toolAction = m_toolRegistry->getToolAction(toolName);
        m_toolsActionSignalMapper->setMapping(toolAction, toolName);
        connect(toolAction, SIGNAL(triggered()), m_toolsActionSignalMapper, SLOT(map()));
        m_toolsActionsRegistry.insert(toolName, toolAction);
    }
    // Retornem l'acció associada
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
    // Recorrem tot el mapa, eliminant totes les entrades on aparegui el viewer en qüestió
    QMutableMapIterator<QString, ViewerToolConfigurationPairType> mapIterator(m_toolViewerMap);
    while (mapIterator.hasNext())
    {
        mapIterator.next();
        ViewerToolConfigurationPairType pair = mapIterator.value();
        if (pair.first == viewer)
        {
            // Eliminem l'element del map
            mapIterator.remove();
        }
    }
}

}
