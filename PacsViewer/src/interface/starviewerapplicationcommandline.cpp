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

#include "starviewerapplicationcommandline.h"

#include "applicationcommandlineoptions.h"
#include "logging.h"
#include "starviewerapplication.h"

#ifndef STARVIEWER_LITE
#include "commandlineoption.h"
#endif

namespace udg {

const QString StarviewerApplicationCommandLine::accessionNumberOption("accessionnumber");

ApplicationCommandLineOptions StarviewerApplicationCommandLine::getStarviewerApplicationCommandLineOptions()
{
    ApplicationCommandLineOptions starviewerCommandLineOptions(ApplicationNameString);

    // Configurem les opcions que Starviewer accepta des de línia de comandes
    #ifndef STARVIEWER_LITE
    // Opció no disponible Starviewer Lite
    starviewerCommandLineOptions.addOption(CommandLineOption(accessionNumberOption, true, QObject::tr("Retrieve the study with the given accession number from the query default PACS.")));
    #endif

    return starviewerCommandLineOptions;
}

bool StarviewerApplicationCommandLine::parse(QStringList argumentsList, QString &errorInvalidCommanLineArguments)
{
    ApplicationCommandLineOptions commandLineOptions = getStarviewerApplicationCommandLineOptions();
    bool ok = commandLineOptions.parseArgumentList(argumentsList);

    if (ok)
    {
        errorInvalidCommanLineArguments = "";
    }
    else
    {
        errorInvalidCommanLineArguments = commandLineOptions.getParserErrorMessage();
    }

    return ok;
}

bool StarviewerApplicationCommandLine::parseAndRun(const QString &argumentsListAsQString, QString &errorInvalidCommanLineArguments)
{
    return parseAndRun(argumentsListAsQString.split(";"), errorInvalidCommanLineArguments);
}

bool StarviewerApplicationCommandLine::parseAndRun(const QString &argumentsListAsQString)
{
    QString errorInvalidCommanLineArguments;
    bool ok;

    ok = parseAndRun(argumentsListAsQString, errorInvalidCommanLineArguments);

    if (!ok)
    {
        ERROR_LOG("Arguments de linia de comandes invalids: " + errorInvalidCommanLineArguments);
    }

    return ok;
}

bool StarviewerApplicationCommandLine::parseAndRun(QStringList arguments, QString &errorInvalidCommanLineArguments)
{
    ApplicationCommandLineOptions commandLineOptions = getStarviewerApplicationCommandLineOptions();

    if (commandLineOptions.parseArgumentList(arguments))
    {
        if (commandLineOptions.getNumberOfParsedOptions() == 0)
        {
            // Vol dir que han executat una nova instància del Starviewer que ha detectat que hi havia una altra instància executant-se
            // i ens ha enviat un missatge en blanc perquè obrim un nova finestra d'Starviewer
            QPair<StarviewerCommandLineOption, QString> commandLineOptionValue(openBlankWindow, "");
            AddOptionToCommandLineOptionListToProcess(commandLineOptionValue);
        }
        else
        {
            if (commandLineOptions.isSet(accessionNumberOption))
            {
                QPair<StarviewerCommandLineOption, QString>
                    commandLineOptionValue(retrieveStudyFromAccessioNumber, commandLineOptions.getOptionArgument(accessionNumberOption));

                AddOptionToCommandLineOptionListToProcess(commandLineOptionValue);
            }
        }

        emit newOptionsToRun();
        errorInvalidCommanLineArguments = "";

        return true;
    }
    else
    {
        errorInvalidCommanLineArguments = commandLineOptions.getParserErrorMessage();
        return false;
    }
}

bool StarviewerApplicationCommandLine::takeOptionToRun(QPair<StarviewerApplicationCommandLine::StarviewerCommandLineOption, QString> &optionValue)
{
    bool optionValueTaken = false;

    m_mutexCommandLineOptionListToProcess.lock();
    if (!m_commandLineOptionListToProcess.isEmpty())
    {
        optionValue = m_commandLineOptionListToProcess.takeFirst();

        optionValueTaken = true;
    }
    m_mutexCommandLineOptionListToProcess.unlock();

    return optionValueTaken;
}

void StarviewerApplicationCommandLine::AddOptionToCommandLineOptionListToProcess(QPair<StarviewerApplicationCommandLine::StarviewerCommandLineOption,
                                                                                 QString> optionValue)
{
    m_mutexCommandLineOptionListToProcess.lock();
    m_commandLineOptionListToProcess.append(optionValue);
    m_mutexCommandLineOptionListToProcess.unlock();
}
}
