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
/// This class helps to interactive with the pacs, allow us to find studies in the pacs setting a search mask. Very important for this class a connection
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
    /// Constructor de la classe
    QueryPacs(PacsDevice pacsDevice);
    ~QueryPacs();

    /// Cerca els estudis que compleixin la màscara passada
    PACSRequestStatus::QueryRequestStatus query(const DicomMask &mask);

    /// Indiquem que la consulta actual s'ha de cancel·lar.
    /// La cancel·lació de la query no es fa immediatament quan s'invoca el mètode, aquest mètode actualitza un flag, que cada vegada
    /// que rebem un element DICOM que compleix la màscara es comprova, si el flag indica que s'ha demanat cancel·lar llavors es
    /// cancel·la la query
    void cancelQuery();

    ///Retornen els pacients amb els estudis trobats. La classe que demani els resultats de cerca d'estudis, és responsable d'eliminar els objects retornats aquest mètode
    QList<Patient*> getQueryResultsAsPatientStudyList();
    ///Retornen les sèries trobades. La classe que demani els resultats de cerca de sèries, és responsable d'eliminar els objects retornats aquest mètode
    QList<Series*> getQueryResultsAsSeriesList();
    ///Retornen les imatges trobades. La classe que demani els resultats de cerca d'imatge, és responsable d'eliminar els objects retornats aquest mètode
    QList<Image*> getQueryResultsAsImageList();

private:
    /// Fa el query al pacs
    PACSRequestStatus::QueryRequestStatus query();

    /// Aquest és un mètode que és cridat en callback per les dcmtk, per cada objecte dicom que es trobi en el PACS que compleix la query dcmtk el crida.
    /// Aquest mètode ens insereix la llista d'estudis, sèries o imatges l'objecte dicom trobat en funció del nivell del que sigui l'objecte.
    static void foundMatchCallback(void *callbackData, T_DIMSE_C_FindRQ *request, int responseCount, T_DIMSE_C_FindRSP *rsp, DcmDataset *responseIdentifiers);

    /// Cancel·la la consulta actual
    void cancelQuery(T_DIMSE_C_FindRQ *request);

    /// Afegeix l'objecte a la llista d'estudis si no hi existeix
    void addPatientStudy(DICOMTagReader *dicomTagReader);
    /// Afegeix l'objecte dicom a la llista de sèries si no hi existeix
    void addSeries(DICOMTagReader *dicomTagReader);
    /// Afegeix l'objecte dicom a la llista d'imatges si no hi existeix
    void addImage(DICOMTagReader *dicomTagReader);

    /// Converteix la respota rebuda per partl del PACS a QueryRequestStatus
    PACSRequestStatus::QueryRequestStatus getDIMSEStatusCodeAsQueryRequestStatus(unsigned int dimseStatusCode);

private:
    T_ASC_PresentationContextID m_presId;
    DicomMask m_dicomMask;
    PacsDevice m_pacsDevice;
    PACSConnection *m_pacsConnection;

    QList<Patient*> m_patientStudyList;
    QList<Series*> m_seriesList;
    QList<Image*> m_imageList;

    // Flag que indica si s'ha de cancel·lar la query actual
    bool m_cancelQuery;
    // Indica si hem demanat la cancel·lació de la consulta actual
    bool m_cancelRequestSent;

    // Indicarà de quin PACS hem obtingut estudis, sèries, imatges
    DICOMSource m_resultsDICOMSource;

    /// Indiquen si s'ha fet un get de la llista de resultats 
    bool m_patientStudyListGot;
    bool m_seriesListGot;
    bool m_imageListGot;
};
};
#endif
