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

#ifndef UDGQCONFIGURATIONDIALOG_H
#define UDGQCONFIGURATIONDIALOG_H

#include "ui_qconfigurationdialogbase.h"

#include <QDialog>
#include <QMultiMap>

namespace udg {

/**
   Class that implements the application configuration dialog.
  */
class QConfigurationDialog : public QDialog, private ::Ui::QConfigurationDialogBase {
    Q_OBJECT
public:
    QConfigurationDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

    ~QConfigurationDialog();

protected:
    ///@brief Calls .close() on each configuration subdialog.
    void closeEvent(QCloseEvent* event);

private slots:
    /// It is responsible for hiding /
    /// showing configuration options depending on whether or not the option has been selected
    /// to show advanced options.
    void setViewAdvancedConfiguration();

    /// Puts scroll on top and resizes widgets
    void sectionChanged(int index);

private:
    /// It helps us to differentiate between advanced configuration and basic configuration
    enum ConfigurationType { BasicConfiguration, AdvancedConfiguration };

    /// Adds a new configuration widget to the dialog.
    void addConfigurationWidget(QWidget *widget, const QString &name, ConfigurationType type);

private:
    QMultiMap<ConfigurationType, QListWidgetItem*> m_configurationListItems;
    QList<QWidget*> m_configurationScreenWidgets;
};

}

#endif
