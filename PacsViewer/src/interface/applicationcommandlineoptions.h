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

#ifndef UDGAPPLICATIONCOMMANDLINEOPTIONS_H
#define UDGAPPLICATIONCOMMANDLINEOPTIONS_H

#include <QHash>
#include <QMap>
#include <QStringList>

#include "commandlineoption.h"

class QString;

namespace udg
{

/**
Class that parses the arguments we enter by command line when running the application.
Line command arguments consist of two parts,
the first part is called the option that indicates which command
and the second part is called argument which
indicates with what value the command should be executed, for example by -accessionnumber 1234
accessionnumber is the option and 1234 the argument.

We can specify options without having to contain an argument
-version

We can specify options with an argument, if it contains more
than one argument it will be ignored and the first one will be taken
-accessionnumber 1234

This class is case sensitive
 */
class ApplicationCommandLineOptions
{

public:
    /// Default constructor in which to specify the name of the application
    ApplicationCommandLineOptions(const QString &applicationName);

    /// Returns the name of the application
    QString getApplicationName() const;

    /// Allows you to add an argument that we will accept
    /// as valid for the line command. Returns false if the argument already exists
    bool addOption(const CommandLineOption &option);

    /// Specifies the list of arguments to parse,
    /// if there are any more options that have not been defined or have some options
    /// more than one value returns false indicating that the argument was not parsed correctly
    bool parseArgumentList(QStringList argumentList);

    /// Returns the list of arguments
    QStringList getArgumentList();

    /// Returns true if we have specified that option in the argument line
    bool isSet(QString optionName);

    /// Returns the value they specified for that option
    /// If optionName has not been set as a valid option it returns NULL
    /// If you have specified the option but have not given any argument it returns empty
    QString getOptionArgument(QString optionName);

    ///Returns the error of parsing a list of arguments
    QString getParserErrorMessage();

    ///Returns the number of options that were found correct when parsing
    int getNumberOfParsedOptions();

    /// Returns a text with the synopsis of using these command options
    QString getSynopsis() const;

    /// Returns the list of possible orders that are registered
    QList<CommandLineOption> getCommandLineOptionsList() const;

private:
    /// Name of the application for which these options apply
    QString m_applicationName;

    // It contains the possible options that can enter us from the command line,
    // and contains associated the description of what that option does
    QMap<QString, CommandLineOption> m_commandLineOptions;
    // Contains the parsed options entered by line commands with their value
    QHash<QString, QString> m_parsedOptions;
    //Walkthrough arguments
    QStringList m_argumentList;
    // Contains an error text if when parsing the command line we find invalid options
    QString m_parserErrorMessage;

    /// Parse the parameters according to the options that have been specified
    bool parse();

    ///Indicates whether the string is an option, an option if it is a -versions formatted command
    bool isAnOption(QString command);

    /// Indicates whether the string is an argument of an option
    bool isAnArgument(QString argument);
};

}

#endif
