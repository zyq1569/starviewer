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

#ifndef UDGSTUDY_H
#define UDGSTUDY_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include "series.h"
#include "dicomsource.h"

namespace udg {

class Patient;

/**
Class that encapsulates the study of a patient
*/
class Study : public QObject {
    Q_OBJECT
public:
    /// Type of arrangement of a set of studies
    enum StudySortType { RecentStudiesFirst, OlderStudiesFirst };

    Study(Patient *parentPatient = 0, QObject *parent = 0);
    ~Study();

    ///Assign / Obtain the Patient Parent object from the study
    void setParentPatient(Patient *patient);
    Patient* getParentPatient() const;

    /// Assign / Obtain the Patient Parent object from the study
    void setInstanceUID(QString uid);
    QString getInstanceUID() const;

    /// Assign / Obtain the universal identifier of the study
    void setID(QString id);
    QString getID() const;

    /// Assign / Obtain the study identifier
    void setAccessionNumber(QString accessionNumber);
    QString getAccessionNumber() const;

    /// Assign / Obtain the accession number of the study
    void setDescription(QString description);
    QString getDescription() const;

    /// Sets the patient age at study date.
    void setPatientAge(const QString &age);

    /// Gets the age of the patient at study date. If it's not set, it will be computed from birth and study date if both are valid.
    QString getPatientAge() const;

    /// Assign / Obtain the patient's weight
    void setWeight(double weight);
    double getWeight() const;

    /// Assign / Obtain the patient's height
    void setHeight(double height);
    double getHeight() const;

    /// Allows you to add a modality to the studio
    void addModality(const QString &modality);

    /// Returns the modalities contained in the study.
    /// @return the list of modalities in a string delimited by the character '/'
    QString getModalitiesAsSingleString() const;

    /// Returns the modalities contained in the study.
    /// @return una llista d'strings
    QStringList getModalities() const;

    /// Assign / Obtain the referring physician's name from the study
    void setReferringPhysiciansName(QString referringPhysiciansName);
    QString getReferringPhysiciansName() const;

    /// Assign / Obtain the date and time of acquisition of the study. The date format will be YYYYMMDD and the
    /// time hhmmss.frac where frac is a fraction of a second of range 000000-999999
    /// Returns false if there is a format error
    bool setDateTime(int day, int month, int year, int hour, int minute, int second = 0);
    bool setDateTime(QString date, QString time);
    bool setDate(int day, int month, int year);
    bool setDate(QString date);
    bool setDate(QDate date);
    bool setTime(int hour, int minute, int second = 0);
    bool setTime(QString time);
    bool setTime(QTime time);
    QDate getDate() const;
    QString getDateAsString();
    QTime getTime() const;
    QString getTimeAsString();
    QDateTime getDateTime() const;

    /// Assign / Obtain the date and time the study was downloaded to the Local database
    void setRetrievedDate(QDate date);
    void setRetrievedTime(QTime time);
    QDate getRetrievedDate() const;
    QTime getRetrievedTime() const;

    /// Assign / Obtain the institution from which the study was obtained
    void setInstitutionName(const QString &institution);
    QString getInstitutionName() const;

    /// Add a new series and assign this study object as a relative. Returns false if a series with the same uid exists
    bool addSeries(Series *series);

    /// Take the series to the studio with the given UID. If none exists with this uid it returns null.
    void removeSeries(QString uid);

    /// Gets the series with the given UID. NULL if not
    Series* getSeries(QString uid);

    /// Tells us if there is a series with this uid in the list
    /// @param uid the uid we are looking for
    /// @return True if it exists, false otherwise
    bool seriesExists(QString uid);

    /// Returns a list of series marked as selected
    QList<Series*> getSelectedSeries();

    /// Method for obtaining the number of series contained in the study
    int getNumberOfSeries();

    /// Method for obtaining the list of study series
    QList<Series*> getSeries() const;

    /// Returns the list of Series that can be displayed in a viewer
    QList<Series*> getViewableSeries();

    /// Returns a list of volumes that compose the study.
    QList<Volume*> getVolumesList();

    /// Returns the number of volumes that compose the study.
    int getNumberOfVolumes();

    /// Returns a DICOMSource that brings together the different DICOMSources of the series that the study contains and that of the study if one has been assigned.
    DICOMSource getDICOMSource() const;

    /// Indicates the DICOM source for the series
    void setDICOMSource(const DICOMSource &studyDICOMSource);

    QString toString();

    /// Operators to compare whether a study is older (operator <) or newer (operator>).
    bool operator<(const Study &study);
    bool operator>(const Study &study);

    /// Sorts a list of studies according to the given ordering criteria
    static QList<Study*> sortStudies(const QList<Study*> &studiesList, StudySortType sortCriteria);

private:
    /// Inserts a series into the series list sorted by SeriesNumber.
    /// Pre: It is assumed that it has been previously verified that the series does not exist in the list
    /// @param serie
    void insertSeries(Series *series);

    /// Find the index of the series with the uid given in the list of series
    /// @param uid The uid of the series we want to find
    /// @return The index of that series within the list, -1 if the series with that uid does not exist.
    int findSeriesIndex(QString uid);

private:
    /// Common study information. C.7.2.1 General Study Module - PS 3.3.

    /// Unique identifier of the study. (0020,000D) Type 1.
    QString m_studyInstanceUID;

    /// List of study modalities
    QStringList m_modalities;

    /// Date and time the study began. (0008,0020), (0008,0030) Type 2.
    QDate m_date;
    QTime m_time;

    /// Identifier of the study generated by the equipment or by the user. (0020,0010) Type 2.
    QString m_studyID;

    /// Accession Number. A RIS generated number that identifies the order for the Study. (0008,0050) Type 2.
    QString m_accessionNumber;

    /// Description of the study generated by the institution. (0008,1030) Type 3
    QString m_description;

    /// Patient information, related only to this study. This is the information we can have from a patient at the time
    /// he was studied. C.7.2.2 Patient Study Module - PS 3.3.

    /// Age of the patient. (0010,1010) Type 3
    QString m_age;

    /// Height (in meters) of the patient (0010,1020) Type 3
    double m_height;

    /// Weight (in Kg.) Of the patient (0010,1030) Type 3
    double m_weight;

    /// Doctor treating the patient during a visit, related to the exploratory test (0008,0090) Type 2
    QString m_referringPhysiciansName;

    /// Date the study was downloaded to the local database
    QDate m_retrievedDate;
    QTime m_retrieveTime;

    /// Institution of which the study has been obtained
    QString m_institutionName;

    /// List of Study Series sorted by serial number
    QList<Series*> m_seriesSet;

    /// The Patient entity to which this study belongs
    Patient *m_parentPatient;

    /// Contains the source of the DICOM objects in the study
    DICOMSource m_studyDICOMSource;
};

}

#endif
