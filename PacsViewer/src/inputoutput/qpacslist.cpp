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

#include "qpacslist.h"

#include <QTreeView>
#include <QList>

#include "pacsdevicemanager.h"
#include "pacsdevice.h"

namespace udg {

QPacsList::QPacsList(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    // The PacsId column is hidden
    m_PacsTreeView->setColumnHidden(0, true);
    // The PACSAddress column is hidden
    m_PacsTreeView->setColumnHidden(4, true);
    // Sort by the AETitle column
    m_PacsTreeView->sortByColumn(1, Qt::AscendingOrder);

    m_filterPacsByService = PacsDeviceManager::PacsWithQueryRetrieveServiceEnabled;
    m_showQueryPacsDefaultHighlighted = true;

    connect(m_PacsTreeView, SIGNAL(itemSelectionChanged()), SIGNAL(pacsSelectionChanged()));

    refresh();
}

QPacsList::~QPacsList()
{
}

void QPacsList::refresh()
{
    PacsDeviceManager pacsDeviceManager;
    QList<PacsDevice> pacsList;

    m_PacsTreeView->clear();

    pacsList = pacsDeviceManager.getPACSList(m_filterPacsByService);

    foreach (PacsDevice pacs, pacsList)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_PacsTreeView);

        item->setText(0, pacs.getID());
        item->setText(1, pacs.getAETitle());
        item->setText(2, pacs.getInstitution());
        item->setText(3, pacs.getDescription());
        item->setText(4, pacs.getAddress());

        if (getShowQueryPacsDefaultHighlighted())
        {
            item->setSelected(pacs.isDefault());
        }
    }
}

QList<PacsDevice> QPacsList::getSelectedPacs()
{
    PacsDeviceManager pacsDeviceManager;
    QList<PacsDevice> selectedPacsList;
    QList<QTreeWidgetItem*> qPacsList(m_PacsTreeView->selectedItems());

    QTreeWidgetItem *item;
    for (int i = 0; i < qPacsList.count(); i++)
    {
        PacsDevice pacs;
        item = qPacsList.at(i);
        // We do the query to look up the PACS information
        pacs = pacsDeviceManager.getPACSDeviceByID(item->text(0));
        // Insert in the list
        selectedPacsList.append(pacs);
    }

    return selectedPacsList;
}

void QPacsList::clearSelection()
{
    m_PacsTreeView->clearSelection();
}

void QPacsList::setFilterPACSByService(PacsDeviceManager::FilterPACSByService filter)
{
    m_filterPacsByService = filter;
}

PacsDeviceManager::FilterPACSByService QPacsList::getFilterPACSByService()
{
    return m_filterPacsByService;
}

void QPacsList::setShowQueryPacsDefaultHighlighted(bool showHighlighted)
{
    m_showQueryPacsDefaultHighlighted = showHighlighted;
}

bool QPacsList::getShowQueryPacsDefaultHighlighted()
{
    return m_showQueryPacsDefaultHighlighted;
}

void QPacsList::setDefaultPACS(QTreeWidgetItem *item)
{
    Q_ASSERT(item);

    PacsDeviceManager pacsDeviceManager;

    PacsDevice pacs;
    pacs = pacsDeviceManager.getPACSDeviceByID(item->text(0));
    pacs.setDefault(item->isSelected());
}

};
