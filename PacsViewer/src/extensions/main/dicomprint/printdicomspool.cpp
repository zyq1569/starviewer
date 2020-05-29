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

#include "printdicomspool.h"

// Classe DicomImage
#include <dcmimage.h>
// Classe DVPSPrintEventHandler
#include <dvpspr.h>
// Classe DVPSStoredPrint
#include <dvpssp.h>
// Classe DVPSHelper
#include <dvpshlp.h>

#include <QDir>

#include "dicomprintpage.h"
#include "logging.h"
#include "pacsdevice.h"
#include "inputoutputsettings.h"

namespace udg {

void PrintDicomSpool::printBasicGrayscale(DicomPrinter dicomPrinter, DicomPrintJob dicomPrintJob, const QString &storedPrintDcmtkFilePath,
                                          const QString &spoolDirectoryPath)
{
    DVPSPrintMessageHandler printerConnection;
    OFCondition result;
    bool printerSupportsPresentationLUTSOPClass = false, printerSupportsAnnotationSOPClass = true, transferSyntaxImplicit = false;

    m_dicomPrinter = dicomPrinter;
    m_dicomPrintJob = dicomPrintJob;
    m_lastError = PrintDicomSpool::Ok;

    // Connectem amb la impressora
    //  1r Paràmetre és de tipus objecte tlayer només s'utilitza si la comunicació es fa amb ssl
    //  2n AETitle del Starviewer 3r AETitle de la impressora Dicom
    //  4t Hostname de la impressora dicom 5è Port pel qual ens hem de comunicar amb la impressora
    //  6è Tamany màxim de la pdu per la comunicació, li donem el valor per defecte del els dcmtk
    //  7è Indiquem si la impressora suporta Presentation LUT SOP Class, com que de moment nosaltres no ho soportem li indiquem fals
    //  8è Indiquem si la impressora suporta Basic Annotation Box SOP Class, com que de moment nosaltres no ho soportem li indiquem fals
    //  9è Indica si la comunicació s'ha de fer amb transfer syntax Implicit, això és degut a que dispositius vells que suportaven Dicom
    //     tot i que indicar en el seu conformance que suportaven transfer syntax explicit, alhora de la veritat tenien problemes, per això
    //     existeix un paràmetre per indicar només de comunicar-se amb Implicit, com que de moment no ens hem de comunicar amb dispositius vells
    //     i tots els moderns suporten Explicit indiquem false
    result = printerConnection.negotiateAssociation(NULL, qPrintable(Settings().getValue(InputOutputSettings::LocalAETitle).toString()),
                                                    qPrintable(m_dicomPrinter.getAETitle()), qPrintable(m_dicomPrinter.getHostname()),
                                                    m_dicomPrinter.getPort(), ASC_DEFAULTMAXPDU, printerSupportsPresentationLUTSOPClass,
                                                    printerSupportsAnnotationSOPClass, transferSyntaxImplicit);

    if (result.bad())
    {
        ERROR_LOG(QString("No s'ha pogut connectar amb la impressora amb AE Title: %1, IP: %2, port: %3, descripcio error:%4 ").arg(m_dicomPrinter.getAETitle(),
                           m_dicomPrinter.getHostname(), QString().setNum(m_dicomPrinter.getPort()), result.text()));

        m_lastError = PrintDicomSpool::CanNotConnectToDICOMPrinter;

        return;
    }

    printStoredPrintDcmtkContent(printerConnection, storedPrintDcmtkFilePath, spoolDirectoryPath);

    result = printerConnection.releaseAssociation();
    if (result.bad())
    {
        ERROR_LOG(QString("No s'ha pogut desconnectar correctament de la impressora, descripcio error: %1").arg(result.text()));
    }

}

void PrintDicomSpool::printStoredPrintDcmtkContent(DVPSPrintMessageHandler &printerConnection, const QString storedPrintDcmtkFilePath,
                                                   const QString &spoolDirectoryPath)
{
    OFCondition result;
    DVPSStoredPrint *storedPrintDcmtk = loadStoredPrintFileDcmtk(storedPrintDcmtkFilePath);
    // Aquesta variable indica si els Attibutes de PLUT que són Illumination (2010,015E) i (2010,0160) s'han d'afegir al BasicFilmSession o al BasicFilmBox
    // Nosaltres indiquem que els afegim al BasicFilmBox perquè si mirem la documentació DICOM, tot i que dcmtk ho suporta aquests dos tags no apareixen com
    // a atributs possibles d'un BasicFilmBox.
    // De totes maneres si mirem el codi de les dcmtk on passem aquesta variable, aquesta no té cap afecte, perquè com no soportem les PresentatioLUTSOP
    // el valor que conté la variable s'ignora i aquestes dos tags no s'envien
    bool addPLUTAttributesInBasicFilmSession = false;

    if (!storedPrintDcmtk)
    {
        m_lastError = PrintDicomSpool::ErrorLoadingImageToPrint;
        return;
    }
    // TODO: És necessari ?, simplement demana a la impressora informació de certs paràmetres, té algun sentit ? Preguntar al fòrum,
    // Enganxar codi dvpssp.cxx línia 1342 ensenyant que no fa res
    result = storedPrintDcmtk->printSCUgetPrinterInstance(printerConnection);
    if (EC_Normal != result)
    {
        ERROR_LOG(QString("No s'ha pogut obtenir els settings de la impressora, descripcio error: %1").arg(result.text()));
    }

    // Abans de fer un acció amb la impressora comprovem si la última ha anat bé amb EC_NORMAL=result, si ha fallat totes les accions restants no s'executaran
    if (EC_Normal == result)
    {
        DcmDataset basicFilmSessionDataset = getAttributesBasicFilmSession();

        result = storedPrintDcmtk->printSCUcreateBasicFilmSession(printerConnection, basicFilmSessionDataset, addPLUTAttributesInBasicFilmSession);

        if (EC_Normal != result)
        {
            // Si no s'ha pogut crear el FilmSession possiblement la impressora no ens ha acceptat algun dels paràmetres de configuració del FilmSession
            ERROR_LOG(QString("No s'ha pogut crear el Basic FilmSession. %1").arg(result.text()));
            m_lastError = PrintDicomSpool::ErrorCreatingFilmSession;
        }
        else
        {
            INFO_LOG("S'ha enviat correctament les dades del FilmSession a la impressora DICOM");
        }
    }

    if (EC_Normal == result)
    {
        result = storedPrintDcmtk->printSCUcreateBasicFilmBox(printerConnection, addPLUTAttributesInBasicFilmSession);
        if (EC_Normal != result)
        {
            ERROR_LOG(QString("No s'ha pogut crear el film box. %1").arg(result.text()));
            m_lastError = PrintDicomSpool::ErrorCreatingImageBox;
        }
        else
        {
            INFO_LOG("S'ha enviat correctament les dades del FilmBox a la impressora DICOM");
        }
    }

    if (EC_Normal == result)
    {
        // Crea i envia els AnnotationBox
        result = createAndSendFilmBoxAnnotations(printerConnection, storedPrintDcmtk);
    }

    if (EC_Normal == result)
    {
        for (size_t imageNumber = 0; imageNumber < storedPrintDcmtk->getNumberOfImages(); imageNumber++)
        {
            result = createAndSendBasicGrayscaleImageBox(printerConnection, storedPrintDcmtk, imageNumber, spoolDirectoryPath);

            if (result != EC_Normal)
            {
                break;
            }
        }
    }

    // Donem ordre d'imprimir el BasicFilmBox, no cal donar ordre d'imprimir el FilmSession amb el BasicFilmBox n'hi ha suficent
    if (EC_Normal == result)
    {
        INFO_LOG("S'ha enviat correctament totes les imatges a la impressora");
        result = storedPrintDcmtk->printSCUprintBasicFilmBox(printerConnection);
        if (EC_Normal != result)
        {
            ERROR_LOG(QString("No s'ha pogut imprimir el Filmbox. %1").arg(result.text()));
        }
        else
        {
            INFO_LOG("Enviada l'ordre d'imprimir el FilmBox");
        }
    }

    // El printSCUDelete el fem sempre encara que algun pas hagi fallat, per indicar a la impressora que ja pot alliberar els recursos guardats per
    // emmagatzemar el FilmSession,FilmBox i ImageBox. Aquest mètode només resultarà útil quan la impressió ha anat correcte o si s'ha produït un error
    // s'ha produït després de crear el FilmSession

    result = storedPrintDcmtk->printSCUdelete(printerConnection);
    if (EC_Normal != result)
    {
        ERROR_LOG(QString("No s'ha pogut fer el delete dels print objects %1.").arg(result.text()));
    }
    else
    {
        INFO_LOG("Enviada l'ordre de fer delete del les imatges enviades per imprimir");
    }

    delete storedPrintDcmtk;
}

DcmDataset PrintDicomSpool::getAttributesBasicFilmSession()
{
    DcmDataset datasetBasicFilmSession;
    DcmElement *attributeBasicFilmSession = NULL;

    attributeBasicFilmSession = new DcmCodeString(DCM_MediumType);
    attributeBasicFilmSession->putString(qPrintable(m_dicomPrintJob.getMediumType()));
    datasetBasicFilmSession.insert(attributeBasicFilmSession);

    attributeBasicFilmSession = new DcmCodeString(DCM_FilmDestination);
    attributeBasicFilmSession->putString(qPrintable(m_dicomPrintJob.getFilmDestination()));
    datasetBasicFilmSession.insert(attributeBasicFilmSession);

    // TODO:Comprovar si l'utilitzarem
    // attributeBasicFilmSession = new DcmLongString(DCM_FilmSessionLabel);
    // attributeBasicFilmSession->putString(printerFilmSessionLabel.c_str()); else result = EC_IllegalCall;
    // datasetBasicFilmSession.insert(attributeBasicFilmSession, OFTrue replaceOld);

    attributeBasicFilmSession = new DcmCodeString(DCM_PrintPriority);
    attributeBasicFilmSession->putString(qPrintable(m_dicomPrintJob.getPrintPriority()));
    datasetBasicFilmSession.insert(attributeBasicFilmSession);

    // TODO:Si no m'equivoco només serveix per quan s'esborra el treball imprés del servidor d'impressió, si no l'especifiquem
    // el servidor d'impressió l'emplena automàticament
    // attributeBasicFilmSession = new DcmShortString(DCM_OwnerID);
    // attributeBasicFilmSession->putString(printerOwnerID.c_str());
    // datasetBasicFilmSession.insert(attributeBasicFilmSession, OFTrue replaceOld);

    attributeBasicFilmSession = new DcmIntegerString(DCM_NumberOfCopies);
    attributeBasicFilmSession->putString(qPrintable(QString().setNum(m_dicomPrintJob.getNumberOfCopies())));
    datasetBasicFilmSession.insert(attributeBasicFilmSession);

    INFO_LOG("Llegits els atributs del FilmSession");

    return datasetBasicFilmSession;
}

OFCondition PrintDicomSpool::createAndSendBasicGrayscaleImageBox(DVPSPrintMessageHandler &printerConnection, DVPSStoredPrint *storedPrintDcmtk,
                                                                 size_t imageNumber, const QString &spoolDirectoryPath)
{
    OFCondition result;
    const char *studyUID = NULL, *seriesUID = NULL, *instanceUID = NULL;
    DicomImage *imageToPrint;
    QString imageToPrintPath;
    bool isImageMonochrome1;

    // Busquem el studyUID, seriesUID i instaceUID de la imatge a imprimir
    result = storedPrintDcmtk->getImageReference(imageNumber, studyUID, seriesUID, instanceUID);

    if (result != EC_Normal || !studyUID || !seriesUID || !instanceUID)
    {
        ERROR_LOG(QString("No s'ha trobat la imatge número %1 per imprimir").arg(QString().setNum(imageNumber)));
        m_lastError = PrintDicomSpool::ErrorLoadingImageToPrint;

        return result;
    }
    else
    {
        // TODO:S'hauria de fer a un altre lloc el càlcul del path de la imatge perquè també s'utilitza a CreateDicomPrintSpool
        imageToPrintPath = QDir::toNativeSeparators(spoolDirectoryPath + QDir::separator() + instanceUID + ".dcm");
        imageToPrint = new DicomImage(qPrintable(imageToPrintPath));

        if (imageToPrint && EIS_Normal == imageToPrint->getStatus())
        {
            isImageMonochrome1 = imageToPrint->getPhotometricInterpretation() == EPI_Monochrome1;

            // Enviem la imatge
            result = storedPrintDcmtk->printSCUsetBasicImageBox(printerConnection, imageNumber, *imageToPrint, isImageMonochrome1);
            if (EC_Normal != result)
            {
                ERROR_LOG(QString("No s'ha pogut enviar el basic grayscale imagebox, descripcio error: %1").arg(result.text()));
                m_lastError = PrintDicomSpool::ErrorCreatingImageBox;
            }
            else
            {
                INFO_LOG("S'ha enviat a imprmir correctament la imatge " + imageToPrintPath);
            }

        }
        else
        {
            ERROR_LOG("No s'ha pogut carregar la imatge a imprimir " + imageToPrintPath);
            m_lastError = PrintDicomSpool::ErrorLoadingImageToPrint;
        }

        // No s'ha de fer el delete del studyUID, seriesUID i instanceUID perquè són un punter a informació del storedPrint
        delete imageToPrint;

        return result;
    }
}

OFCondition PrintDicomSpool::createAndSendFilmBoxAnnotations(DVPSPrintMessageHandler &printConnection, DVPSStoredPrint *storedPrintDcmtk)
{
    OFCondition result;

    INFO_LOG(QString("S'enviaran %1 Annotation Box a la impressora").arg(storedPrintDcmtk->getNumberOfAnnotations()));

    /// Enviem totes les anotacions que tinguem de la film box
    for (size_t currentAnnotation = 0; currentAnnotation < storedPrintDcmtk->getNumberOfAnnotations(); currentAnnotation++)
    {
        if (EC_Normal == result)
        {
            if (EC_Normal != (result = storedPrintDcmtk->printSCUsetBasicAnnotationBox(printConnection, currentAnnotation)))
            {
                INFO_LOG(QString("spooler: printer communication failed, unable to transmit basic annotation box %1. Error %2 ")
                            .arg(currentAnnotation).arg(result.text()));
            }
        }
    }

    return result;
}

DVPSStoredPrint* PrintDicomSpool::loadStoredPrintFileDcmtk(const QString &pathStoredPrintDcmtkFile)
{
    // El constructor del DVPStoredPrint se li ha de passar com a paràmetres
    //  1r El tag (2010,015E) Illumination de la Basic Film Box
    //  2n El tag (2010,0160) Reflected Ambient Light de la Basic Film Box
    //  3r AETitle del Starviewer

    // Els dos primer paràmetres només s'utilitzen si la impressora suporta el Presentation Lut, ara mateix no ho soportem (no està implementat) per tant se
    // suposa que aquests valors s'ignoraran. De totes maneres se li ha donat aquests valors per defecte 2000 i 10 respectivament perquè són els que utilitza
    // dcmtk i també s'ha consultat el dicom conformance de les impressores agfa i kodak i també utiltizen aquests valors per defecte.
    DVPSStoredPrint *storedPrint = new DVPSStoredPrint(2000, 10, qPrintable(Settings().getValue(InputOutputSettings::LocalAETitle).toString()));
    DcmFileFormat *storedPrintDcmtkFile = NULL;
    DcmDataset *datasetStoredPrintDcmtkFile = NULL;

    OFCondition result = DVPSHelper::loadFileFormat(qPrintable(QDir::toNativeSeparators(pathStoredPrintDcmtkFile)), storedPrintDcmtkFile);
    if (EC_Normal != result)
    {
        ERROR_LOG("No s'ha pogut obrir el fitxer StoredPrint de dcmtk, path:" + pathStoredPrintDcmtkFile);
        return NULL;
    }

    datasetStoredPrintDcmtkFile = storedPrintDcmtkFile->getDataset();

    if (!datasetStoredPrintDcmtkFile)
    {
        ERROR_LOG("No s'ha pogut obtenir el dataset del fitxer StoredPrint de dcmtk");
        return NULL;
    }

    result = storedPrint->read(*datasetStoredPrintDcmtkFile);

    if (EC_Normal != result)
    {
        ERROR_LOG("No s'ha pogut carregar les dades del StoredPrint per imprimir");
        return NULL;
    }

    delete storedPrintDcmtkFile;

    INFO_LOG("S'ha carregat correctament el fitxer StoredPrint (DVPSStoredPrint)");

    return storedPrint;
}

PrintDicomSpool::PrintDicomSpoolError PrintDicomSpool::getLastError()
{
    return m_lastError;
}
}
