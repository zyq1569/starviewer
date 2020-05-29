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

#include "../main/applicationtranslationsloader.h"
#include "../core/starviewerapplication.h"
#include "../core/coresettings.h"
#include "qcrashreporter.h"
#include "logging.h"

// Programa principal per executar l'aplicació de crash report

void configureLogging()
{
    // Primer comprovem que existeixi el direcotori ~/.starviewer/log/ on guradarem els logs
    QDir logDir = udg::UserLogsPath;
    if (!logDir.exists())
    {
        // Creem el directori
        logDir.mkpath(udg::UserLogsPath);
    }
    // TODO donem per fet que l'arxiu es diu així i es troba a la localització que indiquem. S'hauria de fer una mica més flexible o genèric;
    // està així perquè de moment volem anar per feina i no entretenir-nos però s'ha de fer bé.
    QString configurationFile = "/etc/starviewer/log.conf";
    if (!QFile::exists(configurationFile))
    {
        configurationFile = qApp->applicationDirPath() + "/log.conf";
    }

    LOGGER_INIT(configurationFile.toStdString());
    DEBUG_LOG("Arxiu de configuració del log: " + configurationFile);
}

void initializeTranslations(QApplication &app)
{
    udg::ApplicationTranslationsLoader translationsLoader(&app);
    // Li indiquem la locale corresponent
    QLocale defaultLocale = translationsLoader.getDefaultLocale();
    QLocale::setDefault(defaultLocale);

    translationsLoader.loadTranslation(":/crashreporter_" + defaultLocale.name());
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    configureLogging();

    INFO_LOG("============================================ CRASH REPORTER BEGIN =============================================");
    INFO_LOG(QString("%1 Crash Reporter Version %2 BuildID %3").arg(udg::ApplicationNameString).arg(udg::StarviewerVersionString).arg(udg::StarviewerBuildID));

    int returnValue;
    // Seguint les recomanacions de la documentació de Qt, guardem la llista d'arguments en una variable, ja que aquesta operació és costosa
    // http://doc.trolltech.com/4.7/qcoreapplication.html#arguments
    QStringList commandLineArgumentsList = app.arguments();
    if (commandLineArgumentsList.size() != 3)
    {
        returnValue = false;
    }
    else
    {
        app.setOrganizationName(udg::OrganizationNameString);
        app.setOrganizationDomain(udg::OrganizationDomainString);
        app.setApplicationName(udg::ApplicationNameString);

        udg::CoreSettings coreSettings;
        coreSettings.init();

        initializeTranslations(app);

        udg::QCrashReporter reporter(commandLineArgumentsList);
        reporter.show();

        returnValue = app.exec();
    }

    INFO_LOG(QString("%1 Crash Reporter Version %2 BuildID %3, returnValue %4").arg(udg::ApplicationNameString).arg(udg::StarviewerVersionString)
             .arg(udg::StarviewerBuildID).arg(returnValue));
    INFO_LOG("============================================= CRASH REPORTER END ==============================================");

    return returnValue;
}
