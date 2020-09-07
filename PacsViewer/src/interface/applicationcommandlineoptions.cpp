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

#include "applicationcommandlineoptions.h"

#include <QString>
#include <QObject>

namespace udg {

ApplicationCommandLineOptions::ApplicationCommandLineOptions(const QString &applicationName)
{
    m_applicationName = applicationName;
}

QString ApplicationCommandLineOptions::getApplicationName() const
{
    return m_applicationName;
}

bool ApplicationCommandLineOptions::addOption(const CommandLineOption &option)
{
    if (!m_commandLineOptions.contains(option.getName()))
    {
        m_commandLineOptions.insert(option.getName(), option);
        return true;
    }
    else
    {
        return false;
    }
}

bool ApplicationCommandLineOptions::parseArgumentList(QStringList argumentList)
{
    m_argumentList = argumentList;
    return parse();
}

QStringList ApplicationCommandLineOptions::getArgumentList()
{
    return m_argumentList;
}

// Line commands consist of an option that is preceded by a '-' and can then contain an argument,
// indicating the value of the option
bool ApplicationCommandLineOptions::parse()
{
    QStringList argumentList = m_argumentList;
    QString parameter;
    bool lastParameterWasAnOption = false, nextParameterHasToBeAnArgumentOption = false;
    CommandLineOption lastOption;

    m_parserErrorMessage = "";
    //We choose the first string which is the name of the application
    argumentList.removeFirst();

    //As long as there are arguments to process or no error has occurred we parse the arguments
    while (!argumentList.isEmpty() && m_parserErrorMessage.isEmpty())
    {
        parameter = argumentList.takeFirst();

        if (isAnOption(parameter))
        {
            if (!nextParameterHasToBeAnArgumentOption)
            {
                // We choose the "-" parameter
                parameter = parameter.right(parameter.length() -1);
                // We check if it is a valid option
                if (m_commandLineOptions.contains(parameter))
                {
                    /// If it is an option that we have specified as valid we insert it as a parsed,
                    /// for now as an argument of the option we put ""
                    m_parsedOptions.insert(parameter, "");

                    lastParameterWasAnOption = true;
                    lastOption = m_commandLineOptions.value(parameter);
                    nextParameterHasToBeAnArgumentOption = lastOption.requiresArgument();
                }
                else
                {
                    m_parserErrorMessage += QObject::tr("Unknown option %1").arg(CommandLineOption::OptionSelectorPrefix + parameter) + "\n";
                }
            }
            else
            {
                // If the last parsed parameter was an option that had to be passed an argument ex "-accessionnumber 12345"
                // and no argument was specified ex: "-accessionnumber -studyUID" we save the error and stop.
                m_parserErrorMessage += QObject::tr("%1 option requires an argument").arg(lastOption.getName()) + "\n";
            }
        }
        else
        {
            /// It's an argument
            if (lastParameterWasAnOption)
            {
                if (nextParameterHasToBeAnArgumentOption)
                {
                    /// If we have an argument and the last parameter was an option, it means this parameter is an argument
                    m_parsedOptions[lastOption.getName()] = parameter;
                }
                else
                {
                    m_parserErrorMessage += QObject::tr("%1 option requires an argument").arg(lastOption.getName()) + "\n";
                }
            }
            else
            {
                m_parserErrorMessage += QObject::tr("Unexpected value %1").arg(parameter) + "\n";
            }

            lastParameterWasAnOption = false;
            nextParameterHasToBeAnArgumentOption = false;
        }
    }

    if (nextParameterHasToBeAnArgumentOption)
    {
        m_parserErrorMessage += QObject::tr("%1 option requires an argument").arg(lastOption.getName()) + "\n";
    }

    return m_parserErrorMessage.isEmpty();
}

bool ApplicationCommandLineOptions::isSet(QString optionName)
{
    return m_parsedOptions.contains(optionName);
}

QString ApplicationCommandLineOptions::getOptionArgument(QString optionName)
{
    if (isSet(optionName))
    {
        return m_parsedOptions[optionName];
    }
    else
    {
        return NULL;
    }
}

int ApplicationCommandLineOptions::getNumberOfParsedOptions()
{
    return m_parsedOptions.count();
}

QString ApplicationCommandLineOptions::getParserErrorMessage()
{
    return m_parserErrorMessage;
}

QString ApplicationCommandLineOptions::getSynopsis() const
{
    QString synopsis;

    synopsis = QObject::tr("Synopsis:");
    synopsis += "\n";
    synopsis += m_applicationName;

    QString applicationOptions;
    QString detailedOptions;
    foreach (const CommandLineOption &option, m_commandLineOptions.values())
    {
        applicationOptions += " [" + option.toString(false) + "]";
        detailedOptions += option.toString(true) + "\n\n";
    }

    synopsis += applicationOptions + "\n";
    synopsis += "\n" + QObject::tr("Options:");
    synopsis += "\n" + detailedOptions;

    return synopsis;
}

QList<CommandLineOption> ApplicationCommandLineOptions::getCommandLineOptionsList() const
{
    return m_commandLineOptions.values();
}

bool ApplicationCommandLineOptions::isAnOption(QString optionName)
{
    // Options always start "-"
    return optionName.startsWith(CommandLineOption::OptionSelectorPrefix);
}

bool ApplicationCommandLineOptions::isAnArgument(QString argument)
{
    return !argument.startsWith(CommandLineOption::OptionSelectorPrefix);
}

}
