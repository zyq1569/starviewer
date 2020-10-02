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


#ifndef UDGPACSREQUESTATUS_H
#define UDGPACSREQUESTATUS_H

namespace udg {

/**
    Class that defines the Enum with the errors that can return to us the different Jobs that communicate with the PACS
  */
class PACSRequestStatus {

public:
    /// SendSomeDICOMFilesFailed - Some images failed to send
    /// SendWarningForSomeImages: For StoreSCU it indicates that all the images have been sent but for all or some image we have received a warning, for example
    /// we can receive a warning because the PACS has saved the images with a different syntax transfer than we had sent them.
    /// SendAllDICOMFilesFailed: All images failed to send
    enum SendRequestStatus { SendOk, SendCanNotConnectToPACS, SendAllDICOMFilesFailed, SendWarningForSomeImages, SendSomeDICOMFilesFailed, SendCancelled,
                             SendUnknowStatus, SendPACSConnectionBroken };

    /// Defines the types of errors we may have when downloading DICOM files:
    /// RetrieveDatabaseError: Indicates when Starviewer database error
    /// MoveDestinationAETileUnknownStatus: PACS does not have our AETitle registered to allow you to download
    /// MoveWarningStatus: Failed to download any of the requested files
    /// RetrieveIncomingDICOMConnectionsPortInUse: The port to receive incoming connections to receive files is in use
    enum RetrieveRequestStatus { RetrieveOk, RetrieveDatabaseError, RetrieveCanNotConnectToPACS, RetrieveNoEnoughSpace, RetrieveErrorFreeingSpace,
                                 RetrievePatientInconsistent, RetrieveDestinationAETileUnknown, RetrieveIncomingDICOMConnectionsPortInUse,
                                 RetrieveFailureOrRefused, RetrieveSomeDICOMFilesFailed, RetrieveCancelled, RetrieveUnknowStatus };

    ///Errors that can occur when doing Queries in PACS
    enum QueryRequestStatus { QueryOk, QueryCanNotConnectToPACS, QueryFailedOrRefused, QueryCancelled, QueryUnknowStatus };

};

}  //  end  namespace udg

#endif
