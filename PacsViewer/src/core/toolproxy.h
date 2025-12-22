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
Class responsible for keeping the tools active for a viewer and providing them with the corresponding events.
Their tasks are:
- Receives all events from a viewer
- Know the active tools for a given viewer
- Pass the received events to all active tools
- It is associated with a viewer as an attribute of this
- Save the name of each of the tools
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

    ///Empty and remove all proxy tools
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

    ///In this repository we will take care of saving the persistent data of the tools that need it
    QMap<QString, ToolData*> m_persistentToolDataRepository;

};

}

#endif
