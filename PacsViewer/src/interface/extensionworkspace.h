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

    /// Afegeix una nova aplicació
    /// Li indiquem el widget, títol visible per l'usuari i nom intern de l'aplicació
    // \TODO: paràmetre de recursos?, afegir un id d'aplicació per poder-la referenciar, afegir una label/nom, +altres possibles paràmetres
    void addApplication(QWidget *application, QString caption, const QString &extensionIdentifier);

    /// Treu una aplicació de l'espai de mini-aplicacions i l'elimina de memòria
    void removeApplication(QWidget *application);

    /// Elimina per complet totes les extensions que hi hagi al workspace
    void killThemAll();

    /// Ens retorna el mapa que conté les extensions obertes associades al seu nom
    QMap<QWidget *, QString> getActiveExtensions() const;

public slots:
    /// Per tancar l'aplicació que està oberta en aquell moment
    void closeCurrentApplication();

private:
    /// Crea les connexions de signals i slots
    void createConnections();

    /// Especifica si es fa servir o no el color de fons fosc
    /// Útil perquè l'usuari no s'enlluerni al estar en habitacions fosques i aparegui l'aplicació per primera vegada
    /// si es té un theme del SO amb colors clars (surt tota una finestra amb 2 pantalles de diagnòstic blanques)
    void setDarkBackgroundColorEnabled(bool value);

private slots:
    /// Tanca la aplicació amb índex "index". Pensat per connectar-ho al signal de tancar pestanya
    void closeApplicationByTabIndex(int index);

private:
    /// Mapa que conté les extensions que tenim obertes, associades al seu nom
    QMap<QWidget *, QString> m_activeExtensions;
};

};  // end namespace udg

#endif
