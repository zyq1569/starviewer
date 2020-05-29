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

#include "qapplicationmainwindow.h"

#include "logging.h"
#include "statswatcher.h"
#include "extensions.h"
#include "extensionmediatorfactory.h"
#include "diagnosistests.h"
#include "syncactionsregister.h"
// Definicions globals d'aplicació
#include "starviewerapplication.h"
// Necessaris per suportar la decodificació de jpeg i RLE
#include <djdecode.h>
#include <dcrledrg.h>
#include "applicationtranslationsloader.h"

#include "coresettings.h"
#include "inputoutputsettings.h"
#include "interfacesettings.h"
#include "shortcuts.h"
#include "starviewerapplicationcommandline.h"
#include "applicationcommandlineoptions.h"
#include "loggingoutputwindow.h"
#include "vtkinit.h"

#ifndef NO_CRASH_REPORTER
#include "crashhandler.h"
#endif

#include <QApplication>
#include <QLabel>
#include <QDesktopWidget>
#include <QLocale>
#include <QTextCodec>
#include <QDir>
#include <QMessageBox>
#include <QLibraryInfo>
#include <qtsingleapplication.h>

#include <vtkNew.h>
#include <vtkOutputWindow.h>
#include <vtkOverrideInformation.h>
#include <vtkOverrideInformationCollection.h>

typedef udg::SingletonPointer<udg::StarviewerApplicationCommandLine> StarviewerSingleApplicationCommandLineSingleton;

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
    // Afegim localització per Mac OS X en desenvolupament
    if (!QFile::exists(configurationFile))
    {
        configurationFile = qApp->applicationDirPath() + "/../../../log.conf";
    }

    LOGGER_INIT(configurationFile.toStdString());
    DEBUG_LOG("Arxiu de configuració del log: " + configurationFile);

    // Redirigim els missatges de VTK cap al log.
    udg::LoggingOutputWindow *loggingOutputWindow = udg::LoggingOutputWindow::New();
    vtkOutputWindow::SetInstance(loggingOutputWindow);
    loggingOutputWindow->Delete();
}

void initializeTranslations(QApplication &app)
{
    udg::ApplicationTranslationsLoader translationsLoader(&app);
    // Li indiquem la locale corresponent
    QLocale defaultLocale = translationsLoader.getDefaultLocale();
    QLocale::setDefault(defaultLocale);

    translationsLoader.loadTranslation("qt_" + defaultLocale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    translationsLoader.loadTranslation(":/core/core_" + defaultLocale.name());
    translationsLoader.loadTranslation(":/interface/interface_" + defaultLocale.name());
    translationsLoader.loadTranslation(":/inputoutput/inputoutput_" + defaultLocale.name());

    initExtensionsResources();
    INFO_LOG("Locales = " + defaultLocale.name());

    QStringList extensionsMediatorNames = udg::ExtensionMediatorFactory::instance()->getFactoryIdentifiersList();
    foreach (const QString &mediatorName, extensionsMediatorNames)
    {
        udg::ExtensionMediator *mediator = udg::ExtensionMediatorFactory::instance()->create(mediatorName);

        if (mediator)
        {
            QString translationFilePath = ":/extensions/" + mediator->getExtensionID().getID() + "/translations_" + defaultLocale.name();
            if (!translationsLoader.loadTranslation(translationFilePath))
            {
                ERROR_LOG("No s'ha pogut carregar el translator " + translationFilePath);
            }
            delete mediator;
        }
        else
        {
            ERROR_LOG("Error carregant el mediator de " + mediatorName);
        }
    }
}

/// Afegeix els directoris on s'han de buscar els plugins de Qt. Útil a windows.
void initQtPluginsDirectory()
{
#ifdef Q_OS_WIN32
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");
#endif
}

void sendToFirstStarviewerInstanceCommandLineOptions(QtSingleApplication &app)
{
    QString errorInvalidCommanLineArguments;

    if (!app.sendMessage(app.arguments().join(";"), 10000))
    {
        ERROR_LOG("No s'ha pogut enviar a la instancia principal la llista d'arguments, sembla que l'instancia principal no respon.");
        QMessageBox::critical(NULL, udg::ApplicationNameString, QObject::tr("%1 is already running, but is not responding. "
            "To open %1, you must first close the existing %1 process, or restart your system.").arg(udg::ApplicationNameString));
    }
    else
    {
        INFO_LOG("S'ha enviat correctament a la instancia principal els arguments de la línia de comandes.");
    }
}

int main(int argc, char *argv[])
{
    // Utilitzem QtSingleApplication en lloc de QtApplication, ja que ens permet tenir executant sempre una sola instància d'Starviewer, si l'usuari executa
    // una nova instància d'Starviewer aquesta ho detecta i envia la línia de comandes amb que l'usuari ha executat la nova instància principal.

    QtSingleApplication app(argc, argv);

    QPixmap splashPixmap;
    #ifdef STARVIEWER_LITE
    splashPixmap.load(":/images/splashLite.png");
    #else
    splashPixmap.load(":/images/splash.png");
    #endif
    QLabel splash(0, Qt::SplashScreen|Qt::FramelessWindowHint);
    splash.setAttribute(Qt::WA_TranslucentBackground);
    splash.setPixmap(splashPixmap);
    splash.resize(splashPixmap.size());
    splash.move(QApplication::desktop()->screenGeometry().center() - splash.rect().center());

    if (!app.isRunning())
    {
        splash.show();
    }

    app.setOrganizationName(udg::OrganizationNameString);
    app.setOrganizationDomain(udg::OrganizationDomainString);
    app.setApplicationName(udg::ApplicationNameString);

#ifndef NO_CRASH_REPORTER
    // Inicialitzem el crash handler en el cas que ho suportem.
    // Només cal crear l'objecte per què s'autoregistri automàticament, per això el marquem com no utilitzat per evitar un warning.
    CrashHandler *crashHandler = new CrashHandler();
    Q_UNUSED(crashHandler);
#endif

    // TODO tot aquest proces inicial de "setups" hauria d'anar encapsulat en
    // una classe dedicada a tal efecte

    configureLogging();

    // Marquem l'inici de l'aplicació al log
    INFO_LOG("==================================================== BEGIN ====================================================");
    INFO_LOG(QString("%1 Version %2 BuildID %3").arg(udg::ApplicationNameString).arg(udg::StarviewerVersionString).arg(udg::StarviewerBuildID));

    // Inicialitzem els settings
    udg::CoreSettings coreSettings;
    udg::InputOutputSettings inputoutputSettings;
    udg::InterfaceSettings interfaceSettings;
    udg::Shortcuts shortcuts;

    coreSettings.init();
    inputoutputSettings.init();
    interfaceSettings.init();
    shortcuts.init();

    initQtPluginsDirectory();
    initializeTranslations(app);

    // Registering the available sync actions
    udg::SyncActionsRegister::registerSyncActions();

    // TODO aixo es necessari per, entre d'altres coses, poder crear thumbnails,
    // dicomdirs, etc de dicoms comprimits i tractar-los correctament amb dcmtk
    // aixo esta temporalment aqui, a la llarga anira a una classe de setup
    // registrem els codecs decompressors JPEG i RLE
    DJDecoderRegistration::registerCodecs();
    DcmRLEDecoderRegistration::registerCodecs();

    // Seguint les recomanacions de la documentació de Qt, guardem la llista d'arguments en una variable, ja que aquesta operació és costosa
    // http://doc.trolltech.com/4.7/qcoreapplication.html#arguments
    QStringList commandLineArgumentsList = app.arguments();

    QString commandLineCall = commandLineArgumentsList.join(" ");
    INFO_LOG("Iniciada nova instancia Starviewer amb el seguents arguments de linia de comandes " + commandLineCall);

    if (commandLineArgumentsList.count() > 1)
    {
        // Només parsegem els arguments de línia de comandes per saber si són correctes, ens esperem més endavant a que tot estigui carregat per
        // processar-los, si els arguments no són correctes mostre QMessagebox si hi ha una altra instància d'Starviewer finalitzem aquí.
        QString errorInvalidCommanLineArguments;
        if (!StarviewerSingleApplicationCommandLineSingleton::instance()->parse(commandLineArgumentsList, errorInvalidCommanLineArguments))
        {
            QString invalidCommandLine = QObject::tr("There were errors invoking %1 from the command line with the following call:\n\n%2")
                                                     .arg(udg::ApplicationNameString).arg(commandLineCall) + "\n\n";
            invalidCommandLine += QObject::tr("Detected errors: ") + errorInvalidCommanLineArguments + "\n";
            invalidCommandLine += StarviewerSingleApplicationCommandLineSingleton::instance()->getStarviewerApplicationCommandLineOptions().getSynopsis();
            QMessageBox::warning(NULL, udg::ApplicationNameString, invalidCommandLine);

            ERROR_LOG("Arguments de linia de comandes invalids, error : " + errorInvalidCommanLineArguments);

            // Si ja hi ha una altra instància execuntat-se donem el missatge d'error i tanquem Starviewer
            if (app.isRunning())
            {
                return 0;
            }
        }
    }

    int returnValue;
    if (app.isRunning())
    {
        // Hi ha una altra instància del Starviewer executant-se
        INFO_LOG("Hi ha una altra instancia de l'starviewer executant-se. S'enviaran els arguments de la linia de comandes a la instancia principal.");

        sendToFirstStarviewerInstanceCommandLineOptions(app);

        returnValue = 0;
    }
    else
    {
        // Instància principal, no n'hi ha cap més executant-se
        udg::QApplicationMainWindow *mainWin = new udg::QApplicationMainWindow;
        // Fem el connect per rebre els arguments de les altres instàncies
        QObject::connect(&app, SIGNAL(messageReceived(QString)), StarviewerSingleApplicationCommandLineSingleton::instance(), SLOT(parseAndRun(QString)));

        INFO_LOG("Creada finestra principal");

        mainWin->show();

        QObject::connect(&app, SIGNAL(lastWindowClosed()),
                         &app, SLOT(quit()));
        splash.close();

        // S'ha esperat a tenir-ho tot carregat per processar els aguments rebuts per línia de comandes, d'aquesta manera per exemoke si en llança algun
        // QMessageBox, ja es llança mostrant-se la MainWindow.
        if (commandLineArgumentsList.count() > 1)
        {
            QString errorInvalidCommanLineArguments;
            StarviewerSingleApplicationCommandLineSingleton::instance()->parseAndRun(commandLineArgumentsList, errorInvalidCommanLineArguments);
        }

        returnValue = app.exec();
    }

    // Marquem el final de l'aplicació al log
    INFO_LOG(QString("%1 Version %2 BuildID %3, returnValue %4").arg(udg::ApplicationNameString).arg(udg::StarviewerVersionString)
             .arg(udg::StarviewerBuildID).arg(returnValue));
    INFO_LOG("===================================================== END =====================================================");

    return returnValue;
}
