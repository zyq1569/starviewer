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

#ifndef UDGQPACSLIST_H
#define UDGQPACSLIST_H

#include "ui_qpacslistbase.h"
#include <QWidget>
#include <QList>
#include "pacsdevicemanager.h"

namespace udg {

class Status;
class PacsDevice;

/**
Interface showing the PACS to which the application can connect,
allows you to select which ones the user wants to connect
*/
class QPacsList : public QWidget, private Ui::QPacsListBase {
    Q_OBJECT
public:
    /// Class builder
    QPacsList(QWidget *parent = 0);

    /// Returns user-selected packs
    QList<PacsDevice> getSelectedPacs();

    ///Clears selected items from the list
    void clearSelection();

    /// Class Destroyer
    ~QPacsList();

    /// Specifies / Returns by which Filter the displayed PACS are filtered,
    /// by default if not specified is not filtered.
    void setFilterPACSByService(PacsDeviceManager::FilterPACSByService);
    PacsDeviceManager::FilterPACSByService getFilterPACSByService();

    /// Specifies / Returns whether PACS that are marked as
    /// PACS to search by default should be highlighted. By default they are displayed
    void setShowQueryPacsDefaultHighlighted(bool showHighlighted);
    bool getShowQueryPacsDefaultHighlighted();

public slots:
    /// Upload the List of Available Packs to ListView
    void refresh();

signals:
    ///Emitted when a PACS is selected or deselected.
    void pacsSelectionChanged();

private slots:
    /// Makes the PACS in the Tree Widget item one of the default PACS
    /// if this item is selected by the user
    void setDefaultPACS(QTreeWidgetItem *item);

private:
    PacsDeviceManager::FilterPACSByService m_filterPacsByService;
    bool m_showQueryPacsDefaultHighlighted;
};

};

#endif
