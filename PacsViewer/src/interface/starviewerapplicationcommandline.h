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

#ifndef UDGSTARVIEWERAPPLICATIONCOMMANDLINE_H
#define UDGSTARVIEWERAPPLICATIONCOMMANDLINE_H

#include <QObject>
#include <QPair>
#include <QMutex>

class QStringList;

namespace udg {

class ApplicationCommandLineOptions;

/**
    Aquesta classe configura un objecte ApplicationCommandLineOptions amb les opcions per línia de comandes vàlides
    per starviewer, d'aquesta manera tenim centralitzat en un sol lloc la configuració dels paràmetres acceptats per Starviewer
    Aquesta classe pot ser utilitza tant per parsejar paràmetres entrats per línia de comandes a la mateixa instància o paràmetres
    rebuts des d'altres instàncies d'Starviewer a través de QtSingleApplication

    Ara Starviewer mateix accepta aquestes opcions:

    -accessionnumber valorAccessionNumber : Cerca l'estudi amb el valor d'accession number especificat als PACS marcats per cercar per
                                            defecte i si el troba el descarrega.
    (blanc)                                : Si s'executa una instància d'starviewer sense cap paràmetre s'obre starviewer amb una finestra en blanc.
  */
class StarviewerApplicationCommandLine : public QObject {
Q_OBJECT
public:
    // Aquí definim les possibles opcions que ens poden demanar des de línia de comandes
    enum StarviewerCommandLineOption { openBlankWindow, retrieveStudyFromAccessioNumber };

    /// Retorna el ApplicationCommandLineOptions amb els arguments vàlids que accepta Starviewer per línia de comandes
    static ApplicationCommandLineOptions getStarviewerApplicationCommandLineOptions();

    /// Parseja una llista d'arguments, retorna boolea indicant si els arugments de la llista sòn valids d'acord amb els paràmetres
    /// que accepta Starviewer per línia de comandes, si algun dels arguments no són vàlids el QString errorInvalidCommanLineArguments
    /// retorna una descripció de quins són els arguments invàlids
    bool parse(QStringList arguments, QString &errorInvalidCommanLineArguments);

    /// Parseja una llista d'arguments, retorna boolea indicant si els arugments de la llista sòn valids d'acord amb els paràmetres
    /// que accepta Starviewer per línia de comandes, si algun dels arguments no són vàlids el QString errorInvalidCommanLineArguments
    /// retorna una descripció de quins són els arguments invàlids, si els arguments són vàlids fa un emit del signal newOptionsToRun, per
    /// indicar que hi ha peticions via comanda de línia per ser ateges
    bool parseAndRun(QStringList arguments, QString &errorInvalidCommanLineArguments);

    /// Parseja una llista d'arguments passats en un QString separats per ';', retorna boolea indicant si els arugments de la llista
    /// sòn valids d'acord amb els paràmetres que accepta Starviewer per línia de comandes, si algun dels arguments no són vàlids el
    /// QString errorInvalidCommanLineArguments retorna una descripció de quins són els arguments invàlids, si els arguments són vàlids
    /// fa un emit del signal newOptionsToRun, per indicar que hi ha peticions via comanda de línia per ser ateges
    bool parseAndRun(const QString &argumentsListAsQString, QString &errorInvalidCommanLineArguments);

    /// Retorna un dels arguments que s'han demanat via línia de comandes amb el seu valor, si no hi ha nous arguments per processar
    /// retorna false el mètode
    bool takeOptionToRun(QPair<StarviewerApplicationCommandLine::StarviewerCommandLineOption, QString> &optionValue);

signals:
    /// Signal que s'emet per indicar que hem rebut nous arguments des de línia de comandes que s'han d'executar
    void newOptionsToRun();

public slots:
    /// Slot que passant la llista d'arguments de la línia de comandes com a QString separat per ";" els parseja, i llança signal
    /// per indicar que hi ha arguments pendents d'executar. Si alguna de les opcions no és vàlida no executa res i retorna fals
    bool parseAndRun(const QString &argumentsListAsQString);

private:
    static const QString accessionNumberOption;

    // Guardem l'opció (argument de comanda de línies) amb el seu valor
    QList<QPair<StarviewerCommandLineOption, QString> > m_commandLineOptionListToProcess;

    // QMutex per assegurar que dos processos accedeixin a la m_commandLineOptionListToProcess amb operacions d'escriptura
    QMutex m_mutexCommandLineOptionListToProcess;

    /// Afegeix a la llista m_commandLineOptionListToProcess, una opció amb el seu valor, com a pendent d'executar
    void AddOptionToCommandLineOptionListToProcess(QPair<StarviewerApplicationCommandLine::StarviewerCommandLineOption, QString> optionValue);
};

}

#endif
