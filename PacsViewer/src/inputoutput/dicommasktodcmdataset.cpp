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

#include "dicommasktodcmdataset.h"

#include <dimse.h>
#include <dcsequen.h>
#include <dcdeftag.h>

#include "dicommask.h"

namespace udg
{

DcmDataset* DicomMaskToDcmDataset::getDicomMaskAsDcmDataset(const DicomMask &dicomMask)
{
    DcmDataset *maskDcmDataset = new DcmDataset();

    /// We specify that by default the Specific character set used to make queries to the PACS will be ISO_IR 100, ie Latin 1, since the PACS
    /// that uses the IDI use this coding (it is the one that supports dcm4chee), in addition with Latin1 is the coding that uses
    /// most European countries. For questions see C.12.1.1.2 where you specify which Specific characters set, is also important
    /// consult the conformance statement of the PACS against which we consult to know which Specific character set it supports. Because the character set is Latin1
    /// we will have to transform all dicom tags that are string (SH, LO, ST, PN, LT, UT) to Latin1
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_SpecificCharacterSet, "ISO_IR 100");

    /// We specify at what level the QueryRetrieve is done
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_QueryRetrieveLevel, getQueryLevelFromDICOMMask(dicomMask));

    ///We add the study tags
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_PatientID, dicomMask.getPatientID());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_PatientName, dicomMask.getPatientName());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_PatientBirthDate, dicomMask.getPatientBirthRangeAsDICOMFormat());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_PatientSex, dicomMask.getPatientSex());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_PatientAge, dicomMask.getPatientAge());

    ///We add the serial tags
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_StudyInstanceUID, dicomMask.getStudyInstanceUID());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_StudyID, dicomMask.getStudyID());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_StudyDescription, dicomMask.getStudyDescription());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_ModalitiesInStudy, dicomMask.getStudyModality());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_StudyDate, dicomMask.getStudyDateRangeAsDICOMFormat());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_StudyTime, dicomMask.getStudyTimeRangeAsDICOMFormat());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_AccessionNumber, dicomMask.getAccessionNumber());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_ReferringPhysicianName, dicomMask.getReferringPhysiciansName());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_SeriesNumber, dicomMask.getSeriesNumber());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_SeriesDate, dicomMask.getSeriesDateRangeAsDICOMFormat());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_Modality, dicomMask.getSeriesModality());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_SeriesTime, dicomMask.getSeriesTimeRangeAsDICOMFormat());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_SeriesDescription, dicomMask.getSeriesDescription());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_ProtocolName, dicomMask.getSeriesProtocolName());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_SeriesInstanceUID, dicomMask.getSeriesInstanceUID());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_PerformedProcedureStepStartDate, dicomMask.getPPSStartDateAsRangeDICOMFormat());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_PerformedProcedureStepStartTime, dicomMask.getPPSStartTimeAsRangeDICOMFormat());

    if (!dicomMask.getRequestedProcedureID().isNull() || !dicomMask.getScheduledProcedureStepID().isNull())
    {
        DcmItem *requestedAttributeSequenceItem = new DcmItem(DCM_Item);
        requestedAttributeSequenceItem->putAndInsertString(DCM_RequestedProcedureID, dicomMask.getRequestedProcedureID().toLatin1().data());
        requestedAttributeSequenceItem->putAndInsertString(DCM_ScheduledProcedureStepID, dicomMask.getScheduledProcedureStepID().toLatin1().data());

        DcmSequenceOfItems *requestedAttributeSequence = new DcmSequenceOfItems(DCM_RequestAttributesSequence);
        requestedAttributeSequence->insert(requestedAttributeSequenceItem);
        maskDcmDataset->insert(requestedAttributeSequence, OFTrue);
    }

    ///We add image tags
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_SOPInstanceUID, dicomMask.getSOPInstanceUID());
    addTagToDcmDatasetAsString(maskDcmDataset, DCM_InstanceNumber, dicomMask.getImageNumber());

    return maskDcmDataset;
}

void DicomMaskToDcmDataset::addTagToDcmDatasetAsString(DcmDataset *dcmDataset, const DcmTagKey &dcmTagKey, const QString &tagValue)
{
    if (!tagValue.isNull())
    {
#ifdef  PACKAGE_VERSION_NUMBER
#if PACKAGE_VERSION_NUMBER == 361
        DcmElement *elem = newDicomElement(dcmTagKey);
#else //if  PACKAGE_VERSION_NUMBER == 363
        DcmElement *elem = DcmItem::newDicomElement(dcmTagKey);
#endif
#endif
        elem->putString(tagValue.toLatin1().data());
        dcmDataset->insert(elem, OFTrue);
    }
}

QString DicomMaskToDcmDataset::getQueryLevelFromDICOMMask(const DicomMask &dicomMask) const
{
    bool isImageLevel = !dicomMask.getSOPInstanceUID().isNull() || !dicomMask.getImageNumber().isNull();
    bool isSeriesLevel = !dicomMask.getSeriesDescription().isNull() || !dicomMask.getSeriesDateRangeAsDICOMFormat().isNull() || !dicomMask.getSeriesModality().isNull() ||
            !dicomMask.getSeriesNumber().isNull() || !dicomMask.getSeriesProtocolName().isNull() || !dicomMask.getSeriesTimeRangeAsDICOMFormat().isNull() ||
            !dicomMask.getSeriesInstanceUID().isNull() || !dicomMask.getRequestedProcedureID().isNull() || !dicomMask.getScheduledProcedureStepID().isNull() ||
            !dicomMask.getPPSStartDateAsRangeDICOMFormat().isNull() || !dicomMask.getPPSStartTimeAsRangeDICOMFormat().isNull();

    if (isImageLevel)
    {
        return "IMAGE";
    }
    else if (isSeriesLevel)
    {
        return "SERIES";
    }
    else
    {
        ///By default they are at least study level
        return "STUDY";
    }
}

}
