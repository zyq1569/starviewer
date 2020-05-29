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

#ifndef UDGDICOMPRINT_H
#define UDGDICOMPRINT_H

#include "createdicomprintspool.h"
#include "printdicomspool.h"

namespace udg {

class DicomPrinter;
class DicomPrintJob;

/**
    Imprimeix en una impressora dicom un DicomPrintJob
  */
class DicomPrint {
public:
    enum DicomPrintError { CanNotConnectToDicomPrinter, ErrorSendingDicomPrintJob, ErrorLoadingImagesToPrint, ErrorCreatingPrintSpool, UnknowError, Ok,
                           NotRespondedAsExpected };

    /// Retorna el número de pàgines impreses, com ara tenim el hack de que només poden enviar cada vegada filmbox a imprimir podria ser que s'hagués
    /// enviat a imprimir correctament alguna de les pàgines, per tant per saber si és aquest el cas indiquem el número de pàgines que s'han imprés
    int print(DicomPrinter printer, DicomPrintJob printJob);

    /// Comprova si amb la configuració passada per paràmetre una impressora respón
    bool echoPrinter(DicomPrinter printer);

    DicomPrint::DicomPrintError getLastError();

private:
    DicomPrintError m_lastError;

    QStringList createDicomPrintSpool(DicomPrinter, DicomPrintJob printJob);

    /// Transforma l'error de CreateDicomPrintSpool a un error de la classe DicomPrint
    DicomPrint::DicomPrintError createDicomPrintSpoolErrorToDicomPrintError(CreateDicomPrintSpool::CreateDicomPrintSpoolError error);

    /// Transforma l'error de CreateDicomPrintSpool a un error de la classe DicomPrint
    DicomPrint::DicomPrintError printDicomSpoolErrorToDicomPrintError(PrintDicomSpool::PrintDicomSpoolError error);

    /// Retorna el directori on genera l'Spool per imprimir les imatges DICOM
    QString getSpoolDirectory();

};
};
#endif
