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

#ifndef UDGEXTENSIONWORKSPACE_H
#define UDGEXTENSIONWORKSPACE_H

#include <QTabWidget>
#include <QMap>

// Forward declarations
class QAction;
class QToolButton;

namespace udg {

class ExtensionWorkspace : public QTabWidget {
    Q_OBJECT
public:
    ExtensionWorkspace(QWidget *parent = 0);

    ~ExtensionWorkspace();

    /// Add a new application
    /// We indicate the widget, title visible to the user and internal name of the application
    // \ TODO: resource parameter ?, add an application id to be able to reference it,
    /// add a label / name, + other possible parameters
    void addApplication(QWidget *application, QString caption, const QString &extensionIdentifier);

    ///Remove an application from the mini-application space and delete it from memory
    void removeApplication(QWidget *application);

    ///Completely remove all extensions in the workspace
    void killThemAll();

    ///It returns the map that contains the open extensions associated with its name
    QMap<QWidget *, QString> getActiveExtensions() const;

public slots:
    /// To close the application that is currently open
    void closeCurrentApplication();

private:
    /// Creates signal and slot connections
    void createConnections();

    /// Specifies whether or not to use the dark background color
    /// Useful so that the user does not dazzle when being in dark rooms and the application appears for the first time
    /// if you have an OS theme with light colors (a whole window comes out with 2 white diagnostic screens)
    void setDarkBackgroundColorEnabled(bool value);

private slots:
    /// Close the application with "index". Designed to connect to the tab close signal
    void closeApplicationByTabIndex(int index);

private:
    ///Map containing the extensions we have open, associated with their name
    QMap<QWidget *, QString> m_activeExtensions;
};

};  // end namespace udg

#endif
