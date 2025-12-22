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

#ifndef UDGPATIENTBROWSERMENULIST_H
#define UDGPATIENTBROWSERMENULIST_H

#include <QWidget>
#include <QKeyEvent>

class QGridLayout;
class QQuickWidget;

namespace udg {

class PatientBrowserMenuGroup;
class PatientBrowserMenuBasicItem;

/**
Widget that represents the contents of the menu in a list of items
These items will be organized in rows and columns within a group to which we will give a "caption"
*/
class PatientBrowserMenuList : public QWidget {
    Q_OBJECT

public:
    PatientBrowserMenuList(QWidget *parent = 0);
    ~PatientBrowserMenuList();

    /// Adds a group of items with the given caption
    void addItemsGroup(const QString &caption, const QList<QPair<QString, QString> > &itemsList);

    /// We mark as selected the item that has the given identifier
    void markItem(const QString &identifier);

    /// Returns the identifier of the item that we have marked as selected
    QString getMarkedItem() const;

    /// Treatment of events
    bool event(QEvent *event);

    /// Sets the maximum height of the widget
    void setMaximumHeight(int maxh);

    /// Sets the width of the widget
    void setMaximumWidth(int width);

    virtual QSize sizeHint() const;
signals:
    /// Signal that is emitted when the mouse enters the widget and notifies us of its identifier
    void isActive(QString);

    /// Signal that is emitted when no item is activated
    void isNotActive();

    /// This signal is emitted when an item is chosen and notifies us of its identifier
    void selectedItem(QString);

    /// Signal that is emitted when the menu closing event is received
    void closed();

private slots:
    void updateSize();

private:
    /// Item we have marked
    QString m_markedItem;

    /// List of item groups of the series
    QList<QObject*> m_groups;

    /// Patient browser menu list based on qml
    QQuickWidget *m_qmlView;
};

}

#endif
