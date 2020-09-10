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

#ifndef UDGPATIENTBROWERMENU_H
#define UDGPATIENTBROWERMENU_H

#include <QWidget>

class QPoint;

namespace udg {

class Patient;
class Volume;
class PatientBrowserMenuExtendedInfo;
class PatientBrowserMenuList;

/**
Widget in the form of a menu that displays the patient information we give him
input so that we can browse their studies and corresponding volumes
  */
class PatientBrowserMenu : public QWidget {
    Q_OBJECT
public:
    PatientBrowserMenu(QWidget *parent = 0);
    ~PatientBrowserMenu();

    /// We assign a patient to represent in the menu
    void setPatient(Patient *patient);

    /// Sets if the menu can show fusion pair options if any
    void setShowFusionOptions(bool show);

public slots:
    /// Let the menu be displayed in the indicated position
    void popup(const QPoint &point, const QString &identifier = QString());

signals:
    /// Signal that notifies the volume corresponding to the chosen item
    void selectedVolume(Volume *);
    void selectedVolumes(const QList<Volume *> &);

private slots:
    /// Update views related to the active item (the one the mouse hovers over)
    void updateActiveItemView(const QString &identifier);

    /// Given the item identifier does the relevant actions.
    /// In this case it is responsible for obtaining the volume selected by the user and notifying it
    void processSelectedItem(const QString &identifier);

private:
    /// Creates the widgets that make up the menu
    void createWidgets();

    /// It returns us true if we need to align the menu to the right. False in case it has to be aligned to the left.
    bool shouldAlignMenuToTheRight(const QRect &currentScreenGeometry);

    /// Calculates the size that is outside the widget screen with the list of studies / series according to the popup point.
    /// If it is inside the screen, the size will be (0,0). The third parameter indicates how we want the menu to be aligned
    /// with respect to the screen. If true, do the calculations keeping in mind that it is aligned to the right, otherwise as if it is aligned to the left
    void computeListOutsideSize(const QPoint &popupPoint, QSize &out, bool rightAligned);

    /// Place the additional information widget in the most appropriate place depending on the position of the main menu
    void placeAdditionalInfoWidget();

private:
    /// Attribute that saves the pointer to the basic menu that represents the patient data
    PatientBrowserMenuList *m_patientBrowserList;

    /// Attribute that saves the pointer in the menu with additional information of the selected item
    PatientBrowserMenuExtendedInfo *m_patientAdditionalInfo;

    /// Identifiers of the screens with respect to where the menu is displayed
    /// The ID of the current screen, and the attached screens, will be calculated each time the menu popup is performed
    int m_currentScreenID;
    int m_leftScreenID;
    int m_rightScreenID;

    /// Boolean to know if menu can show fusion pair options if any
    bool m_showFusionOptions;
};

}

#endif
