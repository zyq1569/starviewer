#include <QDir>
#include <QApplication>

#include "../core/starviewerapplication.h"
#include "../core/logging.h"
#include <QProcess>

INITIALIZE_EASYLOGGINGPP

void configureLogging()
{
    QDir logDir = udg::UserLogsPath;
    if (!logDir.exists())
    {
        logDir.mkpath(udg::UserLogsPath);
    }
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    QString logDirFilename = udg::UserLogsPath+"/starviewer_sapwrapper.log";
    defaultConf.set(el::Level::Global,el::ConfigurationType::Filename, logDirFilename.toStdString());
    el::Loggers::reconfigureLogger("default", defaultConf);

}

void printHelp()
{
    printf("Invalid arguments: you must specify one parameter, the accession number of the study to retrieve.\n\n");
    printf("\t Example calling 'starviewer_sapwrapper 123456' will retrieve the study with accession number 123456.\n\n");
}

QString getStarviewerExecutableFilePath()
{
#ifdef _WIN32
    // In windows to be able to run the starviewer we must keep in mind that if it is in a directory that contains spaces
    // like the C: \ Program Files \ Starviewer \ starviewer.exe directory, we have to put the path in quotes
    // so that it does not interpret it as parameters, for example "C: \ Program Files \ Starviewer \ starviewer.exe"

    // We add quotes in case any of the directory contains space
    return "\"" + QCoreApplication::applicationDirPath() + "/starviewer.exe" + "\"";
#else
    return QCoreApplication::applicationDirPath() + "/starviewer";
#endif
}

/// Engega un starviewer passant-li per comandes de línia el accessionNumber del estudi a descarragar
void retrieveStudy(QString accessionNumber)
{
    QProcess process;
    QString starviewerCommandLine = " -accessionnumber " + accessionNumber;

    // Run an instance of Starviewer using the command line option -accessionnumber "value of accessio number"
    //INFO_LOG("Starviewer_sapwrapper :: New instance of Starviewer will start to request download of the studio with accession number" + accessionNumber);
    process.startDetached(getStarviewerExecutableFilePath() + starviewerCommandLine);
}

int main(int argc, char *argv[])
{
    configureLogging();

    QApplication application(argc, argv);
    QStringList parametersList = application.arguments();

    application.setOrganizationName(udg::OrganizationNameString);
    application.setOrganizationDomain(udg::OrganizationDomainString);
    application.setApplicationName(udg::ApplicationNameString);


    if (parametersList.count() == 2)
    {
        // Hem d'agafar el segon paràmetre perquè el primer és el nom del programa
        retrieveStudy(parametersList.at(1));
    }
    else
    {
        //INFO_LOG(QString("StarviewerSAPWrapper::Número de parametres incorrecte, s'han passat %1 parametres").arg(QString().setNum(argc - 1)));
        printHelp();
    }
}
