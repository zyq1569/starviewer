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

#ifndef UDGDICOMPRINTERMANAGER_H
#define UDGDICOMPRINTERMANAGER_H

#include <QList>
#include "settings.h"

namespace udg {
class DicomPrinter;

/**
    Implementació de la interfície PrinterManager per impressores DICOM en DCMTK.
    Aquest classe permet Afegir/Modificar/Esborrar les impressores DICOM que tenim guardades al sistema mitjançant Settings.
  */
class DicomPrinterManager {
public:
    /// Afegeix una impressora Dicom (\p printer) al sistema.
    /// Retorna True si s'ha pogut afegir la impressora i False si la impressora ja existeix al sistema.
    /// En cas que s'hagi pogut afegir, s'assigna l'id a la impressora passada per paràmetre.
    bool addPrinter(DicomPrinter &printer);

    /// Modifica la impressora amb identificador printerID amb els nous paràmetres guardats a newDicomPrinter.
    /// Retorna True si s'ha pogut modificar correctament i False si la impressora a modificar no existeix o no és vàlida.
    bool updatePrinter(int printerID, DicomPrinter &newDicomPrinter);

    /// Esborra la impressora amb identificador printerID.
    void deletePrinter(int printerID);

    /// Retorna un objecte DicomPrinter que conté les dades de la impressora amb identificador printerID.
    DicomPrinter getPrinterByID(int printerID);

    /// Retorna una llista de DicomPrinters que hi ha emmagatzemades al sistema.
    QList<DicomPrinter> getDicomPrinterList();

    /// Retorna un objecte DicomPrinter amb el rang de valors que pot assignar-se a cada paràmetre configurable de la impressora. Aquests rang de valors són
    /// els que per defecte estableix el DICOM, però la impressora no té perquè acceptar-los tots.
    /// TO-DO: S'hauria de substituir aquest mètode per un que es connectes a la impressora a través del seu AETitle i IP demanant-li per cada paràmetre
    /// configurable
    ///        quins valors accepta
    DicomPrinter getDefaultAvailableParametersValuesDICOMPrinters();

private:
    /// Retorna un Settings::SettingsListItemType omplert amb les dades de la impressora dicomPrinter.
    Settings::SettingsListItemType dicomPrinterToSettingsListItem(DicomPrinter &dicomPrinter);

    /// Retorna un DicomPrinter omplert amb les dades que conté el Settings::SettingsListItemType.
    DicomPrinter settingsListItemToDicomPrinter(Settings::SettingsListItemType &item);

    /// Retorna l'index on està emmagatzemada la impressora printer dins del sistema (Settings).
    int indexOfPrinterInSettings(DicomPrinter &printer);

    /// Retorna una llista de valors que pot agafar el MediumType d'una impressora Dicom.
    QStringList getAvailableMediumTypeValues();

    /// Retorna una llista de possibles valors que pot agafar el FilmSize d'una impressora Dicom.
    QStringList getAvailableFilmSizeValues();

    /// Retorna una llista de possibles valors que pot agafar el FilmLayout d'una impressora Dicom.
    QStringList getAvailableFilmLayoutValues();

    /// Retorna una llista de possibles valors que pot agafar el FilmOrentation d'una impressora Dicom.
    QStringList getAvailableFilmOrientationValues();

    /// Retorna una llista de possibles valors que pot agafar el MagnificationType d'una impressora Dicom.
    QStringList getAvailableMagnificationTypeValues();

    /// Retorna el valor màxim de densitat que pot agafar una impressora Dicom.
    ushort getAvailableMaxDensityValues();

    /// Retorna el valor mínim de densitat que pot agafar una impressora Dicom.
    ushort getAvailableMinDensityValues();

    /// Retorna la possibilitat d'utilitzar Trim en una impressora Dicom.
    bool getAvailableTrim();

    /// Retorna una llista de possibles valors que pot agafar el MagnificationType d'una impressora Dicom.
    QStringList getAvailableBorderDensityValues();

    /// Retorna una llista de possibles valors que pot agafar el EmptyImageDensity d'una impressora Dicom.
    QStringList getAvailableEmptyImageDensityValues();

    /// Retorna una llista de possibles valors que pot agafar el FilmDestination d'uma impressora Dicom.
    QStringList getAvailableFilmDestinationValues();

    /// Retorna una llista de possibles valors que pot agafar el SmoothingType d'uma impressora Dicom.
    QStringList getAvailableSmoothingTypeValues();

    /// Retorna una llista de possibles valors que pot agafar el PrintPriority d'uma impressora Dicom.
    QStringList getAvailablePrintPriorityValues();

    /// Retorna una llista de possibles valors que pot agafar el Polarity d'uma impressora Dicom.
    QStringList getAvailablePolarityValues();

    /// Ens posa totes les impressores que tenim configurades com a impressores no per defecte, és a dir cap d'elles és la
    /// impressora seleccionada per imprimir per defecte
    void setAllPrintersAsNoDefaultPrinter();

private:
    /// Conté el nom de la secció del Settings on es guarden les dades de la impressora.
    static const QString DicomPrinterListSectionName;
};
};
#endif
