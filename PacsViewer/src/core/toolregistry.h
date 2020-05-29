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

#ifndef UDGTOOLREGISTRY_H
#define UDGTOOLREGISTRY_H

#include <QObject>
#include <QMap>
#include <QPair>

class QAction;

namespace udg {

class Tool;
class QViewer;

/**
    Registre de Tools i elements associats
  */
class ToolRegistry : public QObject {
Q_OBJECT
public:
    ToolRegistry(QObject *parent = 0);
    ~ToolRegistry();

    /// Ens crea la tool demanada i li assigna el viewer donat
    /// @param toolName
    /// @param viewer
    Tool* getTool(const QString &toolName, QViewer *viewer);

    /// Crea una acció vàlida per aquella tool
    /// @param toolName El nom de la tool de la qual volem l'acció
    /// @return L'acció de la tool demanada, nul si la tool no existeix TODO o millor una QAction buida?
    QAction* getToolAction(const QString &toolName);

    /// Crea un parell amb l'acció de la "Action Tool" demanada més l'SLOT associat
    /// @param actionToolName Nom de la "Action Tool" de la qual volem obtenir la parella
    /// @return un QPair amb l'acció creada i l'string de l'SLOT a cridar. Si la "Action
    ///  Tool" demanada no està registrada, retornarà un QPair amb un objecte QAction i QString buits (no NULS!)
    QPair<QAction*, QString> getActionToolPair(const QString &actionToolName);
};

}

#endif
