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

#ifndef PACSCONNECTION
#define PACSCONNECTION

#include "pacsdevice.h"

struct T_ASC_Network;
struct T_ASC_Parameters;
struct T_ASC_Association;
class OFCondition;
class QString;

namespace udg {

/**
This class is responsible for setting up the connection and connecting
with the PACS depending on the service we want to request.
*/
class PACSConnection {

public:
    enum PACSServiceToRequest { Query, RetrieveDICOMFiles, Echo, SendDICOMFiles };

    /// Class builder. A PacsDevice object must be passed to it,
    ///  with the pacs parameters correctly specified
    ///  @param Pacs parameters to connect
    PACSConnection(PacsDevice pacsDevice);
    virtual ~PACSConnection();

    /// This function tries to connect us to the PACS
    /// @param Specifies in which mode we want to connect, echo,
    /// search for information or download images
    /// @return returns the status of the connection
    virtual bool connectToPACS(PACSServiceToRequest pacsServiceToRequest);

    /// Returns the PACS parameters
    /// @return Pacs parameters
    PacsDevice getPacs();

    /// Returns a connection to be able to search for information, or download images
    /// @return returns a PACS connection
    T_ASC_Association* getConnection();

    /// Returns a network configuration. It is only necessary when the goal
    /// of the connection is to download images
    /// @return returns the network settings
    T_ASC_Network* getNetwork();

    /// This action close the session with PACS's machine and release all the resources
    void disconnect();

private:
    /// This function is private. It is used to specify in the PACS,
    /// that one of the possible operations we want to do with him is an echo. By default
    /// in any mode of connection we can make an echo
    /// @return returns the status of the configuration
    OFCondition configureEcho();

    /// This private function configures the connection parameters to specify,
    /// that the reason for our connection is to search for information.
    /// @return returns the status of the configuration
    OFCondition configureFind();

    /// This private function allows you to configure the connection to download
    /// images on the local computer. IMPORTANT !!! It must be invoked before connecting
    /// the setLocalhostPort function
    /// @return returns the configuration stateDUL_PRESENTATIONCONTEXTID
    OFCondition configureMove();

    /// This private function allows you to configure the connection to save studies in the pacs.
    /// @return returns the status of the configuration
    OFCondition configureStore();

    /// Construct the server address in ip: port format, to connect to the PACS
    /// @param server address
    /// @param server port
    QString constructPacsServerAddress(PACSServiceToRequest pacsServiceToRequest, PacsDevice pacsDevice);

    /// Add a SOP object to the connection to the PACS
    /// @param presentationContextId pid number
    /// @param abstractSyntax SOP class to add
    /// @param transferSyntaxList
    /// @return method state
    OFCondition addPresentationContext(int presentationContextId, const QString &abstractSyntax, QList<const char*> transferSyntaxList);

    /// This method initializes the AssociationNetwork object based on
    /// mode with PACS parameters, this method does not open the connection
    /// simply initializes the object with the data needed to be able to
    /// open connection, who opens the connection is by invoking the method
    /// from dcmtk ASC_requestAssociation within the connect connect () method;
    T_ASC_Network* initializeAssociationNetwork(PACSServiceToRequest modality);

    /// Fill the array passed by parameters with the syntax transfer to
    /// use for connections to make FIND or Move
    void getTransferSyntaxForFindOrMoveConnection(const char *transferSyntaxes[6]);

private:
    PacsDevice m_pacs;
    /// network struct, contains DICOM upper layer FSM etc. At the DICOM level
    // is nothing is a DCMTK object of its own, contains connection parameters and in the case
    /// image download tells you which port we listen to DICOM requests.
    T_ASC_Network *m_associationNetwork;
    /// Defines the association parameters to be used by the
    /// communication between Starviewer and PACS, contains PACS address, connection type, ....
    T_ASC_Parameters *m_associationParameters;
    /// The association is the communication channel used for the exchange
    /// of information between DICOM devices (it is the connection with the PACS)
    T_ASC_Association *m_dicomAssociation;
};
};
#endif
