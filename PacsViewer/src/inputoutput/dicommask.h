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

#ifndef DICOMMASK
#define DICOMMASK

#include <QString>
#include <QDate>
#include <QTime>

/**
This class builds a mask for searching for studies / series / images in PACS / BD / DICOMDIR.
Each class should be queried if all fields in that DICOMMask class are available in the search.

When searching the PACS, it only returns the fields that have been requested.
if we use the DICOMMask to do a search on the PACS and we want to indicate that it returns us
certain fields we have to set with string.empty with the fields we want us to
return for the studio / series / image and for the fields that are Date / Time a Date / Time null QDate ().
*/
namespace udg {

class Study;
class Series;
class Image;

class DicomMask {
public:
    DicomMask();

    /// We indicate the Patient name to filter by. If both parameters are Nulls
    /// all patients are searched, if for example minimumDate is null
    /// and maximumDate is passed value will look for born patients
    /// before or on the same day as the date indicated in maximumDate
    void setPatientName(const QString &patientName);

    /// This method specifies the date of birth of the patient with
    /// the one to look for studies. The format is DDMMYYYY
    void setPatientBirth(const QDate &minimumDate, const QDate &maximumDate);

    /// This action especified that in the search we want the Patient's sex
    /// @param Patient's sex of the patient to search.
    /// @return state of the method
    void setPatientSex(const QString &patientSex);

    /// We indicate the patient Id for which to filter
    void setPatientID(const QString &patientID);

    /// This action especified that in the search we want the Patient's Age
    /// @param  Patient's age of the patient to search.
    /// @return state of the method
    void setPatientAge(const QString &patientAge);

    /// This action especified that in the search we want the Study's Id
    /// @param  Study's Id of the study to search.
    /// @return state of the method
    void setStudyID(const QString &studyID);

    /// Indicate between which dates we want to filter the studies. If both parameters
    /// are Nulls all studies are searched, if for example minimumDate is null
    /// and maximumDate is passed value will look for the studies done before or the
    /// same day of the date indicated in maximumDate
    void setStudyDate(const QDate &minimumDate, const QDate &maximumDate);

    /// This action especified that in the search we want the Study's description
    ///  @param Study's description of the study to search.
    ///  @return state of the method
    void setStudyDescription(const QString &studyDescription);

    /// This action especified that in the search we want the Study's modality
    /// @param Study's modality the study to search.
    /// @return state of the method
    void setStudyModality(const QString &studyModality);

    /// We indicate between what hours we want to filter the studies. If both parameters
    /// are Nulls all studies are searched, if for example minimumTime is null
    /// and maximumTime is given value will look for the studies done before or of the time indicated in maximumTime
    void setStudyTime(const QTime &studyTimeMinimum, const QTime &studyTimeMaximum);

    /// This action especified that in the search we want the Study's instance UID
    /// @param Study's instance UID the study to search.
    /// @return state of the method
    void setStudyInstanceUID(const QString &studyInstanceUID);

    /// This action especified that in the search we want the Accession Number
    /// @param Accession Number of the study to search.
    /// @return state of the method
    void setAccessionNumber(const QString &accessionNumber);

    /// Set the ReferringPhysiciansName
    /// @param physiciansName
    /// @return method state
    void setReferringPhysiciansName(const QString &physiciansName);

    /// Returns the UID study from the mask
    /// @param mask [in] search mask
    /// @return UID study we are looking for
    QString getStudyInstanceUID() const;

    /// Returns the Patient Id to filter
    QString getPatientID() const;

    /// Returns the patient's name to filter
    QString getPatientName() const;

    /// Returns on what date of birth we should look for patients
    QDate getPatientBirthMinimum() const;
    QDate getPatientBirthMaximum() const;
    QString getPatientBirthRangeAsDICOMFormat() const;

    /// Returns patient age
    /// @return patient age
    QString getPatientAge() const;

    /// Returns the patient's sex
    /// @return patient sex
    QString getPatientSex() const;

    /// Returns Studio Id
    /// @return Studio ID
    QString getStudyID() const;

    /// Returns range of dates on which studies should be searched. He returns
    /// in String format for when searching PACS if it is null means we don't have it
    /// ask the PACS to return that field, if it is empty it means it must return it
    QDate getStudyDateMinimum() const;
    QDate getStudyDateMaximum() const;

    /// Returns in string format the Date Range in which we need to search for the study.
    ///  If it returns null it indicates that we have not been asked to search by date. Returns the date range separated by "-"
    QString getStudyDateRangeAsDICOMFormat() const;

    /// Returns the study description
    /// @return study description
    QString getStudyDescription() const;

    /// Returns from the study mode
    /// @return Study mode
    QString getStudyModality() const;

    /// Returns the time of the study
    /// @return study time
    QTime getStudyTimeMinimum() const;
    QTime getStudyTimeMaximum() const;
    QString getStudyTimeRangeAsDICOMFormat() const;

    /// Returns the accession number of the study
    /// @return accession number of the study
    QString getAccessionNumber() const;

    /// Retorna el ReferringPhysiciansName
    /// @return ReferringPhysiciansName
    QString getReferringPhysiciansName() const;

    /// This action especified in the search which series number we want to match
    /// @param seriesNumber' Number of the series to search. If this parameter
    /// is null it's supose that any mask is applied at this field
    /// @return estat del mètode
    void setSeriesNumber(const QString &seriesNumber);

    /// Indicate between which dates we want to filter the series. If both
    /// parameters are Nulls all studies are searched, if for example minimumDate is null
    /// and at maximumDate value is passed it will look for the series done before or
    /// the same day of the date indicated in maximumDate
    void setSeriesDate(const QDate &minimumDate, const QDate &maximumDate);

    /// Indicate between what hours we want to filter the series. If both
    /// parameters are Nulls all series are searched, if for example minimumTime is null
    /// and at maximumTime value is passed it will look for the series done before or of the time indicated in maximumTime
    void setSeriesTime(const QTime &minimumTime, const QTime &maximumTime);

    /// This action especified that in the search we want the seriess description
    /// @param Series description of the study to search. If this parameter
    /// is null it's supose that any mask is applied at this field.
    /// @return method state
    void setSeriesDescription(const QString &seriesDescription);

    /// This action especified that in the search we want the series modality
    /// @param series modality the study to search. If this parameter is
    /// null it's supose that any mask is applied at this field
    /// @return method state
    void setSeriesModality(const QString &seriesModality);

    /// This action especified that in the search we want to query the Protocol Name
    /// @param Protocol Name. If this parameter is null it's supose that
    /// any mask is applied at this field
    /// @return estat del mètode
    void setSeriesProtocolName(const QString &protocolName);

    /// This action especified that in the search we want the seriess description
    /// @param Series description of the study to search. If this parameter
    /// is null it's supose that any mask is applied at this field.
    /// @return method state
    void setSeriesInstanceUID(const QString &seriesInstanceUID);

    /// Estableix la seqüència per cercar per la requested atribute sequence.
    /// RequestAtrributeSequence està format pel RequestedProcedureID i el procedureStepID
    /// @param RequestedProcedureID Requested Procedure ID pel qual s'ha de cercar
    /// @param ScheduledProcedureStepID Scheduled Procedure Step ID pel qual s'ha de cercar
    /// @return method state
    void setRequestAttributeSequence(const QString &procedureID, const QString &procedureStepID);

    /// Sets the Perfomed Procedure Step Start date search mask
    /// @param startDate Perfomed Procedure Step Start date to search
    /// @return method state
    void setPPSStartDate(const QDate &minimumDate, const QDate &maximumDate);

    /// Sets the Perfomed Procedure Step Start Time search mask
    /// @param startTime Perfomed Procedure Step Start Time to search
    /// @return method state
    void setPPStartTime(const QTime &minimumTime, const QTime &maximumTime);

    /// Retorna el series Number
    /// @return   series Number
    QString getSeriesNumber() const;

    /// Returns the date of the series
    QDate getSeriesDateMinimum() const;
    QDate getSeriesDateMaximum() const;
    QString getSeriesDateRangeAsDICOMFormat() const;

    /// Returns the time of the series
    /// @return series time
    QTime getSeriesTimeMinimum() const;
    QTime getSeriesTimeMaximum() const;
    QString getSeriesTimeRangeAsDICOMFormat() const;

    /// Returns the description of the series
    /// @return description of the series
    QString getSeriesDescription() const;

    /// Returns the series mode
    /// @return series mode
    QString getSeriesModality() const;

    /// Returns the name of the protocol used in the series
    /// @return name of the protocol used in the seire
    QString getSeriesProtocolName() const;

    /// Returns the uid of the series
    /// @return SeriesUID
    QString getSeriesInstanceUID() const;

    /// Retorna el Requested procedures ID
    /// @return requesta procedure ID
    QString getRequestedProcedureID() const;

    /// Retorna el scheduled procedure step ID
    /// @return scheduled procedure step ID
    QString getScheduledProcedureStepID() const;

    /// Retorna el Perfomed Procedure Step Start date
    /// @return Perfomed Procedure Step Start date
    QDate getPPSStartDateMinimum() const;
    QDate getPPSStartDateMaximum() const;
    QString getPPSStartDateAsRangeDICOMFormat() const;

    /// Retorna el Perfomed Procedure Step Start Time
    /// @return Perfomed Procedure Step Start Time
    QTime getPPSStartTimeMinimum() const;
    QTime getPPSStartTimeMaximum() const;
    QString getPPSStartTimeAsRangeDICOMFormat() const;

    /// Set the StudyId of the images
    /// @param   Study instance UID the study to search. If this parameter
    /// is null it's supose that any mask is applied at this field
    /// @return The state of the action
    void setImageNumber(const QString &imageNumber);

    /// Estableix el instance UID de l'image
    /// @param SOPIntanceUID Instance UID de l'imatge
    /// @return estat del mètode
    void setSOPInstanceUID(const QString &SOPInstanceUID);

    /// Retorna el número d'imatge
    /// @return número d'imatge
    QString getImageNumber() const;

    /// Retorna el SOPInstance UID de l'imatge
    /// @return SOPInstance UID de l'imatge
    QString getSOPInstanceUID() const;

    bool operator ==(const DicomMask &mask);

    /// Defineix l'operador suma dos màscares, en el cas que el this i
    /// la màscara passada per paràmetre tinguin un valor de filtratge per una determinada
    /// propietat, no es cercarà pels dos valors,sinó només pel de la màscara this
    DicomMask operator +(const DicomMask &mask);

    /// Mètode de conveniència que ens retorna un string indicant els camps
    /// que s'han omplert per fer la màscara de forma que es pugui loggejar per estadístiques.
    /// Els camps amb informació personal s'indicaran com a omplerts amb un asterisc, però no
    /// es posarà la informació. Per exemple, si es fa servir el camp "nom de pacient" s'indicarà
    /// el valor "#*#" però no la informació que ha posat l'usuari. Tampoc es tindran en compte
    /// les wildcards '*' que puguin haver, per tant, si un camp està omplert amb "*", considerarem
    /// que no s'ha fet servir
    QString getFilledMaskFields() const;

    /// Ens diu si la màscara està buida o no
    bool isEmpty();

    ///Mètode que a partir d'un estudi/series/image ens retornen la DICOMMask
    /// per descarregar o consultar els fills d'aquell objecte. És important que si hem de construir
    ///un DICOMMask a partir d'una imatge aquesta inclogui la informació
    /// de la sèrie pare i estudi pare per construir, i en el cas de series contingui l'estudi pare, sinó
    ///el mètode ens retornarà un DicomMask invàlid que no ens servirà per
    /// fer descàrregues o consultes. El paràmetre ok indica si s'ha pogut fer correctament la conversió
    ///de l'objecte a DICOMMask
    static DicomMask fromStudy(Study *study, bool &ok);
    static DicomMask fromSeries(Series *series, bool &ok);
    static DicomMask fromImage(Image *image, bool &ok);

private:

    ///Retorna el rang d'un data o hora en format DICOM
    QString getDateOrTimeRangeAsDICOMFormat(const QString &minimumDateTime, const QString &maximumDateTime) const;

private:
    static const QString DateFormatAsString;
    static const QString TimeFormatAsString;

    ///Els camps que són Data/Hora els guardem con a QString perquè així alhora de cercar
    /// al PACS sabem si ens han demanat que es retorni aquell camp,
    ///ja que QString té 3 estats null (camp no sol·licitat), empty (s'ha de tornar
    /// aquell camp per tots els estudis que compleixin la cerca),
    ///valor s'han de tornar els estudis que tinguin aquell valor.

    QString m_patientID;
    QString m_patientName;
    QString m_patientBirthMinimum;
    QString m_patientBirthMaximum;
    QString m_patientSex;
    QString m_patientAge;
    QString m_studyID;
    QString m_studyDateMinimum;
    QString m_studyDateMaximum;
    QString m_studyTimeMinimum;
    QString m_studyTimeMaximum;
    QString m_studyDescription;
    QString m_studyModality;
    QString m_studyInstanceUID;
    QString m_accessionNumber;
    QString m_referringPhysiciansName;
    QString m_seriesNumber;
    QString m_seriesDateMinimum;
    QString m_seriesDateMaximum;
    QString m_seriesTimeMinimum;
    QString m_seriesTimeMaximum;
    QString m_seriesModality;
    QString m_seriesDescription;
    QString m_seriesProtocolName;
    QString m_seriesInstanceUID;
    QString m_requestedProcedureID;
    QString m_scheduledProcedureStepID;
    QString m_PPSStartDateMinimum;
    QString m_PPSStartDateMaximum;
    QString m_PPSStartTimeMinimum;
    QString m_PPSStartTimeMaximum;
    QString m_SOPInstanceUID;
    QString m_imageNumber;
};

} // End namespace
#endif
