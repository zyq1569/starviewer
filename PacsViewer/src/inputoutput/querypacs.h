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

#ifndef QUERYPACS
#define QUERYPACS

#include <QList>
#include <QHash>
#include <assoc.h>
#include <dcdeftag.h>

#include "pacsdevice.h"
#include "pacsrequeststatus.h"
#include "dicommask.h"
#include "dicomsource.h"
#include "dimsecservice.h"
/// This class helps to interactive with the pacs, allow us to find studies
/// in the pacs setting a search mask. Very important for this class a connection
/// and a mask search must be setted befoer query Studies

class DcmDataset;

struct T_DIMSE_C_FindRQ;
struct T_DIMSE_C_FindRSP;

namespace udg {

class Patient;
class Study;
class Series;
class Image;
class DICOMTagReader;
class PACSConnection;

class QueryPacs : public DIMSECService {
public:
    ///Class builder
    QueryPacs(PacsDevice pacsDevice);
    ~QueryPacs();

    /// Look for studies that meet the past mask
    PACSRequestStatus::QueryRequestStatus query(const DicomMask &mask);

    /// We indicate that the current query should be canceled.
    /// The query is not canceled immediately when the method is invoked,
    /// this method updates a flag, which every time
    /// that we receive a DICOM element that meets the mask is checked,
    /// if the flag indicates that it has been requested to be canceled then es
    /// cancel the query
    void cancelQuery();

    /// Patients return with the studies found. The class requesting the study search results,
    /// is responsible for removing returned objects this method
    QList<Patient*> getQueryResultsAsPatientStudyList();
    /// Return the found series. The class requesting the series search results,
    /// is responsible for removing returned objects this method
    QList<Series*> getQueryResultsAsSeriesList();
    /// Return the found images. The class requesting the image search results,
    /// is responsible for removing returned objects this method
    QList<Image*> getQueryResultsAsImageList();

private:
    /// Query the pacs
    PACSRequestStatus::QueryRequestStatus query();

    /// This is a method that is called callback by dcmtk,
    /// for each object I say is in the PACS that fulfills the query dcmtk calls it.
    /// This method inserts the list of studies,
    /// series or images the dicom object found depending on the level of what the object is.
    static void foundMatchCallback(void *callbackData, T_DIMSE_C_FindRQ *request,
                                   int responseCount, T_DIMSE_C_FindRSP *rsp, DcmDataset *responseIdentifiers);

    ///Cancel the current query
    void cancelQuery(T_DIMSE_C_FindRQ *request);

    ///Add the object to the study list if it does not exist
    void addPatientStudy(DICOMTagReader *dicomTagReader);
    /// Adds the dicom object to the series list if it does not exist
    void addSeries(DICOMTagReader *dicomTagReader);
    /// Adds the dicom object to the image list if it does not exist
    void addImage(DICOMTagReader *dicomTagReader);

    ///Converts the response received by the PACS to Query Request Status
    PACSRequestStatus::QueryRequestStatus getDIMSEStatusCodeAsQueryRequestStatus(unsigned int dimseStatusCode);

private:
    T_ASC_PresentationContextID m_presId;
    DicomMask m_dicomMask;
    PacsDevice m_pacsDevice;
    PACSConnection *m_pacsConnection;

    QList<Patient*> m_patientStudyList;
    QList<Series*> m_seriesList;
    QList<Image*> m_imageList;

    // Flag indicating whether the current query should be canceled
    bool m_cancelQuery;
    // Indicates whether we have requested the cancellation of the current query
    bool m_cancelRequestSent;

    // It will indicate from which PACS we have obtained studies, series, images
    DICOMSource m_resultsDICOMSource;

    /// Indicate whether a get from the results list has been made
    bool m_patientStudyListGot;
    bool m_seriesListGot;
    bool m_imageListGot;
};
};
#endif
