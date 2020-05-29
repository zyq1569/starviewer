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

#include "senddicomfilestopacs.h"

#include <diutil.h>
#include <dcfilefo.h>
#include <assoc.h>
#include <dctagkey.h>
#include <dcdeftag.h>

#include <QDir>
#include <QSet>

#include "logging.h"
#include "image.h"
#include "pacsconnection.h"
#include "pacsrequeststatus.h"
#include "inputoutputsettings.h"

namespace udg {

SendDICOMFilesToPACS::SendDICOMFilesToPACS(PacsDevice pacsDevice)
 : DIMSECService()
{
    m_pacs = pacsDevice;
    m_abortIsRequested = false;

    this->setUpAsCStore();
}

PacsDevice SendDICOMFilesToPACS::getPacs()
{
    return m_pacs;
}

PACSRequestStatus::SendRequestStatus SendDICOMFilesToPACS::send(QList<Image*> imageListToSend)
{
    QScopedPointer<PACSConnection> pacsConnection(createPACSConnection(m_pacs));
    // TODO: S'hauria de comprovar que es tracti d'un PACS amb el servei d'store configurat
    if (!pacsConnection->connectToPACS(PACSConnection::SendDICOMFiles))
    {
        ERROR_LOG(" S'ha produit un error al intentar connectar al PACS per fer un send. AE Title: " + m_pacs.getAETitle());
        return PACSRequestStatus::SendCanNotConnectToPACS;
    }

    removeDuplicateFiles(imageListToSend);
    initialitzeDICOMFilesCounters(imageListToSend.count());

    foreach (Image *imageToStore, imageListToSend)
    {
        if (m_abortIsRequested)
        {
            break;
        }

        INFO_LOG(QString("S'enviara al PACS %1 el fitxer %2").arg(m_pacs.getAETitle(), imageToStore->getPath()));
        if (storeSCU(pacsConnection->getConnection(), qPrintable(imageToStore->getPath())))
        {
            emit DICOMFileSent(imageToStore, getNumberOfDICOMFilesSentSuccesfully() + this->getNumberOfDICOMFilesSentWarning());
        }
        else if (m_lastOFCondition == DIMSE_SENDFAILED)
        {
            // Si se'ns retorna un OFCondition == DIMSE_SENDFAILED, indica que s'ha perdut la connexió amb el PACS
            break;
        }
    }

    pacsConnection->disconnect();

    return getStatusStoreSCU();
}

void SendDICOMFilesToPACS::requestCancel()
{
    m_abortIsRequested = true;
    INFO_LOG("Ens han demanat cancel·lar l'enviament dels fitxers al PACS");
}

PACSConnection* SendDICOMFilesToPACS::createPACSConnection(const PacsDevice &pacsDevice) const
{
    return new PACSConnection(pacsDevice);
}

void SendDICOMFilesToPACS::removeDuplicateFiles(QList<Image*> &imageList) const
{
    QSet<QString> paths;
    QMutableListIterator<Image*> it(imageList);

    while (it.hasNext())
    {
        Image *image = it.next();

        if (paths.contains(image->getPath()))
        {
            it.remove();
        }
        else
        {
            paths.insert(image->getPath());
        }
    }
}

void SendDICOMFilesToPACS::initialitzeDICOMFilesCounters(int numberOfDICOMFilesToSend)
{
    // Inicialitzem els comptadors
    m_numberOfDICOMFilesSentSuccessfully = 0;
    m_numberOfDICOMFilesSentWithWarning = 0;
    m_numberOfDICOMFilesToSend = numberOfDICOMFilesToSend;
}

// This function will read all the information from the given file,
// figure out a corresponding presentation context which will be used
// to transmit the information over the network to the SCP, and it
// will finally initiate the transmission of all data to the SCP.
//
// Parameters:
//   association - [in] The associationiation (network connection to another DICOM application).
//   filepathToStore - [in] Name of the file which shall be processed.
bool SendDICOMFilesToPACS::storeSCU(T_ASC_Association *association, QString filepathToStore)
{
    DIC_US msgId = association->nextMsgID++;
    T_ASC_PresentationContextID presentationContextID;
    T_DIMSE_C_StoreRQ request;
    T_DIMSE_C_StoreRSP response;
    DIC_UI sopClass;
    DIC_UI sopInstance;
    DcmDataset *statusDetail = NULL;
    DcmFileFormat dcmff;

    m_lastOFCondition = dcmff.loadFile(qPrintable(QDir::toNativeSeparators(filepathToStore)));

    // Figure out if an error occured while the file was read
    if (m_lastOFCondition.bad())
    {
        ERROR_LOG("No s'ha pogut obrir el fitxer " + filepathToStore);
        return false;
    }
    // Figure out which SOP class and SOP instance is encapsulated in the file
    if (!DU_findSOPClassAndInstanceInDataSet(dcmff.getDataset(), sopClass, sopInstance, OFFalse))
    {
        ERROR_LOG("No s'ha pogut obtenir el SOPClass i SOPInstance del fitxer " + filepathToStore);
        return false;
    }

    // Figure out which of the accepted presentation contexts should be used
    DcmXfer filexfer(dcmff.getDataset()->getOriginalXfer());

    // Busquem dels presentationContextID que hem establert al connectar quin és el que hem d'utilitzar per transferir aquesta imatge
    if (filexfer.getXfer() != EXS_Unknown)
    {
        presentationContextID = ASC_findAcceptedPresentationContextID(association, sopClass, filexfer.getXferID());
    }
    else
    {
        presentationContextID = ASC_findAcceptedPresentationContextID(association, sopClass);
    }

    if (presentationContextID == 0)
    {
        // No hem trobat cap presentation context vàlid dels que hem configuarat a la connexió pacsserver.cpp
        const char *modalityName = dcmSOPClassUIDToModality(sopClass);

        if (!modalityName)
        {
            modalityName = dcmFindNameOfUID(sopClass);
        }

        if (!modalityName)
        {
            modalityName = "unknown SOP class";
        }

        ERROR_LOG("No s'ha trobat un presentation context vàlid en la connexió per la modalitat : " + QString(modalityName)
                   + " amb la SOPClass " + QString(sopClass) + " pel fitxer " + filepathToStore);

        return false;
    }
    else
    {
        // Prepare the transmission of data
        bzero((char*)&request, sizeof(request));
        request.MessageID = msgId;
        strcpy(request.AffectedSOPClassUID, sopClass);
        strcpy(request.AffectedSOPInstanceUID, sopInstance);
        request.DataSetType = DIMSE_DATASET_PRESENT;
        request.Priority = DIMSE_PRIORITY_LOW;

        m_lastOFCondition = DIMSE_storeUser(association, presentationContextID, &request, NULL /*imageFileName*/, dcmff.getDataset(),
                                            NULL /*progressCallback*/, NULL /*callbackData */, DIMSE_NONBLOCKING,
                                            Settings().getValue(InputOutputSettings::PACSConnectionTimeout).toInt(), &response, &statusDetail,
                                            NULL /*check for cancel parameters*/, OFStandard::getFileSize(qPrintable(filepathToStore)));

        if (m_lastOFCondition.bad())
        {
            ERROR_LOG("S'ha produit un error al fer el store de la imatge " + filepathToStore + ", descripció de l'error" + QString(m_lastOFCondition.text()));
        }

        processResponseFromStoreSCP(response.DimseStatus, filepathToStore);
        processServiceClassProviderResponseStatus(response.DimseStatus, statusDetail);

        if (statusDetail != NULL)
        {
            delete statusDetail;
        }

        return m_lastOFCondition.good() && response.DimseStatus == STATUS_Success;
    }
}

void SendDICOMFilesToPACS::processResponseFromStoreSCP(unsigned int dimseStatusCode, QString filePathDicomObjectStoredFailed)
{
    QString messageErrorLog = "No s'ha pogut enviar el fitxer " + filePathDicomObjectStoredFailed + ", descripció error rebuda";

    // A la secció B.2.3, taula B.2-1 podem trobar un descripció dels errors.
    // Per a detalls sobre els "related fields" consultar PS 3.7, Annex C - Status Type Enconding

    // Tenir en compte també que el significat dels Status és diferent que els de MoveScu.
    //      - Failure la imatgen o s'ha pogut pujar
    //      - Warning la imatge s'ha pujat, però no condorcada la SOPClass, s'ha fet coerció d'algunes dades...

    if (dimseStatusCode == STATUS_Success)
    {
        // La imatge s'ha enviat correctament
        m_numberOfDICOMFilesSentSuccessfully++;
        return;
    }

    switch (dimseStatusCode)
    {
        case STATUS_STORE_Refused_OutOfResources:
            // 0xA7XX
        case STATUS_STORE_Refused_SOPClassNotSupported:
            // 0x0122
        case STATUS_STORE_Error_DataSetDoesNotMatchSOPClass:
            // 0xA9XX
        case STATUS_STORE_Error_CannotUnderstand:
            // 0xCXXX
            ERROR_LOG(messageErrorLog + QString(DU_cstoreStatusString(dimseStatusCode)));
            break;
        
        // Coersió entre tipus, s'ha convertit un tipus a un altre tipus i es pot haver perdut dades, per exemple passar de decimal a enter, tot i així
        // els fitxers s'han enviat i guardat
        case STATUS_STORE_Warning_CoersionOfDataElements:
            // 0xB000
        case STATUS_STORE_Warning_DataSetDoesNotMatchSOPClass:
            // 0xB007
        case STATUS_STORE_Warning_ElementsDiscarded:
            // 0xB006
            ERROR_LOG(messageErrorLog + QString(DU_cstoreStatusString(dimseStatusCode)));
            m_numberOfDICOMFilesSentWithWarning++;
            break;
        
        default:
            // S'ha produït un error no contemplat. En principi no s'hauria d'arribar mai a aquesta branca
            ERROR_LOG(messageErrorLog + QString(DU_cstoreStatusString(dimseStatusCode)));
            break;
    }
}

PACSRequestStatus::SendRequestStatus SendDICOMFilesToPACS::getStatusStoreSCU()
{
    // El tractament d'erros d'StoreSCU és diferent del moveSCU, en moveSCU rebem un status final indicant com ha anat l'operació, mentre que
    // en storeSCU per cada imatge que s'envia és rep un status, com podem tenir al enviar un estudi, status failure, warning, ..., cap a l'usuari
    // només enviarem un error i mostrarem el més crític, per exemple si tenim 5 errors Warning i un de Failure, enviarem error indica que l'enviament
    // d'algunes imatges ha fallat.

    if (m_abortIsRequested)
    {
        INFO_LOG("S'ha abortat l'enviament d'imatges al PACS");
        return PACSRequestStatus::SendCancelled;
    }
    else if (m_lastOFCondition == DIMSE_SENDFAILED)
    {
        ERROR_LOG("S'ha perdut la connexio amb el PACS mentre s'enviaven els fitxers");
        return PACSRequestStatus::SendPACSConnectionBroken;
    }
    else if (getNumberOfDICOMFilesSentSuccesfully() == 0)
    {
        // No hem guardat cap imatge (Failure Status)
        ERROR_LOG("Ha fallat l'enviament de tots els fitxers al PACS");
        return PACSRequestStatus::SendAllDICOMFilesFailed;
    }
    else if (getNumberOfDICOMFilesSentFailed() > 0)
    {
        // No s'han pogut guardar els fitxers
        ERROR_LOG(QString("L'enviament al PACS de %1 de %2 fitxers ha fallat")
                     .arg(QString().setNum(getNumberOfDICOMFilesSentFailed()), QString().setNum(m_numberOfDICOMFilesToSend)));
        return PACSRequestStatus::SendSomeDICOMFilesFailed;
    }
    else if (getNumberOfDICOMFilesSentWarning() > 0)
    {
        // Alguna imatge s'ha guardat amb l'Status de warning (Normalment significa que el PACS ha modificat les dades del fitxer DICOM enviat)
        WARN_LOG(QString("En l'enviament de %1 de %2 fitxers s'ha rebut un warning")
                    .arg(QString().setNum(getNumberOfDICOMFilesSentWarning()), QString().setNum(m_numberOfDICOMFilesToSend)));
        return PACSRequestStatus::SendWarningForSomeImages;
    }

    INFO_LOG("Totes els fitxers s'han enviat al PACS correctament");

    return PACSRequestStatus::SendOk;
}

int SendDICOMFilesToPACS::getNumberOfDICOMFilesSentSuccesfully()
{
    return m_numberOfDICOMFilesSentSuccessfully;
}

int SendDICOMFilesToPACS::getNumberOfDICOMFilesSentFailed()
{
    return m_numberOfDICOMFilesToSend - m_numberOfDICOMFilesSentSuccessfully - m_numberOfDICOMFilesSentWithWarning;
}

int SendDICOMFilesToPACS::getNumberOfDICOMFilesSentWarning()
{
    return m_numberOfDICOMFilesSentWithWarning;
}

}
