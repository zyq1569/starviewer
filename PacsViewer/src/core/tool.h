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

#ifndef UDGTOOL_H
#define UDGTOOL_H

#include <QObject>

namespace udg {

class ToolConfiguration;
class ToolData;
class QViewer;
class Q2DViewer;

/**
Base class for all tools
*/
class Tool : public QObject {
    Q_OBJECT
public:
    Tool(QViewer *viewer, QObject *parent = 0);
    ~Tool();

    /// We assign a configuration, if the one you have by default is not valid for us
    /// @param configuration Configuration we want to assign to it
    virtual void setConfiguration(ToolConfiguration *configuration);

    /// Returns the current tool configuration
    /// EVERYTHING should be const or not?
    ToolConfiguration* getConfiguration() const;

    /// Assign the data
    /// @param data data that we assign to the tool
    virtual void setToolData(ToolData *data);

    /// Returns the tool data to us
    /// @return Tool data
    /// EVERYTHING should be const or not?
    virtual ToolData* getToolData() const;

    /// Determines whether this tool has shared data or not
    /// @return True if you have shared data, otherwise false
    /// EVERYTHING we can implement this method in two ways.
    /// 1.- Each class defines with the variable m_hasSharedData whether it has shared data or not
    /// 2.- We define this virtual method and each class re-implements it returning the value we want
    /// Method 1 seems better because you don't have to maintain as many lines of code, although it's easier to have
    /// problems because it is easier to give values ​​to the variable by mistake or not to initialize it correctly
    bool hasSharedData() const;

    /// Determines whether the tool has persistent data or not
    bool hasPersistentData() const;

    /// Returns the name of the tool
    QString toolName();

    /// Decide what to do for each event received
    virtual void handleEvent(unsigned long eventID) = 0;

protected:
    /// Viewer on which the tool runs
    QViewer *m_viewer;

    /// Tool configuration
    ToolConfiguration *m_toolConfiguration;

    /// Tool data
    ToolData *m_toolData;

    /// Indicates whether or not your data should be shared
    bool m_hasSharedData;

    /// Tool name Can we all use QMetaObject :: className ()? and do we save this variable?
    QString m_toolName;

    /// Indicates whether the tool has persistent data
    bool m_hasPersistentData;
};

}

#endif
