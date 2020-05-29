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
    // Variable global de dcmtk per evitar el dnslookup, que dona problemes de lentitu a windows.
    // TODO: Al fer refactoring aquesta inicialització hauria de quedar en un lloc central de configuracions per dcmtk.
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

    return ASC_addPresentationContext(m_associationParameters, presentationContextID, UID_VerificationSOPClass, transferSyntaxes, DIM_OF(transferSyntaxes));
}

OFCondition PACSConnection::configureFind()
{
    const char *transferSyntaxes[] = { NULL, NULL, NULL };
    // Sempre ha de ser imparell, el seu valor és 1 perquè només passem un presentation context
    int presentationContextID = 1;

    getTransferSyntaxForFindOrMoveConnection(transferSyntaxes);

    return ASC_addPresentationContext(m_associationParameters, presentationContextID, UID_FINDStudyRootQueryRetrieveInformationModel, transferSyntaxes,
        DIM_OF(transferSyntaxes));
}

OFCondition PACSConnection::configureMove()
{
    T_ASC_PresentationContextID associationPresentationContextID = 1;
    const char *transferSyntaxes[] = { NULL, NULL, NULL };

    getTransferSyntaxForFindOrMoveConnection(transferSyntaxes);

    return ASC_addPresentationContext(m_associationParameters, associationPresentationContextID, UID_MOVEStudyRootQueryRetrieveInformationModel,
                                      transferSyntaxes, 3 /*number of TransferSyntaxes*/);
}

// TODO Estudiar si el millor transferSyntax per defecte és UID_LittleEndianExplicitTransferSyntax o com els cas del move és el JPegLossLess
OFCondition PACSConnection::configureStore()
{
    // Each SOP Class will be proposed in two presentation contexts (unless the opt_combineProposedTransferSyntaxes global variable is true).
    // The command line specified a preferred transfer syntax to use. This prefered transfer syntax will be proposed in one presentation context
    // and a set of alternative (fallback) transfer syntaxes will be proposed in a different presentation context.

    // Generally, we prefer to use Explicitly encoded transfer syntaxes and if running on a Little Endian machine we prefer LittleEndianExplicitTransferSyntax
    // to BigEndianTransferSyntax. Some SCP implementations will just select the first transfer syntax they support (this is not part of the standard) so
    // organise the proposed transfer syntaxes to take advantage of such behaviour.

    /// Indiquem que con Transfer syntax preferida volem utilitzar JpegLossless
    const char *preferredTransferSyntax = UID_JPEGProcess14SV1TransferSyntax;

    QList<const char*> fallbackSyntaxes;
    fallbackSyntaxes.append(UID_LittleEndianExplicitTransferSyntax);
    fallbackSyntaxes.append(UID_BigEndianExplicitTransferSyntax);
    fallbackSyntaxes.append(UID_LittleEndianImplicitTransferSyntax);

    // Afegim totes les classes SOP de transfarència d'imatges. com que desconeixem de quina modalitat són
    // les imatges alhora de preparar la connexió les hi incloem totes les modalitats. Si alhora de connectar sabèssim de quina modalitat és
    // l'estudi només caldria afegir-hi la de la motalitat de l'estudi

    // Les sopClass o també conegudes com AbstractSyntax és equivalent amb el Move quina acció volem fer per exemple
    // UID_MOVEStudyRootQueryRetrieveInformationModel, en el case del move, en el cas de StoreScu, el sopClass que tenim van en funció del tipus d'imatge
    // per exemple tenim ComputedRadiographyImageStorage, CTImageStore, etc.. aquestes sopClass indiquen  quin tipus d'imatge anirem a guardar, per això sinó
    // sabem de quin tipus de SOPClass són les imatges que anem a guardar al PACS, li indiquem una llista per defecte que cobreix la gran majoria i més
    // comuns de SOPClass que existeixen

    // Amb l'Abstract syntax o SOPClass definim quina operació volem fer, i amb els transfer syntax indiquem amb quin protocol es farà
    // la transfarència de les dades, LittleEndian, JPegLossLess, etc..

    // TODO Si que la podem arribar a saber la transfer syntax, només hem de mirar la SOPClassUID de cada imatge a enviar, mirar
    // codi storescu.cc a partir de la línia 639
    QStringList sopClasses;
    for (int i = 0; i < numberOfDcmShortSCUStorageSOPClassUIDs; i++)
    {
        // Comprovem que no hi hagi que cap SOPClas duplicada
        if (!sopClasses.contains(QString(dcmShortSCUStorageSOPClassUIDs[i])))
        {
            sopClasses.append(QString(dcmShortSCUStorageSOPClassUIDs[i]));
        }
    }

    OFCondition condition = EC_Normal;
    int presentationContextID = 1;

    // Creem un presentation context per cada SOPClass que tinguem, indicant per cada SOPClass quina transfer syntax utilitzarem
    // En el cas del Store amb el presentation Context indiquem que per cada tipus d'imatge que volem guardar SOPClass amb quins
    // transfer syntax ens podem comunicar, llavors el PACS ens indicarà si ell pot guardar aquest tipus de SOPClass, i amb quin
    // transfer syntax li hem d'enviar la imatge

    foreach (const QString &sopClass, sopClasses)
    {
        // No poden haver més de 255 presentation context
        if (presentationContextID > 255)
        {
            return ASC_BADPRESENTATIONCONTEXTID;
        }

        // Sop class with preferred transfer syntax
        condition = ASC_addPresentationContext(m_associationParameters, presentationContextID, qPrintable(sopClass), &preferredTransferSyntax, 1,
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

    OFCondition condition = ASC_addPresentationContext(m_associationParameters, presentationContextId, qPrintable(abstractSyntax), transferSyntaxes,
        transferSyntaxCount, ASC_SC_ROLE_DEFAULT);

    delete[] transferSyntaxes;
    return condition;
}

bool PACSConnection::connectToPACS(PACSServiceToRequest pacsServiceToRequest)
{
    // Hi ha invocacions de mètodes de dcmtk que no se'ls hi comprova el condition que retornen, perquè se'ls hi ha mirat el codi i sempre retornen EC_NORMAL
    Settings settings;

    // Create the parameters of the connection
    OFCondition condition = ASC_createAssociationParameters(&m_associationParameters, ASC_DEFAULTMAXPDU);
    if (!condition.good())
    {
        ERROR_LOG("Error al crear els parametres de l'associacio, descripcio error: " + QString(condition.text()));
        return false;
    }

    // Set calling and called AE titles
    ASC_setAPTitles(m_associationParameters, qPrintable(settings.getValue(InputOutputSettings::LocalAETitle).toString()), qPrintable(m_pacs.getAETitle()),
                    NULL);

    // Defineix el nivell de seguretat de la connexió en aquest cas diem que no utilitzem cap nivell de seguretat
    ASC_setTransportLayerType(m_associationParameters, OFFalse);

    ASC_setPresentationAddresses(m_associationParameters, qPrintable(QHostInfo::localHostName()),
        qPrintable(constructPacsServerAddress(pacsServiceToRequest, m_pacs)));

    // Especifiquem el timeout de connexió, si amb aquest temps no rebem resposta donem error per time out
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
        ERROR_LOG("S'ha produit un error al configurar la connexio. AE Title: " + m_pacs.getAETitle() + ", adreca: " +
            constructPacsServerAddress(pacsServiceToRequest, m_pacs) + ". Descripcio error: " + QString(condition.text()));
        return false;
    }

    // Inicialitzem l'objecte network però la connexió no s'obre fins a l'invocacació del mètode ASC_requestAssociation
    m_associationNetwork = initializeAssociationNetwork(pacsServiceToRequest);

    if (m_associationNetwork == NULL)
    {
        ERROR_LOG("S'ha produit un error inicialitzant els parametres de la connexio. AE Title: " + m_pacs.getAETitle() + ", adreca: " +
            constructPacsServerAddress(pacsServiceToRequest, m_pacs));
        return false;
    }

    // Intentem connectar
    condition = ASC_requestAssociation(m_associationNetwork, m_associationParameters, &m_dicomAssociation);

    if (condition.good())
    {
        if (ASC_countAcceptedPresentationContexts(m_associationParameters) == 0)
        {
            ERROR_LOG("El PACS no ens ha acceptat cap dels Presentation Context presentats. AE Title: " + m_pacs.getAETitle() + ", adreca: " +
                constructPacsServerAddress(pacsServiceToRequest, m_pacs));
            return false;
        }
    }
    else
    {
        ERROR_LOG("S'ha produit un error al intentar connectar amb el PACS. AE Title: " + m_pacs.getAETitle() + ", adreca: " +
            constructPacsServerAddress(pacsServiceToRequest, m_pacs) + ". Descripcio error: " + QString(condition.text()));

        // Si no hem pogut connectar al PACS i és una descàrrega haurem obert el port per rebre connexions entrants DICOM, com no que podrem descarregar
        // les imatges perquè no hem pogut connectar amb el PACS per sol·licitar-ne la descarrega, tanquem el port local que espera per connexions entrants.
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
        ERROR_LOG("No s'ha pogut desconnectar del PACS, descripcio error: " + QString(condition.text()));
    }

    condition = ASC_destroyAssociation(&m_dicomAssociation);
    if (condition.bad())
    {
        ERROR_LOG("Error al destruir la connexio amb el PACS, descripcio error: " + QString(condition.text()));
    }

    // Destrueix l'objecte i tanca el socket obert, fins que no es fa el drop de l'objecte no es tanca el socket
    condition = ASC_dropNetwork(&m_associationNetwork);
    if (condition.bad())
    {
        ERROR_LOG("Error al tancar el port de connexions entrants, descripcio error: " + QString(condition.text()));
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
                ERROR_LOG("No s'ha pogut configurar per quin port fer l'echo perque el PACS " + pacsDevice.getAETitle() + " no te cap servei activat");
            }
            break;
        default:
            ERROR_LOG("No s'ha pogut configurar per quin port fer l'echo al PACS " + pacsDevice.getAETitle() + " perque la modalitat de connexio és invalida");
    }

    INFO_LOG("PACS Adress build:" + pacsServerAddress);

    return pacsServerAddress;
}

T_ASC_Network* PACSConnection::initializeAssociationNetwork(PACSServiceToRequest pacsServiceToRequest)
{
    Settings settings;
    // Si no es tracta d'una descarrega indiquem port 0
    int networkPort = pacsServiceToRequest == RetrieveDICOMFiles ? settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toInt() : 0;
    int timeout = settings.getValue(InputOutputSettings::PACSConnectionTimeout).toInt();
    T_ASC_NetworkRole networkRole = pacsServiceToRequest == RetrieveDICOMFiles ? NET_ACCEPTORREQUESTOR : NET_REQUESTOR;
    T_ASC_Network *associationNetwork;

    OFCondition condition = ASC_initializeNetwork(networkRole, networkPort, timeout, &associationNetwork);
    if (!condition.good())
    {
        ERROR_LOG("No s'ha pogut inicialitzar l'objecte network, despripcio error" + QString(condition.text()));
        return NULL;
    }

    return associationNetwork;
}

void PACSConnection::getTransferSyntaxForFindOrMoveConnection(const char *transferSyntaxes[3])
{
    // We prefer to use Explicitly encoded transfer syntaxes. If we are running on a Little Endian machine we prefer LittleEndianExplicitTransferSyntax
    // to BigEndianTransferSyntax. Some SCP implementations will just select the first transfer syntax they support (this is not part of the standard) so
    // organise the proposed transfer syntaxes to take advantage of such behaviour.
    // The presentation presentationContextIDs proposed here are only used for C-FIND and C-MOVE, so there is no need to support compressed transmission.

    // gLocalByteOrder is defined in dcxfer.h
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
