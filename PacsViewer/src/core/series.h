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

#ifndef UDGSERIES_H
#define UDGSERIES_H

#include <QObject>
#include <QString>
#include <QSet>
#include <QDateTime>
#include <QPixmap>
#include "identifier.h"
#include "dicomsource.h"

namespace udg {

class EncapsulatedDocument;
class Volume;
class Image;
class Study;

/**
Class that encapsulates a patient's series.
The class contains all kinds of information related to a patient's series. A series can be equivalent to one or more volumes,
therefore we will have the list of Volumes corresponding to the series.
*/
class Series : public QObject {
    Q_OBJECT
public:
    Series(QObject *parent = 0);
    ~Series();

    /// Assign / Obtain the universal identifier of the series
    void setInstanceUID(QString uid);
    QString getInstanceUID() const;

    /// Assign / Obtain the SOP Class UID
    void setSOPClassUID(QString sopClassUID);
    QString getSOPClassUID() const;

    /// Assign / Obtain the mode of the series
    void setModality(QString modality);
    QString getModality() const;

    /// Assign / Obtain the serial number
    void setSeriesNumber(QString number);
    QString getSeriesNumber() const;

    /// Assign / Obtain the FrameOfReferenceUID
    void setFrameOfReferenceUID(QString uid);
    QString getFrameOfReferenceUID() const;

    /// Assign / Obtain the PositionReferenceIndicator
    void setPositionReferenceIndicator(QString position);
    QString getPositionReferenceIndicator() const;

    /// Assign / Obtain the description of the series
    void setDescription(QString description);
    QString getDescription() const;

    /// Assign / Obtain the patient's position relative to the machine
    /// We have several defined terms that tell us in what position the patient enters the machine
    /// - Head First means that the patient enters with his head
    /// - Feet First means that the patient enters from the feet
    /// - Prone means the patient looks down
    /// - Supine means the patient looks up
    /// - Decubitus Right means that the patient is leaning on his left side
    /// - Decubitus Left means that the patient is leaning on his right side
    ///
    /// These are the available terms
    /// HFP: Head First-Prone HFS: Head First-Supine
    /// HFDR: Head First-Decubitus Right HFDL: Head First-Decubitus Left
    /// FFDR: Feet First-Decubitus Right FFDL: Feet First-Decubitus Left
    /// FFP: Feet First-Prone FFS: Feet First-Supine
    ///
    /// @param position String with one of the values ​​mentioned
    void setPatientPosition(QString position);
    QString getPatientPosition() const;

    /// Assign / Obtain the protocol of the series
    void setProtocolName(QString protocolName);
    QString getProtocolName() const;

    /// Assign / Obtain the date and time of acquisition of the series. The date format will be YYYYMMDD and the
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

    /// Assign / Obtain the institution where the study was carried out
    void setInstitutionName(QString institutionName);
    QString getInstitutionName() const;

    /// Assigns / Returns the examined body part
    void setBodyPartExamined(QString bodyPart);
    QString getBodyPartExamined() const;

    /// Assigns / Returns the radiographic view associated with the patient's position
    void setViewPosition(QString viewPosition);
    QString getViewPosition() const;

    /// Assign / Obtain the name of the equipment of the equipment with which the images of the series have been captured
    void setManufacturer(QString manufactures);
    QString getManufacturer() const;

    /// Assign / Obtain the date and time the series was downloaded to the Local database
    void setRetrievedDate(QDate date);
    void setRetrievedTime(QTime time);
    QDate getRetrievedDate() const;
    QTime getRetrievedTime() const;

    /// Assign the parent studio of the series
    void setParentStudy(Study *study);
    Study* getParentStudy() const;

    /// Adds an image object to the series and assigns it as a relative to that series object.
    /// If the image already exists in the set it returns false, otherwise true
    bool addImage(Image *image);

    /// Tells us if an image with this identifier exists in the list
    /// @param identifier image identifier we are looking for
    /// @return True if it exists, false otherwise
    bool imageExists(const QString &identifier);

    /// Returns a list of all images in the series
    QList<Image*> getImages() const;
    void setImages(QList<Image*> imageSet);

    /// It tells us how many images this series has
    /// @return The number of images. 0 if it is not a series of images or does not contain them
    int getNumberOfImages() const;

    /// Returns the number of items in the series, without differentiating whether they are images or not.
    /// Equivalent to the number of files contained in the series
    int getNumberOfItems();

    /// Specify the number of images: required for when we fill from the bdd
    void setNumberOfImages(int numberOfImages);

    /// Indicates whether a series has images
    bool hasImages() const;

    /// Adds the given encapsulated document to this series and sets this series as the parent of the document.
    /// Returns true if the encapsulated document is added and false if it's not added because it was already in this series.
    bool addEncapsulatedDocument(EncapsulatedDocument *document);
    /// Returns true if this series contains an encapsulated document with the given key and false otherwise.
    bool encapsulatedDocumentExists(const QString &key) const;
    /// Returns the list of all the encapsulated documents in this series.
    const QList<EncapsulatedDocument*>& getEncapsulatedDocuments() const;
    /// Returns the number of encapsulated documents in this series.
    int getNumberOfEncapsulatedDocuments() const;
    /// Sets the number of encapsulated documents that is returned when there are no documents.
    /// It's used when querying the database to avoid retrieving the documents themselves.
    void setNumberOfEncapsulatedDocuments(int numberOfEncapsulatedDocuments);
    /// Returns true if this series contains at least one encapsulated document and false otherwise.
    bool hasEncapsulatedDocuments() const;

    ///Assign / Obtain the path of the images in the series
    void setImagesPath(QString imagesPath);
    QString getImagesPath() const;

    /// Assignar/Obtenir el requeste procedure ID
    void setRequestedProcedureID(const QString &requestedProcedureID);
    QString getRequestedProcedureID() const;

    /// Assignar/Obtenir el scheduled procedure step ID
    void setScheduledProcedureStepID(const QString &scheduledProcedureStepID);
    QString getScheduledProcedureStepID() const;

    /// Assignar/Obtenir el Performed Procedure Step Start Date
    void setPerformedProcedureStepStartDate(const QString &startDate);
    QString getPerformedProcedureStepStartDate() const;

    /// Assign / Get the Perfomed Procedure Step Start Time
    void setPerformedProcedureStepStartTime(const QString &startTime);
    QString getPerformedProcedureStepStartTime() const;

    /// Assign / Get the laterality of the series
    void setLaterality(const QChar &laterality);
    QChar getLaterality() const;

    /// Indicates the DICOM source for the series
    void setDICOMSource(const DICOMSource &seriesDICOMSource);
    /// Returns a DICOMSource that brings together the different DICOMSources of the images contained in the series and the one in the series if one has been assigned.
    DICOMSource getDICOMSource() const;


    /// Returns the Volume with identifier id
    /// Returns NULL in case there is no volume with this id.
    Volume* getVolume(Identifier id) const;

    /// Convenience method used to return the "first" volume. In 90% of cases (currently 100%)
    /// we will have that for each series there will be only one volume. This method returns this or, in the case of more than one, the first.
    /// Returns NULL in case there is no volume.
    Volume* getFirstVolume() const;

    /// Method for adding a single volume to the list of volumes in the series. Returns the id that was saved in the repository
    /// of volumes.
    Identifier addVolume(Volume *volume);

    /// Returns a list of volumes that compose the series.
    QList<Volume*> getVolumesList();

    /// Returns the number of volumes that compose the series.
    int getNumberOfVolumes();

    /// Returns the list of volume identifiers
    QList<Identifier> getVolumesIDList() const;

    /// List of image filenames
    QStringList getImagesPathList();

    /// Tells us if the series is marked as selected or not
    bool isSelected() const;

    QString toString(bool verbose = false);

    /// Gets the preview image of the series. It will be the image in the middle.
    QPixmap getThumbnail();

    /// Temporary method to get the Image according to how it is sorted in the list
    /// If the index we give is out of range, a null image is returned
    /// ALL of this does not assure us that we are getting the image we are supposed to want
    Image* getImageByIndex(int index) const;

    /// Lets you set the thumbnail of the series
    void setThumbnail(QPixmap seriesThumbnail);

    /// This method checks if this series is viewable
    /// that is to say that it can be visualized in a Q * Viewer
    /// The checks he makes are to make sure that it is not a modality
    /// that is not an image (PR, KIN, SR ...) and / or that actually contains images in image set
    /// The check is done "on the fly", ie on demand
    /// EVERYTHING remains to be done other more careful checks like
    /// check that it is a SOP Class supported by the application
    bool isViewable() const;

    /// Tells us if the series is a CT localizer
    bool isCTLocalizer() const;

    /// Tells us if the series is an MR survey
    bool isMRSurvey() const;
    
    /// Method for obtaining the volume contained in a particular image
    Volume* getVolumeOfImage(Image *image);

public slots:
    /// Select / deselect this series
    void select();
    void unSelect();
    void setSelectStatus(bool select);

private:
    /// Find the index of the image with the given identifier in the list of images
    /// @param identifier The identifier of the image we want to find
    /// @return The index of that image within the list, -1 if the image with that identifier does not exist.
    int findImageIndex(const QString &identifier);

private:
    /// Unique identification of the type of SOP. See PS 3.4 for possible values.
    QString m_sopClassUID;

    /// Common information of the series. C.7.3.1 General Series Module - PS 3.3.

    /// Type of equipment that originally acquired the data to create the images created in this series.
    /// See C.7.3.1.1.1 for the defined terms. (0008,0060) Type 1
    QString m_modality;

    /// Universal identifier of the series. (0020,000E) Type 1
    QString m_seriesInstanceUID;

    /// Number that identifies the series. (0020,0011) Type 2
    QString m_seriesNumber;

    /// Date and time the series started. (0008,0021), (0008,0031) Type 3.
    QDate m_date;
    QTime m_time;

    // \ TODO this tag is inside the Equipment IE module. It is supposed to be more correct to put it on a serial level.
    // Read A.1.2.3 IE SERIES, section c., A.1.2.4 IE EQUIPMENT and C.7.5 Common Equipment IE Modules - PS 3.3
    /// Name of the institution in which it was made
    QString m_institutionName;

    /// Descriptor of the patient's position relative to the machine. Required by CT and MR images. It does not have to be present
    /// if Patient Orientation Code Sequence (0054,0410) is present. See C.7.3.1.1.2 for defined terms and a deeper explanation.
    /// (0018,5100) Type 2C
    QString m_patientPosition;

    /// Protocol that has been applied to obtain user-defined images. (0018,1030) Type 3
    QString m_protocolName;

    /// Description of the user-defined series. (0008,103E) Type 3
    QString m_description;

    // FRAME OF REFERENCE MODULE. C.7.4
    // TODO In principle it is not necessary to implement it as a separate entity and including it within series should be
    // enough to be able to deal with what we need
    /// Identifies the frame of reference universally. See C.7.4.1.1.1 for a more in-depth explanation. (0020,0052) Type 1
    QString m_frameOfReferenceUID;

    /// Part of the patient's anatomy used as a reference. See C.7.4.1.1.2, for annotation purposes only. (0020,1040) Type 2
    QString m_positionReferenceIndicator;

    /// Laterality of the body part examined. Required if this is paired and we do not have Image or Frame Laterality.
    /// Defined values:
    /// R = right
    /// L = left
    QChar m_laterality;

    /// Indicates whether the series is marked as selected or not
    bool m_selected;

    /// Directory under which we find the images
    QString m_imagesPath;

    /// List of id's of volumes that make up the series. The series can be separated into several volumes for several reasons,
    /// such as different image sizes, series with dynamics or phases, stacks, etc.
    QList<Identifier> m_volumesList;

    /// List of Images in the series sorted by sort criteria such as SliceLocation, InstanceNumber, etc.
    /// EVERYTHING needs to be defined which is the default sorting strategy
    QList<Image*> m_imageSet;

    /// List of encapsulated documents contained in this series.
    QList<EncapsulatedDocument*> m_encapsulatedDocumentSet;

    /// Father study
    Study *m_parentStudy;

    /// Body part examined. (0018,015). Type 2/3, according to modality
    QString m_bodyPartExamined;

    /// Radiographic view associated with patient position [(0018,5101)]. (0018,5101). Type 2/3, according to modality
    QString m_viewPosition;

    /// Obtained by the manufacturer of the appliance with which the series (0008,0070) Type 2 was obtained.
    QString m_manufacturer;

    /// Date the series was downloaded to the local database
    QDate m_retrievedDate;
    QTime m_retrieveTime;

    /// Number of images in the series. We do the count manually for when we consult the BDD
    int m_numberOfImages;

    /// Number of encapsulated documents in the series. It's used when querying the database to avoid querying the full document set.
    int m_numberOfEncapsulatedDocuments;

    /// Save the series thumbnail
    QPixmap m_seriesThumbnail;

    /// Request Procedure ID (0040,1001) Type 1C / 2
    QString m_requestedProcedureID;
    /// Schedulet Procedure Step ID (0040,0253) Type 1C / 3
    QString m_scheduledProcedureStepID;
    /// Perfomed procedure step start date (0040,0244) Type 3
    QString m_performedProcedureStepStartDate;
    /// Perfomed procedure step start time (0040,0345) Type 3
    QString m_performedProcedureStepStartTime;

    ///Indicates from which DICOM sources the DICOM series and images were obtained
    DICOMSource m_seriesDICOMSource;
};

}

#endif
