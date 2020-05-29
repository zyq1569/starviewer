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

#include "qthumbnailspreviewwidget.h"

#include <QString>
#include <QIcon>

namespace udg {

const int QThumbnailsPreviewWidget::scaledSeriesSizeX = 100;
const int QThumbnailsPreviewWidget::scaledSeriesSizeY = 100;

QThumbnailsPreviewWidget::QThumbnailsPreviewWidget(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);
    QSize size;

    // Definim la mida de la imatge que mostrem
    size.setHeight(scaledSeriesSizeY);
    size.setWidth(scaledSeriesSizeX);
    m_thumbnailsPreviewWidget->setIconSize(size);

    createConnections();
}

void QThumbnailsPreviewWidget::createConnections()
{
    connect(m_thumbnailsPreviewWidget, SIGNAL(itemClicked (QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));
    connect(m_thumbnailsPreviewWidget, SIGNAL(itemDoubleClicked (QListWidgetItem*)), SLOT(itemDoubleClicked(QListWidgetItem*)));
}

void QThumbnailsPreviewWidget::append(QString IDThumbnail, const QPixmap &thumbnail, QString thumbnailDescription)
{
    m_thumbnailsPreviewWidget->addItem(createQListWidgetItem(IDThumbnail, thumbnail, thumbnailDescription));
}

void QThumbnailsPreviewWidget::insert(int position, QString IDThumbnail, const QPixmap &thumbnail, QString thumbnailDescription)
{
    m_thumbnailsPreviewWidget->insertItem(position, createQListWidgetItem(IDThumbnail, thumbnail, thumbnailDescription));
}

void QThumbnailsPreviewWidget::remove(QString IDThumbnail)
{
    QListWidgetItem *itemToDelete = getQListWidgetItem(IDThumbnail);

    if (itemToDelete)
    {
        delete itemToDelete;
    }
}

void QThumbnailsPreviewWidget::setCurrentThumbnail(QString IDThumbnail)
{
    m_thumbnailsPreviewWidget->setCurrentItem(getQListWidgetItem(IDThumbnail));
}

QStringList QThumbnailsPreviewWidget::getSelectedThumbnailsID()
{
    QStringList selectedThumbnailsID;

    foreach (QListWidgetItem *selectedItem, m_thumbnailsPreviewWidget->selectedItems())
    {
        selectedThumbnailsID.append(selectedItem->statusTip());
    }

    return selectedThumbnailsID;
}

void QThumbnailsPreviewWidget::setSelectionMode(QAbstractItemView::SelectionMode selectionMode)
{
    m_thumbnailsPreviewWidget->setSelectionMode(selectionMode);
}

QAbstractItemView::SelectionMode QThumbnailsPreviewWidget::getSelectionMode()
{
    return m_thumbnailsPreviewWidget->selectionMode();
}

void QThumbnailsPreviewWidget::itemClicked(QListWidgetItem *item)
{
    if (item != NULL)
    {
        emit(thumbnailClicked(item->statusTip()));
    }
}

void QThumbnailsPreviewWidget::itemDoubleClicked(QListWidgetItem *item)
{
    if (item != NULL)
    {
        emit(thumbnailDoubleClicked(item->statusTip()));
    }
}

void QThumbnailsPreviewWidget::clear()
{
    m_thumbnailsPreviewWidget->clear();
}

QListWidgetItem* QThumbnailsPreviewWidget::createQListWidgetItem(QString IDThumbnail, const QPixmap &thumbnail, QString thumbnailDescription)
{
    QListWidgetItem *item = new QListWidgetItem();

    item->setText(thumbnailDescription);
    item->setIcon(QIcon(thumbnail));
    item->setToolTip(thumbnailDescription);
    item->setStatusTip(IDThumbnail);

    return item;
}

QListWidgetItem* QThumbnailsPreviewWidget::getQListWidgetItem(QString IDThumbnail)
{
    QListWidgetItem *itemToReturn = NULL;

    foreach(QListWidgetItem *item, m_thumbnailsPreviewWidget->findItems("", Qt::MatchContains))
    {
        if (item->statusTip() == IDThumbnail)
        {
            itemToReturn = item;
            break;
        }
    }

    return itemToReturn;
}

}
