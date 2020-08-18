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

    /// Afegeix una tool al registre intern. Aquesta tool escoltarà els events
    /// que se li comuniquin mitjançant forward event
    /// @param tool
    void addTool(Tool *tool);

    /// Donat un nom de tool, l'elimina del registre.
    /// @param toolName Nom de la tool a eliminar del proxy
    /// @return Cert si la tenia, fals altrament
    bool removeTool(const QString &toolName);

    /// Buida i elimina totes les tools del proxy
    void removeAllTools();

    /// Ens diu si la tool demanada està activa o no
    /// @param toolName Nom de la tool que volem consultar
    /// @return Cert si la tool està activa, fals altrament
    bool isToolActive(const QString &toolName);

    /// Ens retorna la tool demanada
    /// @param toolName Nom de la tool que volem obtenir
    /// @return La tool demanada, NUL si no està registrada en el proxy
    Tool* getTool(const QString &toolName);

public slots:
    /// Avalua l'event que ha rebut del visualitzador al que està associat i l'envia a les tools actives
    void forwardEvent(unsigned long eventID);

private:
     /// Map on hi guardem les tools associades a una clau determinada pel nom de la tool.
     /// En principi no es poden tenir dues tools amb el mateix nom
    QMap<QString, Tool*> m_toolsMap;

    /// En aquest repositori ens encarregarem de guardar les dades persistents de les tools que ho necessitin
    QMap<QString, ToolData*> m_persistentToolDataRepository;

};

}

#endif
