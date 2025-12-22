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

#ifndef UDGQQUERYPACSJOB_H
#define UDGQQUERYPACSJOB_H

#include <QList>
#include <QHash>

#include "dicommask.h"
#include "pacsjob.h"
#include "pacsrequeststatus.h"

class QString;

namespace udg {

class Patient;
class Study;
class Series;
class Image;
class QueryPacs;

/**
    Classe que cercar estudis en un dispositiu pacs, creant un nou job utilitzant les threadweaver
  */
class QueryPacsJob : public PACSJob {
    Q_OBJECT
public:
    /// Indica a quin nivell fem la query
    enum QueryLevel { study, series, image };

    /// Class Constructor / Descriptor
    /// To the DICOMMask for each field we want the PACS to return to us
    /// we must have done the set with an empty string for strings or Null for
    /// dates and times, otherwise consult the PACS
    /// will not return the information in this field for studio / series / image,
    /// since the PACS does not return all the fields but only the ones requested
    QueryPacsJob(PacsDevice parameters, DicomMask mask, QueryLevel queryLevel);
    ~QueryPacsJob();

    /// The code for this method is what runs in a new thread
    virtual void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread);

    /// Returns the type of PACSJob that is the object
    PACSJob::PACSJobType getPACSJobType();

    ///Returns the mask on which the query is made
    DicomMask getDicomMask();

    /// Indicates at what level the study, series, image query is made
    QueryLevel getQueryLevel();

    /// Returns the list of found studies that meet the criteria
    /// search. The class requesting the study search results is responsible
    /// to remove returned objects this method
    QList<Patient*> getPatientStudyList();

    /// Returns the list of found series that satisfy the
    /// search criteria. The class requesting the series search results is responsible
    /// to remove returned objects this method
    QList<Series*> getSeriesList();

    /// Returns the list of found images that meet the
    /// search criteria. The class that asks for them
    /// image search results, is responsible for removing
    /// objects returned this method
    QList<Image*> getImageList();

    ///Returns the status of the query
    PACSRequestStatus::QueryRequestStatus getStatus();

    /// Returns a description of the returned state
    /// for consultation at PACS
    QString getStatusDescription();

private:
    ///Requests that the job query be canceled
    void requestCancelJob();

    /// Returns the Query Level as QString to power
    /// generate error messages
    QString getQueryLevelAsQString();

private:
    DicomMask m_mask;
    QueryPacs *m_queryPacs;
    QueryLevel m_queryLevel;

    PACSRequestStatus::QueryRequestStatus m_queryRequestStatus;
};

}  // End namespace UdG

#endif
