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

#ifndef UDGTOOLPROXY_H
#define UDGTOOLPROXY_H

#include <QObject>
#include <QMap>

// Forward declarations
class QString;

namespace udg {

// Forward declarations space udg
class Tool;
class ToolData;

/**
    Classe encarregada de mantenir les tools actives per un visualitzador i de proporcionar-lis els events corresponents.
    Les seves tasques són:
        - Rep tots els events d'un viewer
        - Coneix les tools actives per a un determinat viewer
        - Passa els events rebuts a totes les tools actives
        - Està associat a un viewer com atribut d'aquest
        - Guarda el nom de cadascuna de les tools
  */
class ToolProxy : public QObject {
    Q_OBJECT
public:
    ToolProxy(QObject *parent = 0);
    ~ToolProxy();

    /// Add a tool to the internal registry. This tool will listen to the events
    /// to be communicated to you by forward event
    /// @param tool
    void addTool(Tool *tool);

    /// Given a tool name, remove it from the registry.
    /// @param toolName Name of the tool to remove from the proxy
    /// @return True if I had it, false otherwise
    bool removeTool(const QString &toolName);

    /// Empty and remove all proxy tools
    void removeAllTools();

    /// Tells us if the requested tool is active or not
    /// @param toolName Name of the tool we want to consult
    /// @return True if the tool is active, false otherwise
    bool isToolActive(const QString &toolName);

    /// Returns the requested tool
    /// @param toolName Name of the tool we want to get
    /// @return The requested tool, NUL if it is not registered in the proxy
    Tool* getTool(const QString &toolName);

public slots:
    /// Evaluates the event it has received from the viewer it is associated with and sends it to the active tools
    void forwardEvent(unsigned long eventID);

private:
    /// Map where we store the tools associated with a key determined by the name of the tool.
    /// In principle you can't have two tools with the same name
    QMap<QString, Tool*> m_toolsMap;

    /// In this repository we will take care of saving the persistent data of the tools that need it
    QMap<QString, ToolData*> m_persistentToolDataRepository;

};

}

#endif
