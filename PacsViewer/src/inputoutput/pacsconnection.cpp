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

#include "pacsconnection.h"

#include <dimse.h>
#include <ofcond.h>
#include <assoc.h>
#include <QHostInfo>
#include <QStringList>

#include "logging.h"
#include "inputoutputsettings.h"

namespace udg {

PACSConnection::PACSConnection(PacsDevice pacsDevice)
{
    /// dcmtk global variable to avoid dnslookup,
    /// which gives lentitu problems to windows.
    /// TODO: When refactoring this initialization should
    /// stay in a central configuration site for dcmtk.
    dcmDisableGethostbyaddr.set(OFTrue);

    m_pacs = pacsDevice;
    m_associationNetwork = NULL;
    m_associationParameters = NULL;
    m_dicomAssociation = NULL;
}

PACSConnection::~PACSConnection()
{
}

OFCondition PACSConnection::configureEcho()
{
    const char *transferSyntaxes[] = { UID_LittleEndianImplicitTransferSyntax };
    // PresentationContextID always has to be odd
    int presentationContextID = 1;

    return ASC_addPresentationContext(m_associationParameters,
                                      presentationContextID, UID_VerificationSOPClass, transferSyntaxes, DIM_OF(transferSyntaxes));
}

OFCondition PACSConnection::configureFind()
{
    const char *transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    /// It must always be odd, its value is 1 because we only pass a presentation context
    int presentationContextID = 1;

    getTransferSyntaxForFindOrMoveConnection(transferSyntaxes);

    return ASC_addPresentationContext(m_associationParameters,
                                      presentationContextID, UID_FINDStudyRootQueryRetrieveInformationModel, transferSyntaxes,
                                      DIM_OF(transferSyntaxes));
}

OFCondition PACSConnection::configureMove()
{
    T_ASC_PresentationContextID associationPresentationContextID = 1;
    const char *transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL , NULL, NULL, NULL, NULL};

    getTransferSyntaxForFindOrMoveConnection(transferSyntaxes);

    return ASC_addPresentationContext(m_associationParameters,
                                      associationPresentationContextID, UID_MOVEStudyRootQueryRetrieveInformationModel,
                                      transferSyntaxes, DIM_OF(transferSyntaxes) /*number of TransferSyntaxes*/);
}

/// TODO Study if the best default transferSyntax is
/// UID_LittleEndianExplicitTransferSyntax or as the case of move is JPegLossLess
OFCondition PACSConnection::configureStore()
{
    /// Each SOP Class will be proposed in two presentation contexts
    /// (unless the opt_combineProposedTransferSyntaxes global variable is true).
    /// The command line specified a preferred transfer syntax to use.
    ///  This prefered transfer syntax will be proposed in one presentation context
    /// and a set of alternative (fallback) transfer syntaxes
    ///  will be proposed in a different presentation context.

    /// Generally, we prefer to use Explicitly encoded transfer
    /// syntaxes and if running on a Little Endian machine we prefer LittleEndianExplicitTransferSyntax
    /// to BigEndianTransferSyntax. Some SCP implementations will
    /// just select the first transfer syntax they support (this is not part of the standard) so
    /// organise the proposed transfer syntaxes to take advantage of such behaviour.

    ///We indicate that with preferred Transfer syntax we want to use JpegLossless
    const char *preferredTransferSyntax = UID_JPEGProcess14SV1TransferSyntax;

    QList<const char*> fallbackSyntaxes;
    fallbackSyntaxes.append(UID_LittleEndianExplicitTransferSyntax);
    fallbackSyntaxes.append(UID_BigEndianExplicitTransferSyntax);

    fallbackSyntaxes.append(UID_JPEG2000TransferSyntax);
    fallbackSyntaxes.append(UID_JPEG2000LosslessOnlyTransferSyntax);
    fallbackSyntaxes.append(UID_JPEGProcess2_4TransferSyntax);
    fallbackSyntaxes.append(UID_JPEGProcess1TransferSyntax);
    fallbackSyntaxes.append(UID_JPEGProcess14SV1TransferSyntax);
    fallbackSyntaxes.append(UID_JPEGLSLossyTransferSyntax);
    fallbackSyntaxes.append(UID_JPEGLSLosslessTransferSyntax);
    fallbackSyntaxes.append(UID_RLELosslessTransferSyntax);

    /// We add all image transfer SOP classes. as we do not know what modality they are
    /// the images at the same time as preparing the connection are included in all the modalities.
    /// If at the time of connecting we knew what mode it is
    /// the study should only be added to that of the study motality

    /// SopClass or also known as AbstractSyntax is equivalent
    /// with the Move what action we want to do for example
    /// UID_MOVEStudyRootQueryRetrieveInformationModel, in the move case,
    /// in the case of StoreScu, the sopClass we have depends on the type of image
    /// for example we have ComputedRadiographyImageStorage, CTImageStore,
    /// etc .. these sopClass indicate what kind of image we are going to save, that's why otherwise
    /// we know what kind of SOPClass the images we are going to save are
    /// in PACS, we give you a default list that covers the vast majority and more
    /// common SOPClass that exist

    /// With the Abstract syntax or SOPClass we define which operation we want to do,
    /// and with the transfer syntax we indicate with which protocol it will be done
    /// data transfer, LittleEndian, JPegLossLess, etc ..

    /// EVERYTHING If we can get to know the transfer syntax, only
    /// we have to look at the SOPClassUID of each image to send, look
    /// storescu.cc code from line 639
    QStringList sopClasses;
    for (int i = 0; i < numberOfDcmShortSCUStorageSOPClassUIDs; i++)
    {
        // We check that there are no duplicate SOPClas
        if (!sopClasses.contains(QString(dcmShortSCUStorageSOPClassUIDs[i])))
        {
            sopClasses.append(QString(dcmShortSCUStorageSOPClassUIDs[i]));
        }
    }

    OFCondition condition = EC_Normal;
    int presentationContextID = 1;

    /// We create a presentation context for each SOPClass we have,
    /// indicating for each SOPClass which transfer syntax we will use
    /// In the case of the Store with the presentation Context we indicate that
    /// for each type of image we want to save SOPClass with which
    /// transfer syntax we can communicate, then the PACS us
    /// will indicate if he can save this type of SOPClass, and with which
    /// transfer syntax we have to send the image

    foreach (const QString &sopClass, sopClasses)
    {
        // There can be no more than 255 presentation contexts
        if (presentationContextID > 255)
        {
            return ASC_BADPRESENTATIONCONTEXTID;
        }

        // Sop class with preferred transfer syntax
        condition = ASC_addPresentationContext(m_associationParameters,
                                               presentationContextID, qPrintable(sopClass), &preferredTransferSyntax, 1,
                                               ASC_SC_ROLE_DEFAULT);
        // Only odd presentation context id's
        presentationContextID += 2;
        if (!condition.good())
        {
            break;
        }

        if (fallbackSyntaxes.size() > 0)
        {
            if (presentationContextID > 255)
            {
                return ASC_BADPRESENTATIONCONTEXTID;
            }

            // Sop class with fallback transfer syntax
            condition = addPresentationContext(presentationContextID, sopClass, fallbackSyntaxes);
            // Only odd presentation context id's
            presentationContextID += 2;
            if (!condition.good())
            {
                break;
            }
        }
    }

    return condition;
}

OFCondition PACSConnection::addPresentationContext(int presentationContextId, const QString &abstractSyntax, QList<const char*> transferSyntaxList)
{
    // Create an array of supported/possible transfer syntaxes
    const char **transferSyntaxes = new const char*[transferSyntaxList.size()];
    int transferSyntaxCount = 0;

    foreach (const char *transferSyntax, transferSyntaxList)
    {
        transferSyntaxes[transferSyntaxCount++] = transferSyntax;
    }

    OFCondition condition = ASC_addPresentationContext(m_associationParameters,
                                                       presentationContextId, qPrintable(abstractSyntax), transferSyntaxes,
                                                       transferSyntaxCount, ASC_SC_ROLE_DEFAULT);

    delete[] transferSyntaxes;
    return condition;
}

bool PACSConnection::connectToPACS(PACSServiceToRequest pacsServiceToRequest)
{
    /// There are invocations of dcmtk methods that are not checked on
    /// condition that they return, because the code has been looked at and they always return EC_NORMAL
    Settings settings;

    /// Create the parameters of the connection
    OFCondition condition = ASC_createAssociationParameters(&m_associationParameters, ASC_DEFAULTMAXPDU);
    if (!condition.good())
    {
        ERROR_LOG("Error creating association parameters, error description: " + QString(condition.text()));
        return false;
    }

    /// Set calling and called AE titles
    ASC_setAPTitles(m_associationParameters, qPrintable(settings.getValue(InputOutputSettings::LocalAETitle).toString()),
                    qPrintable(m_pacs.getAETitle()), NULL);

    /// Set the security level of the connection in which case we say we do not use any security level
    ASC_setTransportLayerType(m_associationParameters, OFFalse);

    ASC_setPresentationAddresses(m_associationParameters, qPrintable(QHostInfo::localHostName()),
                                 qPrintable(constructPacsServerAddress(pacsServiceToRequest, m_pacs)));

    ///We specify the connection timeout, if with this time we do not receive a response we give an error for time out
    dcmConnectionTimeout.set(settings.getValue(InputOutputSettings::PACSConnectionTimeout).toInt());

    switch (pacsServiceToRequest)
    {
    case Echo:
        condition = configureEcho();
        break;
    case Query:
        condition = configureFind();
        break;
    case RetrieveDICOMFiles:
        condition = configureMove();
        break;
    case SendDICOMFiles:
        condition = configureStore();
        break;
    }

    if (!condition.good())
    {
        ERROR_LOG("An error occurred while setting up the connection. AE Title: "
                  + m_pacs.getAETitle() + ", adress: " +
                  constructPacsServerAddress(pacsServiceToRequest, m_pacs) + ". Descripcio error: " + QString(condition.text()));
        return false;
    }

    /// Inicialitzem l'objecte network però la connexió no s'obre fins
    /// a l'invocacació del mètode ASC_requestAssociation
    m_associationNetwork = initializeAssociationNetwork(pacsServiceToRequest);

    if (m_associationNetwork == NULL)
    {
        ERROR_LOG("An error occurred initializing the connection parameters. AE Title: "
                  + m_pacs.getAETitle() + ", adress: " +
                  constructPacsServerAddress(pacsServiceToRequest, m_pacs));
        return false;
    }

    // Intentem connectar
    condition = ASC_requestAssociation(m_associationNetwork, m_associationParameters, &m_dicomAssociation);

    if (condition.good())
    {
        if (ASC_countAcceptedPresentationContexts(m_associationParameters) == 0)
        {
            ERROR_LOG("The PACS has not accepted any of the Presentation Contexts presented to us. AE Title: "
                      + m_pacs.getAETitle() + ", adress: " +
                      constructPacsServerAddress(pacsServiceToRequest, m_pacs));
            return false;
        }
    }
    else
    {
        ERROR_LOG("An error occurred while trying to connect to the PACS. AE Title: "
                  + m_pacs.getAETitle() + ", adress: " +
                  constructPacsServerAddress(pacsServiceToRequest, m_pacs) + ". Descripcio error: " + QString(condition.text()));

        /// If we have not been able to connect to the PACS and it is a download we will have opened the
        /// port to receive DICOM incoming connections, of course we can download
        /// images because we were unable to connect to the PACS alone ·
        /// bid for the download, we close the local port waiting for incoming connections.
        if (pacsServiceToRequest == RetrieveDICOMFiles)
        {
            disconnect();
        }

        return false;
    }

    return true;
}

void PACSConnection::disconnect()
{
    OFCondition condition = ASC_releaseAssociation(m_dicomAssociation);
    if (condition.bad())
    {
        ERROR_LOG("Could not disconnect from PACS, description error: " + QString(condition.text()));
    }

    condition = ASC_destroyAssociation(&m_dicomAssociation);
    if (condition.bad())
    {
        ERROR_LOG("Error destroying connection to PACS, descripcio error: " + QString(condition.text()));
    }

    /// Destroy the object and close the open socket,
    /// until the object is dropped the socket is not closed
    condition = ASC_dropNetwork(&m_associationNetwork);
    if (condition.bad())
    {
        ERROR_LOG("Error closing incoming connection port, descripcio error: " + QString(condition.text()));
    }

}

QString PACSConnection::constructPacsServerAddress(PACSServiceToRequest pacsServiceToRequest, PacsDevice pacsDevice)
{
    // The format is "server:port"
    QString pacsServerAddress = pacsDevice.getAddress() + ":";

    switch (pacsServiceToRequest)
    {
    case PACSConnection::Query:
    case PACSConnection::RetrieveDICOMFiles:
        pacsServerAddress += QString().setNum(pacsDevice.getQueryRetrieveServicePort());
        break;
    case PACSConnection::SendDICOMFiles:
        pacsServerAddress += QString().setNum(pacsDevice.getStoreServicePort());
        break;
    case PACSConnection::Echo:
        if (pacsDevice.isQueryRetrieveServiceEnabled())
        {
            pacsServerAddress += QString().setNum(pacsDevice.getQueryRetrieveServicePort());
        }
        else if (pacsDevice.isStoreServiceEnabled())
        {
            pacsServerAddress += QString().setNum(pacsDevice.getStoreServicePort());
        }
        else
        {
            ERROR_LOG("Could not configure which port to echo for PACS " + pacsDevice.getAETitle() + " no te cap servei activat");
        }
        break;
    default:
        ERROR_LOG("Could not configure which port to echo in PACS " + pacsDevice.getAETitle() + " perque la modalitat de connexio és invalida");
    }

    INFO_LOG("PACS Adress build:" + pacsServerAddress);

    return pacsServerAddress;
}

T_ASC_Network* PACSConnection::initializeAssociationNetwork(PACSServiceToRequest pacsServiceToRequest)
{
    Settings settings;
    // If it is not a download we indicate port0
    int networkPort = pacsServiceToRequest == RetrieveDICOMFiles ? settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toInt() : 0;
    int timeout = settings.getValue(InputOutputSettings::PACSConnectionTimeout).toInt();
    T_ASC_NetworkRole networkRole = pacsServiceToRequest == RetrieveDICOMFiles ? NET_ACCEPTORREQUESTOR : NET_REQUESTOR;
    T_ASC_Network *associationNetwork;

    OFCondition condition = ASC_initializeNetwork(networkRole, networkPort, timeout, &associationNetwork);
    if (!condition.good())
    {
        ERROR_LOG("Could not initialize network object, despripcio error" + QString(condition.text()));
        return NULL;
    }

    return associationNetwork;
}

void PACSConnection::getTransferSyntaxForFindOrMoveConnection(const char *transferSyntaxes[])
{
    /// We prefer to use Explicitly encoded transfer syntaxes. If we are running on
    /// a Little Endian machine we prefer LittleEndianExplicitTransferSyntax
    /// to BigEndianTransferSyntax. Some SCP implementations will just select the
    /// first transfer syntax they support (this is not part of the standard) so
    /// organise the proposed transfer syntaxes to take advantage of such behaviour.
    /// The presentation presentationContextIDs proposed here are only used for
    /// C-FIND and C-MOVE, so there is no need to support compressed transmission.

    /// gLocalByteOrder is defined in dcxfer.h
    if (gLocalByteOrder == EBO_LittleEndian)
    {
        // We are on a little endian machine
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
    }
    else
    {
        // We are on a big endian machine
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
    }

    transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
    transferSyntaxes[3] = UID_JPEGProcess14SV1TransferSyntax;
    transferSyntaxes[4] = UID_JPEGLSLosslessTransferSyntax;
    transferSyntaxes[5] = UID_JPEG2000LosslessOnlyTransferSyntax;
    transferSyntaxes[6] = UID_JPEG2000TransferSyntax;
    transferSyntaxes[7] = UID_JPEGProcess2_4TransferSyntax;
    transferSyntaxes[8] = UID_JPEGProcess1TransferSyntax;
    transferSyntaxes[9] = UID_JPEGLSLossyTransferSyntax;
    //numTransferSyntaxes = 6;
}

PacsDevice PACSConnection::getPacs()
{
    return m_pacs;
}

T_ASC_Association* PACSConnection::getConnection()
{
    return m_dicomAssociation;
}

T_ASC_Network* PACSConnection::getNetwork()
{
    return m_associationNetwork;
}

}
