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

#ifndef UDGPATIENTBROWSERMENUEXTENDEDITEM_H
#define UDGPATIENTBROWSERMENUEXTENDEDITEM_H

#include <QWidget>
#include <QLabel>

namespace udg {

/**
Class to display additional information regarding a series of a patient study.
*/
class PatientBrowserMenuExtendedItem : public QFrame {
    Q_OBJECT
public:
    PatientBrowserMenuExtendedItem(QWidget *parent = 0);
    ~PatientBrowserMenuExtendedItem();

    void setPixmap(const QPixmap &pixmap);
    void setText(const QString &text);

private:
    /// Create the initial widget without any information
    void createInitialWidget();

private:
    /// Label containing the additional text to display
    QLabel *m_text;

    /// Label containing the icon to display
    QLabel *m_icon;
};

}

#endif
