#include "logging.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include <QDir>
#include <QApplication>

// Global definitions of application
#include "starviewerapplication.h"
#include <QProcess>


/// We print the program help
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

/// Start a starviewer by going through line commands the accessionNumber of the studio to download
void retrieveStudy(QString accessionNumber)
{
    QProcess process;
    QString starviewerCommandLine = " -accessionnumber " + accessionNumber;

    //We run an instance of Starviewer using the command line option -accessionnumber "accessio value number"

    INFO_LOG("Starviewer_sapwrapper::New instance of Starviewer will start to request download of the studio with accession number" + accessionNumber);
    process.startDetached(getStarviewerExecutableFilePath() + starviewerCommandLine);
}

int main(int argc, char *argv[])
{
    int returnValue = 0;
    QApplication application(argc, argv);
    udg::beginLogging();
    INFO_LOG("====================== BEGIN STARVIEWER SAP WRAPPER ===============================");
    INFO_LOG(QString("%1 Version %2 BuildID %3").arg(udg::ApplicationNameString).
             arg(udg::StarviewerVersionString).arg(udg::StarviewerBuildID));

    QStringList parametersList = application.arguments();

    application.setOrganizationName(udg::OrganizationNameString);
    application.setOrganizationDomain(udg::OrganizationDomainString);
    application.setApplicationName(udg::ApplicationNameString);


    if (parametersList.count() == 2)
    {
        // We need to take the second parameter because the first is the name of the program
        retrieveStudy(parametersList.at(1));
    }
    else
    {
        INFO_LOG(QString("StarviewerSAPWrapper :: Incorrect number of parameters, %1 parameters passed").arg(QString().setNum(argc - 1)));
        printHelp();
    }

    INFO_LOG(QString("%1 Version %2 BuildID %3, returnValue %4").arg(udg::ApplicationNameString)
             .arg(udg::StarviewerVersionString).arg(udg::StarviewerBuildID).arg(returnValue));
    INFO_LOG("===================================================== END STARVIEWER SAP WRAPPER =====================================================");

    return returnValue;
}
