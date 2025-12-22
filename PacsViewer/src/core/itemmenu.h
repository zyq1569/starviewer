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

#ifndef UDGITEMMENU_H
#define UDGITEMMENU_H

#include <QFrame>

// FWD declarations
class QVariant;
class QEvent;

namespace udg {

/**
Menu item. It sends a signal when it is activated and when it is selected.
As well as changes color when activated. You can save QVariant information.
You can also choose the behavior of the color change.
*/
class ItemMenu : public QFrame {
    Q_OBJECT
public:
    ItemMenu(QWidget *parent = 0);

    ~ItemMenu();

    ///Put a dice
    void setData(QString data);

    /// Get the data
    QString getData();

    /// Method to set the boxes to be selected or deselected when the mouse is left
    void setFixed(bool option);

    /// Method for setting the item as selected or unselected
    void setSelected(bool option);

    /// Returns true if this item has the border enabled and false otherwise.
    bool hasBorder() const;
    /// Enables or disables the border of this item according to the given value.
    void setBorder(bool on);

signals:
    ///Signal emitted when the mouse enters the widget
    void isActive(ItemMenu *);

    ///Signal emitted when selecting the item
    void isSelected(ItemMenu *);

protected:
    /// Overloading the method that handles all events
    bool event(QEvent *event);

protected:
    /// Data that the widget can save
    QString m_data;

    /// Attribute that informs whether to stay selected or deselected
    bool m_fixed;

    /// True if the item is selected and false otherwise.
    bool m_selected;

    /// True if this item has the border enabled and false otherwise.
    bool m_border;

};

}

#endif
