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

#include "qadvancedsearchwidget.h"

#include <QCalendarWidget>

#include "dicommask.h"

namespace udg {

const QRegExp QAdvancedSearchWidget::regExpGetMemberWidgets("^m_*");

QAdvancedSearchWidget::QAdvancedSearchWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    createConnections();
    initialize();

    m_patientAge->setValidator(new QIntValidator(0, 199, m_patientAge));
}

void QAdvancedSearchWidget::initialize()
{
    m_fromPatientBirth->setDisplayFormat("dd/MM/yyyy");
    m_toPatientBirth->setDisplayFormat("dd/MM/yyyy");

    m_fromPatientBirth->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
    m_toPatientBirth->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
}

void QAdvancedSearchWidget::createConnections()
{
    connect(m_fromPatientBirthCheckBox, SIGNAL(toggled(bool)), m_fromPatientBirth, SLOT(setEnabled(bool)));
    connect(m_toPatientBirthCheckBox, SIGNAL(toggled(bool)), m_toPatientBirth, SLOT(setEnabled(bool)));
    connect(m_fromPatientBirth, SIGNAL(dateChanged(const QDate&)), SLOT(fromPatientBirthDateChanged()));
    connect(m_toPatientBirth, SIGNAL(dateChanged(const QDate&)), SLOT(toPatientBirthDateChanged()));

    /// We create connect to know when some of the controls to filter have value,
    /// findChildren returns only the added widgets
    /// for us in the UI.
    foreach (QLineEdit *lineEdit, m_qwidgetAdvancedSearch->findChildren<QLineEdit*>(regExpGetMemberWidgets))
    {
        connect(lineEdit, SIGNAL(textChanged(const QString&)), SLOT(updateAdvancedSearchModifiedStatus()));
    }

    foreach (QCheckBox *checkBox, m_qwidgetAdvancedSearch->findChildren<QCheckBox*>())
    {
        connect(checkBox, SIGNAL(toggled(bool)), SLOT(updateAdvancedSearchModifiedStatus()));
    }
}

void QAdvancedSearchWidget::clear()
{
    //Patient fields
    m_fromPatientBirthCheckBox->setChecked(false);
    m_toPatientBirthCheckBox->setChecked(false);
    m_patientAge->clear();
    
    // Fields of study
    m_studyIDText->clear();
    m_studyUIDText->clear();
    m_accessionNumberText->clear();
    m_referringPhysiciansNameText->clear();
    m_studyModalityText->clear();
    m_studyTimeText->clear();

    // Series fields
    m_seriesUIDText->clear();
    m_requestedProcedureIDText->clear();
    m_scheduledProcedureStepIDText->clear();
    m_PPStartDateText->clear();
    m_PPStartTimeText->clear();
    m_seriesNumberText->clear();

    // Image fields
    m_SOPInstanceUIDText->clear();
    m_instanceNumberText->clear();
}

DicomMask QAdvancedSearchWidget::buildDicomMask()
{
    // To search between values see chapter 4 of DICOM point C.2.2.2.5
    // By default if we pass an empty value to the mask, it will search for all the values in that field
    // Here we have to make a set in all the fields we want to search
    DicomMask mask;

    // The DICOM PatientAge format is 3 digits + Y Ex: 054Y. PS3.5 Table 6.2-1 VR AS
    mask.setPatientAge(m_patientAge->text().isEmpty() ? "" : m_patientAge->text().rightJustified(3, '0') + "Y");
    mask.setPatientSex("");
    mask.setPatientBirth(m_fromPatientBirthCheckBox->isChecked() ? m_fromPatientBirth->date() : QDate(),
                         m_toPatientBirthCheckBox->isChecked() ? m_toPatientBirth->date() : QDate());
    mask.setPatientID("");
    mask.setPatientName("");

    mask.setStudyID(m_studyIDText->text());
    mask.setStudyDescription("");
    mask.setStudyInstanceUID(m_studyUIDText->text());
    mask.setStudyModality(m_studyModalityText->text());
    mask.setAccessionNumber(m_accessionNumberText->text());
    mask.setReferringPhysiciansName(m_referringPhysiciansNameText->text());
    QPair<QTime, QTime> studyTimeRange = getTimeRangeToSearchFromString(m_studyTimeText->text());
    mask.setStudyTime(studyTimeRange.first, studyTimeRange.second);


    // If we have to filter by a field at image or serial level we activate the serial filters
    if (!m_seriesUIDText->text().isEmpty() || !m_scheduledProcedureStepIDText->text().isEmpty() ||
            !m_requestedProcedureIDText->text().isEmpty() ||
            !m_SOPInstanceUIDText->text().isEmpty() || !m_instanceNumberText->text().isEmpty() ||
            !m_PPStartDateText->text().isEmpty() || !m_PPStartTimeText->text().isEmpty() ||
            !m_seriesNumberText->text().isEmpty())
    {
        mask.setSeriesDate(QDate(), QDate());
        mask.setSeriesTime(QTime(), QTime());
        mask.setSeriesModality("");
        mask.setSeriesNumber(m_seriesNumberText->text());
        mask.setSeriesInstanceUID(m_seriesUIDText->text());
        mask.setRequestAttributeSequence(m_requestedProcedureIDText->text(), m_scheduledProcedureStepIDText->text());

        QPair<QDate, QDate> rangePPSStartDate = getDateRangeToSearchFromString(m_PPStartDateText->text());
        mask.setPPSStartDate(rangePPSStartDate.first, rangePPSStartDate.second);
        QPair<QTime, QTime> rangePPSStartTime = getTimeRangeToSearchFromString(m_PPStartTimeText->text());
        mask.setPPStartTime(rangePPSStartTime.first, rangePPSStartTime.second);

        if (!m_SOPInstanceUIDText->text().isEmpty() || !m_instanceNumberText->text().isEmpty())
        {
            mask.setImageNumber(m_instanceNumberText->text());
            mask.setSOPInstanceUID(m_SOPInstanceUIDText->text());
        }

    }

    return mask;
}

void QAdvancedSearchWidget::updateAdvancedSearchModifiedStatus()
{
    for (int i = 0; i < m_qwidgetAdvancedSearch->count(); i++)
    {
        bool hasModifiedLineEdit = false;
        QWidget *tab = m_qwidgetAdvancedSearch->widget(i);

        /// We filter that the Widgets that the findChildren method returns to us are
        /// Widgets added by us to the UI, if we didn't filter
        /// the findChildren method would return all the objects of
        /// Qt that make up the tab, for example for the patient tab the QDateEdit is formed
        /// among other controls for QLineEdit, this can generate us
        /// problems to know if in that tab we have some value to filter in the searches
        /// because we would find a QLineEdit with value
        foreach (QObject *child, tab->findChildren<QObject*>(QRegExp(regExpGetMemberWidgets)))
        {
            if (isQLineEditEnabledAndIsNotEmpty(child) || isQCheckboxAndIsChecked(child))
            {
                hasModifiedLineEdit = true;
                break;
            }
        }

        QString tabText = m_qwidgetAdvancedSearch->tabText(i).remove(QRegExp("\\*$"));
        if (hasModifiedLineEdit)
        {
            tabText += "*";
        }
        m_qwidgetAdvancedSearch->setTabText(i, tabText);
    }
}

void QAdvancedSearchWidget::fromPatientBirthDateChanged()
{
    m_toPatientBirth->setDate(m_fromPatientBirth->date());
}

void QAdvancedSearchWidget::toPatientBirthDateChanged()
{
    if (m_fromPatientBirth->date() > m_toPatientBirth->date())
    {
        m_fromPatientBirth->setDate(m_toPatientBirth->date());
    }
}

QPair<QDate, QDate> QAdvancedSearchWidget::getDateRangeToSearchFromString(QString dateRangeAsDICOMFormat)
{
    QPair<QDate, QDate> rangeDate;

    if (!dateRangeAsDICOMFormat.contains("-"))
    {
        //It is not a rank
        rangeDate.first = QDate().fromString(dateRangeAsDICOMFormat, "yyyyMMdd");
        rangeDate.second = QDate().fromString(dateRangeAsDICOMFormat, "yyyyMMdd");
    }
    else
    {
        QStringList rangeDateSplitted = dateRangeAsDICOMFormat.split("-");

        if (rangeDateSplitted.count() > 1)
        {
            rangeDate.first = QDate().fromString(rangeDateSplitted.at(0), "yyyyMMdd");
            rangeDate.second = QDate().fromString(rangeDateSplitted.at(1), "yyyyMMdd");
        }
    }

    return rangeDate;
}

QPair<QTime, QTime> QAdvancedSearchWidget::getTimeRangeToSearchFromString(QString timeRangeAsDICOMFormat)
{
    QPair<QTime, QTime> rangeTime;

    if (!timeRangeAsDICOMFormat.contains("-"))
    {
        //It is not a rank
        rangeTime.first = QTime().fromString(timeRangeAsDICOMFormat, "HHmmss");
        rangeTime.second = QTime().fromString(timeRangeAsDICOMFormat, "HHmmss");
    }
    else
    {
        QStringList rangeTimeSplitted = timeRangeAsDICOMFormat.split("-");

        if (rangeTimeSplitted.count() > 1)
        {
            rangeTime.first = QTime().fromString(rangeTimeSplitted .at(0), "HHmmss");
            rangeTime.second = QTime().fromString(rangeTimeSplitted .at(1), "HHmmss");
        }
    }

    return rangeTime;
}

bool QAdvancedSearchWidget::isQLineEditEnabledAndIsNotEmpty(QObject *qObject)
{
    if (qobject_cast<QLineEdit*>(qObject) != NULL)
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(qObject);
        return !lineEdit->text().isEmpty() && lineEdit->isEnabled();
    }
    else
    {
        return false;
    }
}

bool QAdvancedSearchWidget::isQCheckboxAndIsChecked(QObject *qObject)
{
    if (qobject_cast<QCheckBox*>(qObject) != NULL)
    {
        return (qobject_cast<QCheckBox*>(qObject))->isChecked();
    }
    else
    {
        return false;
    }
}

}
