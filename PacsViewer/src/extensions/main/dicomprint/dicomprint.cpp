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

#include "dicomprint.h"

#include <QDir>
#include <QStandardPaths>
#include <QStringList>

#include "dicomprinter.h"
#include "dicomprintjob.h"
#include "createdicomprintspool.h"
#include "printdicomspool.h"
#include "pacsdevice.h"
#include "echotopacs.h"
#include "logging.h"
#include "directoryutilities.h"

namespace udg {

int DicomPrint::print(DicomPrinter printer, DicomPrintJob printJob)
{
    PrintDicomSpool printDicomSpool;

    QStringList dcmtkStoredPrintPathFileList;
    int indexNumberOfCopies = 0, numberOfCopies, numberOfFilmSessionPrinted = 0;
    DirectoryUtilities deleteDirectory;

    m_lastError = Ok;

    INFO_LOG("Han demanat imprimir imatges DICOM a la impresssora " + printer.getAETitle() + ", IP: " + printer.getHostname() + ", port: " +
             QString().setNum(printer.getPort()));

    dcmtkStoredPrintPathFileList = createDicomPrintSpool(printer, printJob);

    if (getLastError() == Ok)
    {
        // Workaround per poder imprimir més d'una còpia d'un printjob. Degut a que ara no podem imprimir més d'una pàgina per connexió amb la impressora,
        // per la limitació de les classes utilitzades de dcmtk que només ens deixen associar un filmBox per filmeSsion, provoca que en el cas que d'un
        // printjob en volem fer més d'una còpia, per exemple amb un tenim un DicomPrintJob amb dos pàgines del qual en volem 3 còpies, primer s'imprimirà
        // 3 vegades la primera pàgina i després sortirà 3 vegades la segona pàgina, per evitar que sortin ordenades així fem aquest workaround, en el qual
        // es repeteix el procés d'enviar cada printjob tantes còpies com ens n'hagin sol·licitat, d'aquesta manera les pàgines sortiran correctament
        // ordenades.

        numberOfCopies = printJob.getNumberOfCopies();
        // Indiquem que només en volem una còpia
        printJob.setNumberOfCopies(1);

        while (indexNumberOfCopies < numberOfCopies)
        {
            INFO_LOG("Envio a imprimir la copia " + QString().setNum(indexNumberOfCopies + 1) + "/" + QString().setNum(numberOfCopies));
            // Enviem a imprimir cada pàgina
            foreach (const QString &dcmtkStoredPrintPathFile, dcmtkStoredPrintPathFileList)
            {
                INFO_LOG("Envio FilmSession a imprimir");
                printDicomSpool.printBasicGrayscale(printer, printJob, dcmtkStoredPrintPathFile, getSpoolDirectory());
                if (printDicomSpool.getLastError() != PrintDicomSpool::Ok)
                {
                    // Si hi ha error parem
                    break;
                }

                numberOfFilmSessionPrinted++;
            }

            if (printDicomSpool.getLastError() != PrintDicomSpool::Ok)
            {
                // Si hi ha error parem
                break;
            }

            indexNumberOfCopies++;
        }

        if (printDicomSpool.getLastError() == PrintDicomSpool::Ok)
        {
            INFO_LOG("S'ha imprés correctament.");
        }

        m_lastError = printDicomSpoolErrorToDicomPrintError(printDicomSpool.getLastError());
    }

    INFO_LOG("Esborro directori spool");
    deleteDirectory.deleteDirectory(getSpoolDirectory(), true);

    return numberOfFilmSessionPrinted;
}

bool DicomPrint::echoPrinter(DicomPrinter printer)
{
    PacsDevice pacs;
    bool resultTest = false;
    EchoToPACS echoToPACS;

    // HACK el codi de fer echoSCU espera que li passem un PACS, per això transformem l'objecte printer a PACS per poder fer l'echo i utilitzem
    // les classes de PACS
    pacs.setAETitle(printer.getAETitle());
    pacs.setQueryRetrieveServicePort(printer.getPort());
    pacs.setAddress(printer.getHostname());

    INFO_LOG("Es fa echoSCU a la impressora amb AETitle " + printer.getAETitle());
    resultTest = echoToPACS.echo(pacs);

    switch (echoToPACS.getLastError())
    {
        case EchoToPACS::EchoOk:
            m_lastError = DicomPrint::Ok;
            break;
        case EchoToPACS::EchoCanNotConnectToPACS:
            ERROR_LOG("No s'ha pogunt connectar amb la impressora");
            m_lastError = DicomPrint::CanNotConnectToDicomPrinter;
            break;
        default:
            ERROR_LOG("L'echo ha fallat, la impressora no ha respos com s'esperava");
            m_lastError = DicomPrint::NotRespondedAsExpected;
    }

    return resultTest;
}

QStringList DicomPrint::createDicomPrintSpool(DicomPrinter printer, DicomPrintJob printJob)
{
    CreateDicomPrintSpool dicomPrintSpool;
    QString storedDcmtkFilePath;
    QStringList dcmtkStoredPrintPathFileList;

    // Per cada pàgina que tenim generem el fitxer storedPrint de dcmtk, cada fitxer és un FilmBox (una placa)
    foreach (DicomPrintPage dicomPrintPage, printJob.getDicomPrintPages())
    {
        QString storedPrintPathFile;
        INFO_LOG("Creo les " + QString().setNum(dicomPrintPage.getImagesToPrint().count()) + " imatges de la pagina " +
                 QString().setNum(dicomPrintPage.getPageNumber()));

        storedPrintPathFile = dicomPrintSpool.createPrintSpool(printer, dicomPrintPage, getSpoolDirectory());

        if (dicomPrintSpool.getLastError() == CreateDicomPrintSpool::Ok)
        {
            dcmtkStoredPrintPathFileList.append(storedPrintPathFile);
        }
        else
        {
            break;
        }
    }

    if (dicomPrintSpool.getLastError() != CreateDicomPrintSpool::Ok)
    {
        // Si hi ha error no enviem a imprimir cap imatge, netegem la llista de fitxer StoredPrint
        dcmtkStoredPrintPathFileList.clear();
    }

    m_lastError = createDicomPrintSpoolErrorToDicomPrintError(dicomPrintSpool.getLastError());

    return dcmtkStoredPrintPathFileList;
}

DicomPrint::DicomPrintError DicomPrint::getLastError()
{
    return m_lastError;
}

DicomPrint::DicomPrintError DicomPrint::createDicomPrintSpoolErrorToDicomPrintError(
                                CreateDicomPrintSpool::CreateDicomPrintSpoolError createDicomPrintSpoolError)
{
    DicomPrint::DicomPrintError error;

    switch (createDicomPrintSpoolError)
    {
        case CreateDicomPrintSpool::ErrorCreatingImageSpool:
            error = DicomPrint::ErrorCreatingPrintSpool;
            break;
        case CreateDicomPrintSpool::ErrorLoadingImageToPrint:
            error = DicomPrint::ErrorLoadingImagesToPrint;
            break;
        case CreateDicomPrintSpool::Ok:
            error = DicomPrint::Ok;
            break;
        default:
            error = DicomPrint::UnknowError;
            break;
    }

    return error;
}

DicomPrint::DicomPrintError DicomPrint::printDicomSpoolErrorToDicomPrintError(PrintDicomSpool::PrintDicomSpoolError printDicomSpoolError)
{
    DicomPrint::DicomPrintError error;

    switch (printDicomSpoolError)
    {
        case PrintDicomSpool::CanNotConnectToDICOMPrinter:
            error = DicomPrint::CanNotConnectToDicomPrinter;
            break;
        case PrintDicomSpool::ErrorCreatingFilmbox:
            error = DicomPrint::ErrorSendingDicomPrintJob;
            break;
        case PrintDicomSpool::ErrorCreatingFilmSession:
            error = DicomPrint::ErrorSendingDicomPrintJob;
            break;
        case PrintDicomSpool::ErrorCreatingImageBox:
            error = DicomPrint::ErrorSendingDicomPrintJob;
            break;
        case PrintDicomSpool::ErrorLoadingImageToPrint:
            error = DicomPrint::ErrorLoadingImagesToPrint;
            break;
        case PrintDicomSpool::Ok:
            error = DicomPrint::Ok;
            break;
        default:
            error = DicomPrint::UnknowError;
            break;
    }

    return error;
}

QString DicomPrint::getSpoolDirectory()
{
    // Creem Spool al directori tempora del S.O.
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QDir::separator() + "DICOMSpool";
}

}
