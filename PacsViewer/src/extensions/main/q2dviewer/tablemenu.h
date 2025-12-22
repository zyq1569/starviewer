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

#ifndef UDGTABLEMENU_H
#define UDGTABLEMENU_H

#include <QWidget>

// FWD declarations
class QGridLayout;
class QLabel;
class QEvent;
class QMouseEvent;

namespace udg {

// FWD declarations
class ItemMenu;

/**
    Class to implement the menu to select the grids as if creating a table.
  */
class TableMenu : public QWidget {
    Q_OBJECT
public:
    TableMenu(QWidget *parent = 0);

    ~TableMenu();

    ///Initializes the table with a single unselected item
    void initializeTable();

    /// Empty all table items
    void dropTable();

public slots:
    ///Method that every time the mouse is placed on items checks if it is necessary to add rows or columns
    void verifySelected(ItemMenu *selected);

    /// Method that each time one of the items is selected emits the resulting grid
    void emitSelected(ItemMenu *selected);

signals:
    /// It emits that a grid has been chosen
    void selectedGrid(int, int);

protected:
    /// Overloading the method that handles all events
    bool event(QEvent *event);

    /// Overloading the method that handles mouse events
    void mouseMoveEvent (QMouseEvent *event);

private:
    ///Add a column to the table
    void addColumn();

    ///Add a row to the table
    void addRow();

private:
    /// Number of current columns
    int m_columns;

    ///Number of current rows
    int m_rows;

    ///Grid to display table items
    QGridLayout *m_gridLayout;

    ///Label that reports the selected rows and columns
    QLabel *m_information;

    /// List of items
    QList<ItemMenu*> m_itemList;

};

}

#endif
