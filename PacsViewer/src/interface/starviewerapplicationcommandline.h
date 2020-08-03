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
This class configures an ApplicationCommandLineOptions object with valid command line options
     for starviewer, in this way we have centralized in one place the configuration of the parameters accepted by Starviewer
     This class can be used to either parse parameters entered by command line in the same instance or parameters
     received from other instances of Starviewer via QtSingleApplication

Now Starviewer itself accepts these options:

     -accessionnumber valueAccessionNumber: Search the study with the accession number value specified in the PACS marked to search by
                                             defect and if it finds it download it.
     (blank): Running an instance of starviewer without any parameters opens starviewer with a blank window.
  */
class StarviewerApplicationCommandLine : public QObject {
    Q_OBJECT
public:
    //Here we define the possible options you can ask us from the command line
    enum StarviewerCommandLineOption { openBlankWindow, retrieveStudyFromAccessioNumber };

    /// Returns the ApplicationCommandLineOptions with the valid arguments that Starviewer accepts per command line
    static ApplicationCommandLineOptions getStarviewerApplicationCommandLineOptions();

    /// Parse a list of arguments, returns boolea indicating whether the arugments in the list are valid according to the parameters
    /// that accepts Starviewer by command line, if any of the arguments are invalid the QString errorInvalidCommanLineArguments
    /// returns a description of what the invalid arguments are
    bool parse(QStringList arguments, QString &errorInvalidCommanLineArguments);

    /// Parse a list of arguments, returns boolea indicating whether the arugments in the list are valid according to the parameters
    /// that accepts Starviewer by command line, if any of the arguments are invalid the QString errorInvalidCommanLineArguments
    /// returns a description of what the invalid arguments are, if the arguments are valid it emits the signal newOptionsToRun, for
    /// indicate that there are requests via line command to be ateges
    bool parseAndRun(QStringList arguments, QString &errorInvalidCommanLineArguments);

    /// Parse a list of past arguments in a QString separated by ';', return boolea indicating whether the arugments in the list
    /// are valid according to the parameters accepted by Starviewer per command line, if any of the arguments are invalid on
    /// QString errorInvalidCommanLineArguments returns a description of what the invalid arguments are, if the arguments are valid
    /// emits the signal newOptionsToRun, to indicate that there are requests via line command to be ateges
    bool parseAndRun(const QString &argumentsListAsQString, QString &errorInvalidCommanLineArguments);

    /// Returns one of the arguments requested via command line with its value, if there are no new arguments to process
    /// returns false the method
    bool takeOptionToRun(QPair<StarviewerApplicationCommandLine::StarviewerCommandLineOption, QString> &optionValue);

signals:
    /// Signal that is issued to indicate that we have received new arguments from the command line to be executed
    void newOptionsToRun();

public slots:
    /// Slot that passing the command line argument list as QString separated by ";" he parses them, and sends signal
    /// to indicate that there are arguments to execute. If any of the options is invalid it does not execute anything and returns false
    bool parseAndRun(const QString &argumentsListAsQString);

private:
    static const QString accessionNumberOption;

    // We save the option (line command argument) with its value
    QList<QPair<StarviewerCommandLineOption, QString> > m_commandLineOptionListToProcess;

    //QMutex to ensure that two processes access the m_commandLineOptionListToProcess with write operations
    QMutex m_mutexCommandLineOptionListToProcess;

    ///Adds to the list m_commandLineOptionListToProcess, an option with its value, as pending
    void AddOptionToCommandLineOptionListToProcess(QPair<StarviewerApplicationCommandLine::StarviewerCommandLineOption, QString> optionValue);
};

}

#endif
