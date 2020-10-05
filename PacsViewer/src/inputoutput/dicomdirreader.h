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

#ifndef UDGDICOMDIRREADER_H
#define UDGDICOMDIRREADER_H

#include <QString>
#include <QList>

class DcmDicomDir;
class DcmDirectoryRecord;

namespace udg {

class DicomMask;
class Patient;
class Study;
class Series;
class Image;
class Status;

/**
This class allows you to read a dicomdir and consult its elements.
Accessing through the tree structure representing the dicomdirs
Patient / Study / Series / Images, we access the Dicomdir information for
perform searches.
*/
class DICOMDIRReader {
public:
    DICOMDIRReader();
    ~DICOMDIRReader();

    /// Open a domcidr directory
    /// @param dicomdirPath directory where dicomdir is located
    /// @return method state
    Status open(const QString &dicomdirFilePath);

    /// Returns the list of studies contained in the dicomdir
    /// @param outResultsStudyList list of studies contained in dicomdir
    /// @param studyMask search mask for studies to search within the dicomdir
    /// @return method state
    Status readStudies(QList<Patient*> &outResultsStudyList, DicomMask studyMask);

    /// Returns the list of series of a study that is in the dicomdir
    /// @param studyUID UID of the study whose series you want to consult
    /// @param outResultsSeriesList lists the series that the study contains
    /// @return method state
    Status readSeries (const QString &studyUID, const QString &seriesUID, QList<Series*> &outResultsSeriesList);

    /// Returns the list of images contained in a studio
    /// @param seriesUID UID of the series we want to get the images
    /// @param imageList List of images it contains
    /// @return method state
    Status readImages(const QString &seriesUID, const QString &sopInstanceUID, QList<Image*> &outResultsImageList);

    /// Returns the path of the dicomdir
    /// @return path of dicomdir
    QString getDicomdirFilePath();

    ///
    /// It returns all the files that make up that study
    /// EVERY method of convenience for not having to do weird loops
    /// to queryscreen and remove one more dependency from DICOMSeries / Study, etc
    /// @param studyUID UID of the study whose files we want
    /// @return A list of the absolute paths of the files in question
    QStringList getFiles(const QString &studyUID);

    /// Returns the Patient structure for the study that meets the mask passed to him.
    /// Only the StudyInstanceUID is considered in the mask.
    Patient* retrieve(DicomMask maskToRetrieve);

private:
    DcmDicomDir *m_dicomdir;
    QString m_dicomdirAbsolutePath, m_dicomdirFileName;
    bool m_dicomFilesInLowerCase;

    ///Check that a patient complies with the mask (check that they comply with Patient Name and Patient ID)
    bool matchPatientToDicomMask(Patient *patient, DicomMask *mask);

    /// Check if a study meets the mask, by the fields StudyUID, StudyDate
    bool matchStudyToDicomMask(Study *study, DicomMask *mask);

    /// Check that the two StudyUIDs in the mask and in the study match.
    /// If the mask UID study is empty, it returns true by default.
    /// In this case we do wildcard matching
    bool matchDicomMaskToStudyUID(DicomMask *mask, Study *study);

    /// Check that the two PatientId in the mask and in the study match.
    /// If the Patient Id of the mask is empty, it returns true by default.
    /// In this case we do wildcard matching
    bool matchDicomMaskToPatientId(DicomMask *mask, Patient *patient);

    /// Check that the date of the mask and that of the studio match.
    /// If the studyMaskDate is empty it returns true by default
    bool matchDicomMaskToStudyDate(DicomMask *mask, Study *study);

    /// Check that the name of the patient in the mask and that of the study match.
    /// and the studyMaskPatientName is empty returns true by default.
    /// In this case we do wildcard matching
    bool matchDicomMaskToPatientName(DicomMask *mask, Patient *patient);

    /// From a DcmDirectoryRecord returns the data of a Patient
    Patient* fillPatient(DcmDirectoryRecord *dcmDirectoryRecordPatient);

    /// From a DcmDirectoryRecord returns the data from a Study
    Study* fillStudy(DcmDirectoryRecord *dcmDirectoryRecordStudy);

    ///From a DcmDirectoryRecord returns data from a Series
    Series* fillSeries(DcmDirectoryRecord *dcmDirectoryRecordSeries);

    /// From a DcmDirectoryRecord returns the data of an Image
    Image* fillImage(DcmDirectoryRecord *dcmDirectoryRecordImage);

    /// Change the '\' to '/'. This is because the dcmtk return the path of
    /// the image in Windows format with the directories separated by '\'. In the case
    /// from linux we have to pass them to '/'
    /// ALL this method can be replaced by QDir :: toNativeSeparators () or similar
    /// which will return the appropriate separators to the system
    /// @param original path original
    /// @return path with '/'
    QString backSlashToSlash(const QString &original);

    /// Build us the relative Path of an image, put the '/'
    /// correctly and capitalize the file name depending on whether the dicomdir
    /// contains lowercase or uppercase files
    QString buildImageRelativePath(const QString &relativePath);
};

}

#endif
