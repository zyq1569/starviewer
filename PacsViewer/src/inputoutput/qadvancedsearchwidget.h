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


#ifndef UDGQADVANCEDSEARCHWIDGET_H
#define UDGQADVANCEDSEARCHWIDGET_H

#include <ui_qadvancedsearchwidgetbase.h>

#include <QPair>
#include <QTime>
#include <QDate>

class QString;

namespace udg {

class DicomMask;

/**
   Basic item search class
*/
class QAdvancedSearchWidget : public QWidget, private Ui::QAdvancedSearchWidgetBase {
    Q_OBJECT
public:
    QAdvancedSearchWidget(QWidget *parent = 0);

    /// Clear the basic search fields
    void clear();
    /// Build the search mask
    DicomMask buildDicomMask();

private slots:
    /// Put an * in the tab which belongs to the control that has been given value,
    /// so that the user knows which tabs have set search values
    void updateAdvancedSearchModifiedStatus();

    /// When you change the date of QDateEdit from which date of birth to search,
    /// update QDateEdit to what date to search with
    /// the same date
    void fromPatientBirthDateChanged();
    /// When you change the date check that the date from is not greater than this,
    /// if it is assigned to the date from the same as the date to
    void toPatientBirthDateChanged();

private:
    
    void initialize();

    /// Creates connections between signals and slots
    void createConnections();

    /// Convert a string to DICOM date format YYYYMMDD-YYYYMMDD, YYYYMMDD-, YYYYMMDD,
    /// ... to a Date range in which to search.
    /// If the String does not have the correct date DICOM format Returns invalid QDate ()
    /// (Consult PS 3.5 the Value Representation DA to see what format it can have)
    QPair<QDate, QDate> getDateRangeToSearchFromString(QString dateRangeAsDICOMFormat);

    ///Converteix un string en format de temps DICOM HHMMSS-HHMMSS, HHMMSS-, HHMMSS, ... a un rang de Time en que s'ha de fer la cerca
    ///Si l'String no té el format DICOM de temps correcte Retorna QTime() invàlids (Consultar PS 3.5 el Value Representation TM per veure quin format pot tenir)
    QPair<QTime, QTime> getTimeRangeToSearchFromString(QString timeRangeAsDICOMFormat);

    ///Ens indica si el QObject és un QLineEdit enabled i conté text
    bool isQLineEditEnabledAndIsNotEmpty(QObject *);
    ///Ens indica si el QOjbect és un QCheckBox i conté text
    bool isQCheckboxAndIsChecked(QObject *);

    ///Retorna el Widgets del control que he afegit nosaltres a la UI
    QList<QObject*> getMemberWidgets();

private:

    static const QRegExp regExpGetMemberWidgets;
};

}

#endif
