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

#ifndef UDGPATIENT_H
#define UDGPATIENT_H

#include <QObject>
#include <QString>
#include <QDate>
#include "study.h"

namespace udg {

/**
Class that represents the central unit of work of the application: the Patient.
It will save all the information related to these (Studies, Series, etc.)
*/
class Patient : public QObject {
    Q_OBJECT
public:
    /// Enumeration of degrees of similarity
    enum PatientsSimilarity { SamePatients, DifferentPatients, SamePatientIDsDifferentPatientNames, SamePatientNamesDifferentPatientIDs };

    Patient(QObject *parent = 0);

    /// Copy constructor, required by operators +, + =, -, - =
    Patient(const Patient &patient, QObject *parent = 0);
    ~Patient();

    /// Assign / Get the full name of the patient
    void setFullName(const QString &name);
    QString getFullName() const;

    /// Assign / Obtain the patient ID
    void setID(const QString &id);
    QString getID() const;

    /// Assign / Obtain the patient ID in the Starviewer database
    void setDatabaseID(qlonglong databaseID);
    qlonglong getDatabaseID() const;

    /// Assign / Get date of birth
    void setBirthDate(int day, int month, int year);
    void setBirthDate(const QString &date);
    QString getBirthDateAsString();
    QDate getBirthDate() const;
    int getDayOfBirth();
    int getMonthOfBirth();
    int getYearOfBirth();

    /// Assign / Get sex from the patient
    void setSex(const QString &sex);
    QString getSex() const;

    /// Add a new study. The study assigns this Patient as a "parentPatient".
    /// Returns false if a study already exists with the same uid or the uid is empty.
    bool addStudy(Study *study);

    /// Take the patient out of the study with the given UID
    void removeStudy(const QString &uid);

    /// Get the study with the given UID. If none with this uid returns null
    Study* getStudy(const QString &uid);

    /// Returns true if this patient is assigned the study with the given UID, otherwise false
    bool studyExists(const QString &uid);

    /// Method for obtaining the number of patient studies
    int getNumberOfStudies();

    /// Method for obtaining the patient's list of studies. By default it returns
    /// it to us in order with the most recent studies first.
    QList<Study*> getStudies(Study::StudySortType sortCriteria = Study::RecentStudiesFirst) const;

    /// Returns a list of the patient's studies that contain the given modality.
    /// The list of studies is not in any specific order.
    QList<Study*> getStudiesByModality(const QString &modality);

    /// Returns a string list with its studies modalities
    QStringList getModalities() const;
    Series* getSeries(const QString &uid);

    /// Quick method to find if there is a series with the given uid. Returns null if it does not exist
    QList<Series*> getSelectedSeries();
    
    /// Returns a list of volumes that compose the patient.
    QList<Volume*> getVolumesList();

    /// Returns the number of volumes that compose the patient.
    int getNumberOfVolumes();

    /// Tells us if this file belongs to any patient series
    bool hasFile(const QString &filename);

    /// Assignment operator
    Patient& operator =(const Patient &patient);

    /// Patient fusion operator. With this operator we can unify in a single Patient object the information of two Patient
    /// as long as they are identified as the same patient. This would be equivalent to a Union set operator
    /// EVERYTHING here would have to be considered if fusion can be forced even if they are apparently different patients.
    Patient operator+(const Patient &patient);
    Patient operator+=(const Patient &patient);

    /// Returns if it is considered that it can be identified as the same patient (from the ID and name).
    /// It does not compare either the studies or the series it contains, only the patient identification.
    Patient::PatientsSimilarity compareTo(const Patient *patient) const;

    /// Dumps patient information into a useful string for debugging tasks
    /// EVERYTHING Incorporate "verbose" parameter to be able to choose whether to make a complete dump or not
    QString toString() const;

    /// Mark the selected series with the indicated UID
    /// In case no such UID exists, the first one is selected
    /// series of the first study
    void setSelectedSeries(const QString &selectedSeriesUID);

	//clear
	void clearAllStudy();

signals:
    /// Emitted when a study has been added
    void studyAdded(Study*);

    /// Emitted when a study has been removed
    void studyRemoved(Study*);

private:
    /// Given a patient, copy only the patient information. It does nothing about studies, series or images.
    /// The information is crushed and no checks are made.
    void copyPatientInformation(const Patient *patient);

    /// Inserts a study into the study list sorted by date. The most recent studies first.
    /// Pre: It is assumed that it has been previously verified that the study does not exist in the list
    void insertStudy(Study *study);

    /// Find the index of the study with the uid given in the list of studies
    /// @param uid The study uid we want to find
    /// @return The index of that study within the list, -1 if the study with that uid does not exist.
    int findStudyIndex(const QString &uid);

    /// Create a log message to find out which patients we are merging
    /// @param patient The data of the patient that merges with this one
    void patientFusionLogMessage(const Patient &patient);

    /// Returns the patient name without non-alphanumeric characters,
    /// with spaces at the beginning and at the end trimmed, and converted to uppercase.
    QString getCleanName() const;

private:
    /// Common patient information for all images that refer to this patient. Section C.7.1.1 PS 3.3 DICOM.
    /// These are patient attributes necessary for diagnostic interpretation of patient images and are common to
    /// to all studies performed on the patient. \ EVERY time we do not include any optional attributes

    /// Full name of the patient. (0010,0010) Type 2.
    QString m_fullName;

    /// Primary identifier given to the patient by the hospital. (0010,0020) Type 2.
    QString m_patientID;

    /// Patient ID in Starviewer bd
    qlonglong m_databaseID;

    /// Date of birth. (0010,0030) Type 2.
    QDate m_birthDate;

    /// Sex. (0010,0040) Type 2. M = male, F = female, O = other
    QString m_sex;

    /// Indicates whether the patient's actual identity has been removed from attributes and data. (0012,0062) Type 3. Consider
    /// this parameter if necessary when anonymizing. This would force us to use the tags (0012,0063) and (0012,0064)
    // ALL This attribute will not be used yet, but it is to be taken into account in case it is needed.
    bool m_identityIsRemoved;

    /// List containing patient studies sorted by date
    QList<Study*> m_studiesList;
};

}

#endif
