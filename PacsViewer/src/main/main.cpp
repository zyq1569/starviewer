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
//全局应用程序定义
#include "starviewerapplication.h"
//全局应用程序定义
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
    QDir logDir = udg::UserLogsPath;
    if (!logDir.exists())
    {
        logDir.mkpath(udg::UserLogsPath);
    }
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    QString logDirFilename = udg::UserLogsPath+"/starviewer.log";
    defaultConf.set(el::Level::Global,el::ConfigurationType::Filename, logDirFilename.toStdString());
    el::Loggers::reconfigureLogger("default", defaultConf);

    // We redirect VTK messages to the log.
    udg::LoggingOutputWindow *loggingOutputWindow = udg::LoggingOutputWindow::New();
    vtkOutputWindow::SetInstance(loggingOutputWindow);
    loggingOutputWindow->Delete();
}

void initializeTranslations(QApplication &app)
{
    udg::ApplicationTranslationsLoader translationsLoader(&app);
    // We indicate the corresponding premises
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
                ERROR_LOG("The translator could not be loaded: " + translationFilePath);
            }
            delete mediator;
        }
        else
        {
            ERROR_LOG("Error loading mediator from: " + mediatorName);
        }
    }
}

/// Add the directories where to look for Qt plugins. Useful in windows.
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
        ERROR_LOG("The argument list could not be sent to the main instance, the primary instance does not appear to respond.");
        QMessageBox::critical(NULL, udg::ApplicationNameString, QObject::tr("%1 is already running, but is not responding. "
                                                                            "To open %1, you must first close the existing %1 process, or restart your system.").arg(udg::ApplicationNameString));
    }
    else
    {
        INFO_LOG("The command line arguments were successfully sent to the main instance.");
    }
}

int main(int argc, char *argv[])
{
    // ALL of this initial setup process should be encapsulated in
    // a class dedicated to that purpose
    configureLogging();

    // We use QtSingleApplication instead of QtApplication, as it allows us to always have a single instance of Starviewer running, if the user runs
    // a new instance of Starviewer detects this and sends the command line with which the user has executed the new main instance.
    // 使用QtSingleApplication而不是QtApplication，因为如果用户运行，始终可以运行单个Starviewer实例
    // Starviewer的新实例检测到此情况，并发送用户执行新主实例的命令行。

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

    // We initialize the crash handler in case we support it.
    // You only need to create the object for it to self-register automatically, so we mark it as unused to avoid a warning.
#ifndef NO_CRASH_REPORTER
    CrashHandler *crashHandler = new CrashHandler();
    Q_UNUSED(crashHandler);
#endif


    // Mark the start of the application in the log
    INFO_LOG("==================================================== BEGIN ====================================================");
    INFO_LOG(QString("%1 Version %2 BuildID %3").arg(udg::ApplicationNameString).arg(udg::StarviewerVersionString).arg(udg::StarviewerBuildID));

    // We initialize the settings
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

    // ALL this is necessary to, among other things, be able to create thumbnails,
    // dicomdirs, etc. of compressed dicoms and treat them correctly with dcmtk
    // this is temporarily here, in the long run I will go to a setup class
    // register the JPEG and RLE decompressor codecs
    DJDecoderRegistration::registerCodecs();
    DcmRLEDecoderRegistration::registerCodecs();

    // Following the recommendations of the Qt documentation, we save the list of arguments in a variable, as this operation is expensive
    // http://doc.trolltech.com/4.7/qcoreapplication.html#arguments
    QStringList commandLineArgumentsList = app.arguments();

    QString commandLineCall = commandLineArgumentsList.join(" ");
    INFO_LOG("Started Starviewer instance with the following command line arguments: " + commandLineCall);

    // We just parse the command line arguments to see if they are correct, we'll wait until everything is loaded by
    // process them, if the arguments are not correct show QMessagebox if there is another instance of Starviewer we end here.
    // 我们只是解析命令行参数以查看它们是否正确，我们将等到所有内容加载完毕后，
    // 处理它们，如果参数不正确，则显示QMessagebox（如果还有另一个Starviewer实例），我们在这里结束。
    if (commandLineArgumentsList.count() > 1)
    {
        QString errorInvalidCommanLineArguments;
        if (!StarviewerSingleApplicationCommandLineSingleton::instance()->parse(commandLineArgumentsList, errorInvalidCommanLineArguments))
        {
            QString invalidCommandLine = QObject::tr("There were errors invoking %1 from the command line with the following call:\n\n%2")
                    .arg(udg::ApplicationNameString).arg(commandLineCall) + "\n\n";
            invalidCommandLine += QObject::tr("Detected errors: ") + errorInvalidCommanLineArguments + "\n";
            invalidCommandLine += StarviewerSingleApplicationCommandLineSingleton::instance()->getStarviewerApplicationCommandLineOptions().getSynopsis();
            QMessageBox::warning(NULL, udg::ApplicationNameString, invalidCommandLine);

            ERROR_LOG("Invalid command line arguments, error : " + errorInvalidCommanLineArguments);

            // If there is already another instance running we give the error message and close Starviewer
            if (app.isRunning())
            {
                return 0;
            }
        }
    }
    int returnValue;
    if (app.isRunning())
    {
        // There is another instance of Starviewer running
        //starviewer已经正在运行
        WARN_LOG("Another instance of starviewer is running. Command line arguments will be sent to the main instance.");

        sendToFirstStarviewerInstanceCommandLineOptions(app);

        returnValue = 0;
    }
    else
    {
        INFO_LOG("start to creat QApplicationMainWindow");
        udg::QApplicationMainWindow *mainWin = new udg::QApplicationMainWindow; // Main instance, no more running
        // We connect to receive arguments from other instances
        QObject::connect(&app, SIGNAL(messageReceived(QString)), StarviewerSingleApplicationCommandLineSingleton::instance(), SLOT(parseAndRun(QString)));

        INFO_LOG("Created main window");

        mainWin->show();

        QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
        splash.close();

        // It is expected to have everything loaded to process the arguments received by command line, this way by exemoke if it throws any
        // QMessageBox, already launched showing the MainWindow.
        if (commandLineArgumentsList.count() > 1)
        {
            QString errorInvalidCommanLineArguments;
            StarviewerSingleApplicationCommandLineSingleton::instance()->parseAndRun(commandLineArgumentsList, errorInvalidCommanLineArguments);
        }

        returnValue = app.exec();
    }

    // We mark the end of the application in the log
    INFO_LOG(QString("%1 Version %2 BuildID %3, returnValue %4").arg(udg::ApplicationNameString).
             arg(udg::StarviewerVersionString).arg(udg::StarviewerBuildID).arg(returnValue));
    INFO_LOG("===================================================== END =====================================================");

    return returnValue;
}
