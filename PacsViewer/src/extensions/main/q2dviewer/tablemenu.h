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
    Classe per implementar el menu per seleccionar els grids com si es crees una taula.
  */
class TableMenu : public QWidget {
Q_OBJECT
public:
    TableMenu(QWidget *parent = 0);

    ~TableMenu();

    /// Inicialitza la taula amb un únic element no seleccionat
    void initializeTable();

    /// Buida tots els elements de la taula
    void dropTable();

public slots:
    /// Mètode que cada vegada que el mouse es situi sobre items comprova si cal afegir files o columnes
    void verifySelected(ItemMenu *selected);

    /// Mètode que cada vegada que es seleccioni un dels items emet el grid resultat
    void emitSelected(ItemMenu *selected);

signals:
    /// Emet que s'ha escollit un grid
    void selectedGrid(int, int);

protected:
    /// Sobrecàrrega del mètode que tracta tots els events
    bool event(QEvent *event);

    /// Sobrecàrrega del mètode que tracta els events del mouse
    void mouseMoveEvent (QMouseEvent *event);

private:
    /// Afegir una columna a la taula
    void addColumn();

    /// Afegir una fila a la taula
    void addRow();

private:
    /// Nombre de columnes actuals
    int m_columns;

    /// Nombre de files actuals
    int m_rows;

    /// Grid per mostrar els elements de la taula
    QGridLayout *m_gridLayout;

    /// Label que informa de les files i columnes seleccionades
    QLabel *m_information;

    /// Llista dels items
    QList<ItemMenu*> m_itemList;

};

}

#endif
