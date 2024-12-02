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

#include "extensionhandler.h"

// Qt
#include <QFileInfo>
#include <QDir>
#include <QProgressDialog>
#include <QMessageBox>
// Recursos
#include "logging.h"
#include "extensionworkspace.h"
#include "qapplicationmainwindow.h"
#include "volume.h"
#include "extensionmediatorfactory.h"
#include "extensionfactory.h"
#include "extensioncontext.h"
#include "singleton.h"
#include "starviewerapplication.h"
#include "interfacesettings.h"
#include "screenmanager.h"
#include "patientcomparer.h"
#include "patient.h"
#include "image.h"
#include "qviewer.h"
// PACS --------------------------------------------
#include "queryscreen.h"
#include "patientfiller.h"

/// httpserver
#include "httpclient.h"
#include "hmanagethread.h"

///
#include <QLocalSocket>
#include <QLocalServer>

///
///
///
namespace udg
{

typedef SingletonPointer<QueryScreen> QueryScreenSingleton;
typedef Singleton<PatientComparer> PatientComparerSingleton;

ExtensionHandler::ExtensionHandler(QApplicationMainWindow *mainApp, QObject *parent): QObject(parent)
{
    m_mainApp = mainApp;

    createConnections();

    /// Each time we create a new window we will close any instance of QueryScreen. So it is clearer that
    /// the window that invokes it is the one that receives the result of this one
    /// EVERYTHING It is necessary to improve the design of the interaction with the QueryScreen in
    /// order not to have problems like those exposed in the tickets
    /// # 1858, # 1018. At the moment we solve it with these hacks, but they are not a good solution
    /// TODO: Slap for the starviewer to listen to RIS requests, as all the code to listen to RIS requests is in the
    /// queryscreen we have to instantiate it at the beginning so that it listens to the requests
    disconnect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), 0, 0);
    //connect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), SLOT(processInput(QList<Patient*>, bool)));
    connect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), SLOT(patientsInput(QList<Patient*>, bool)));


    ///patientsInput    ???   why???  不知为何不能统一到 SIGNAL patientsInput
    connect(QueryScreenSingleton::instance(), SIGNAL(setPatientsThumbnail(QList<Patient*>, bool)), SLOT(setPatientsThumbnail(QList<Patient*>, bool)));
    connect(QueryScreenSingleton::instance(), SIGNAL(closed()), SLOT(queryScreenIsClosed()));
    m_haveToCloseQueryScreen = false;


    ///-----------20201207---------
    m_httpclient = NULL;
    m_localserver =  new QLocalServer(this);
    connect(m_localserver, SIGNAL(newConnection()), this, SLOT(newClientConnection()));
    QLocalServer::removeServer(ImageAppName);
    if(m_localserver->listen(ImageAppName))
    {
        INFO_LOG("----Listen succeed!----");
    }
    else
    {
        ERROR_LOG("---Listen failed!!----");
    }
    ///---------------------
}

ExtensionHandler::~ExtensionHandler()
{
    // Every time we close a window we will force the queryscreen to close. This is done to make it clear that
    // QueryScreen <-> window where the studies are opened, are linked according to who invoked it
    // TODO All this needs a better design, but for now it avoids problems like those in ticket # 1858
    if (m_haveToCloseQueryScreen)
    {
        QueryScreenSingleton::instance()->close();
    }
}

void ExtensionHandler::queryScreenIsClosed()
{
    m_haveToCloseQueryScreen = false;
}

void ExtensionHandler::request(int who)
{
    // \ TODO: create the extension with factory :: createExtension, not as it is now
    // \ ALL numbering is completely temporary !!! this system will have to be changed
    INFO_LOG("Extension request with ID: " + QString::number(who));
    switch (who)
    {
        case 1:
            m_importFileApp.open();
            break;

        case 6:
            m_importFileApp.openDirectory();
            break;

        case 7:
            // HACK because the QueryScreen is a singleton, this causes side effects when we had
            // two windows (see ticket # 542). Let's do this little hack so that this doesn't happen.
            // It remains to be resolved in the appropriate way
            disconnect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), 0, 0);
            QueryScreenSingleton::instance()->showPACSTab();
            connect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), SLOT(processInput(QList<Patient*>, bool)));
            m_haveToCloseQueryScreen = true;
            break;

        case 8:
            // HACK because the QueryScreen is a singleton, this causes side effects when we had
            // two windows (see ticket # 542). Let's do this little hack so that this doesn't happen.
            // It remains to be resolved in the appropriate way
            disconnect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), 0, 0);
            QueryScreenSingleton::instance()->openDicomdir();
            connect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), SLOT(processInput(QList<Patient*>, bool)));
            m_haveToCloseQueryScreen = true;
            break;

        case 10:
            // Show location
            // HACK because the QueryScreen is a singleton, this causes side effects when we had
            // two windows (see ticket # 542). Let's do this little hack so that this doesn't happen.
            // It remains to be resolved in the appropriate way
            disconnect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), 0, 0);
            QueryScreenSingleton::instance()->showLocalExams();
            connect(QueryScreenSingleton::instance(), SIGNAL(selectedPatients(QList<Patient*>, bool)), SLOT(processInput(QList<Patient*>, bool)));
            m_haveToCloseQueryScreen = true;
            break;
    }
}

bool ExtensionHandler::createExtension(const QString &who, QString tableText)
{
    bool ok = true;
    ///template <typename T>
    ///class Singleton {
    ///
    ///class ExtensionMediator : public QObject {
    ///
    ///typedef Singleton<GenericFactory<ExtensionMediator, QString> > ExtensionMediatorFactory;
    ///
    ///template <class BaseClass, typename ClassIdentifier, class ParentType = QObject>
    ///class GenericFactory {...}
    ExtensionMediator *mediator = ExtensionMediatorFactory::instance()->create(who);
    if (!mediator)
    {
        WARN_LOG("Could not create mediator for: " + who);
        DEBUG_LOG("Could not create mediator for: " + who);
        ok = false;
        return ok;
    }
	bool bQ3DViewerExtension = ("Q3DViewerExtension" == who);
    bool createExtension = true;
    int extensionIndex = 0;
    QString requestedExtensionLabel = mediator->getExtensionID().getLabel();
    if (!Settings().getValue(InterfaceSettings::AllowMultipleInstancesPerExtension).toBool())
    {
        //We only want one instance per extension
        //We must then check that the extension we are requesting is not already created
        int count = m_mainApp->getExtensionWorkspace()->count();
        bool found = false;
        while (extensionIndex < count && !found)
        {
            if (m_mainApp->getExtensionWorkspace()->tabText(extensionIndex).contains(requestedExtensionLabel))
            {
                found = true;
            }
            else
            {
                extensionIndex++;
            }
        }
        //If we find it, we won't have to create it again
        if (found)
        {
            createExtension = false;
        }
    }

    // Depending on the existing settings and extensions, we may or may not create the requested extension
    if (createExtension)
    {
        ///template <typename T>
        ///class Singleton {
        ///
        /// typedef Singleton<GenericFactory<QWidget, QString, QWidget> > ExtensionFactory;
        ///
        /// template <class BaseClass, typename ClassIdentifier, class ParentType = QObject>
        ///  class GenericFactory {...}
        QWidget *extension = ExtensionFactory::instance()->create(who);
        if (extension)
        {
            INFO_LOG("Activate extension: " + who);
            if ("Q2DViewerExtension" == who)
            {
                //(Q2DViewerExtension*)extension->m_extensionHandler = this;
                mediator->executionCommand(extension, NULL, this, 0);
            }
			else if (bQ3DViewerExtension)
			{
				requestedExtensionLabel = "3D-Viewer#Series:" + tableText;
			}
            mediator->initializeExtension(extension, m_extensionContext);
            m_mainApp->getExtensionWorkspace()->addApplication(extension, requestedExtensionLabel, who);

        }
        else
        {
            ok = false;
            DEBUG_LOG("Error carregant " + who);
        }
    }
    else
    {
        //Otherwise we show the already existing extension
        m_mainApp->getExtensionWorkspace()->setCurrentIndex(extensionIndex);
		if (bQ3DViewerExtension)
		{
			Volume * selVolume = QViewer::selectVolume();
			Volume *volume = selVolume ? selVolume : m_extensionContext.getDefaultVolumeNoLocalizer();
			if (volume && volume->is3Dimage())
			{
				m_mainApp->getExtensionWorkspace()->setTabText(extensionIndex, "3D-Viewer#Series:" + volume->getSeries()->getSeriesNumber());
				QWidget* widget = m_mainApp->currentWidgetOfExtensionWorkspace();
				if (widget)
				{
					mediator->executionCommand(widget, volume);
				}
			}
		}
    }

    delete mediator;

    return ok;

}
bool ExtensionHandler::request(const QString &who)
{
	if (!m_mainApp->getCurrentPatient())
	{
		return true;
	}
	QString tableText;
	if (who == "Q3DViewerExtension")
	{
		Volume * selVolume = QViewer::selectVolume();		
		Volume *volume = selVolume ? selVolume: m_extensionContext.getDefaultVolumeNoLocalizer();
		if (!volume)
		{
			QMessageBox::warning(0, "3D-Viewer", ("3D-Viewer: No image is selected!!"));
			//QMessageBox::information(0, tr("Starviewer"), tr("The selected item is not an image"));
			return true;
		}
		if (!volume->is3Dimage())
		{
			QMessageBox::warning(0, "3D-Viewer", ("The selected item : 3D-Viewer fail!!! images < 5 or SliceThickness = 0.0"));
			return true;
		}	
		tableText = volume->getSeries()->getSeriesNumber();
	}
	else if (who == "MPR3DExtension")
	{
		Volume * selVolume = QViewer::selectVolume();
		if (!selVolume)
		{
			QViewer::selectVolume(m_extensionContext.getDefaultVolumeNoLocalizer());
		}
		Volume *volume = selVolume ? selVolume : m_extensionContext.getDefaultVolumeNoLocalizer();
		if (!volume)
		{
			QMessageBox::warning(0, "MPR-2DViewer", ("MPR-2D fail: No image is selected!!"));
			return true;
		}
		if (volume->getNumberOfSlicesPerPhase() < 5)
		{
			QMessageBox::warning(0, "MPR-2DViewer", ("The selected item :MPR-2D fail!!! images < 5!!"));
			return true;
		}

	}
    QList<Patient*> Patients;
    Patients<<m_mainApp->getCurrentPatient();
    setPatientsThumbnail(Patients);
    return createExtension(who, tableText);
}

///20201205
void ExtensionHandler::setPatientsThumbnail(QList<Patient*> patientsList, bool loadOnly)
{
    m_mainApp->addPatientsThumbnail(patientsList);
}

void ExtensionHandler::setContext(const ExtensionContext &context)
{
    m_extensionContext = context;
}

ExtensionContext &ExtensionHandler::getContext()
{
    return m_extensionContext;
}

void ExtensionHandler::createConnections()
{
    connect(&m_importFileApp, SIGNAL(selectedFiles(QStringList)), SLOT(processInput(QStringList)));
}


void ExtensionHandler::processInput(const QStringList &inputFiles)
{
    if (inputFiles.isEmpty())
    {
        return;
    }

    QProgressDialog progressDialog(m_mainApp);
    progressDialog.setModal(true);
    progressDialog.setRange(0, 0);
    progressDialog.setMinimumDuration(0);
    progressDialog.setWindowTitle(tr("Patient Loading"));
    progressDialog.setLabelText(tr("Loading, please wait..."));
    progressDialog.setCancelButton(0);

    qApp->processEvents();
    PatientFiller patientFiller;
    connect(&patientFiller, SIGNAL(progress(int)), &progressDialog, SLOT(setValue(int)));

    QList<Patient*> patientsList = patientFiller.processFiles(inputFiles);

    progressDialog.close();

    int numberOfPatients = patientsList.size();

    if (numberOfPatients == 0)
    {
        QMessageBox::critical(0, ApplicationNameString, tr("Sorry, it seems that there is no patient data that can be loaded."));
        ERROR_LOG("Error making patientFiller child. 0 patients returned.");
        return;
    }

    DEBUG_LOG(QString("NumberOfPatients: %1").arg(numberOfPatients));

    QList<int> correctlyLoadedPatients;

    for (int i = 0; i < numberOfPatients; i++)
    {
        DEBUG_LOG(QString("Patient # %1\n %2").arg(i).arg(patientsList.at(i)->toString()));

        bool error = true;

        // We mark the selected series
        QList<Study*> studyList = patientsList.at(i)->getStudies();
        if (!studyList.isEmpty())
        {
            Study *study = studyList.first();

            QList<Series*> seriesList = study->getSeries();
            if (!seriesList.isEmpty())
            {
                Series *series = seriesList.first();
                series->select();
                error = false;
            }
            else
            {
                ERROR_LOG(patientsList.at(i)->toString());
                ERROR_LOG("Error loading this patient. The returned series is null.");
            }
        }
        else
        {
            ERROR_LOG(patientsList.at(i)->toString());
            ERROR_LOG("Error loading this patient. Returned studio is null.");
        }

        if (!error)
        {
            correctlyLoadedPatients << i;
        }
    }

    QString patientsWithError;
    if (correctlyLoadedPatients.count() != numberOfPatients)
    {
        for (int i = 0; i < numberOfPatients; i++)
        {
            if (!correctlyLoadedPatients.contains(i))
            {
                patientsWithError += "- " + patientsList.at(i)->getFullName() + "; ID: " + patientsList.at(i)->getID() + "<br>";
            }
        }
        QMessageBox::critical(0, ApplicationNameString, tr("Sorry, an error occurred while loading data from patients:<br>%1").arg(patientsWithError));
    }
    if (patientsWithError.isEmpty())
    {
        // There are no mistakes, we load them all
        processInput(patientsList);
        ///----------add patientsList--------------------
        setPatientsThumbnail(patientsList);
        ///----------------------------------------------
    }
    else
    {
        // We only load the correct ones
        QList<Patient*> rightPatients;
        foreach (int index, correctlyLoadedPatients)
        {
            rightPatients << patientsList.at(index);
        }
        processInput(rightPatients);
        ///----------add patientsList--------------------
        setPatientsThumbnail(rightPatients);
        ///----------------------------------------------
    }
}

void ExtensionHandler::patientsInput(QList<Patient*> patientsList, bool loadOnly )
{
    setPatientsThumbnail(patientsList);
    processInput(patientsList,loadOnly);
}

void ExtensionHandler::processInput(QList<Patient*> patientsList, bool loadOnly)
{
    QList<Patient*> mergedPatientsList = mergePatients(patientsList);
    /// If we try to load one of all the patients that is
    /// loaded the same as the one we have already loaded, we keep it
    bool canReplaceActualPatient = true;
    if (m_mainApp->getCurrentPatient())
    {
        QListIterator<Patient*> patientsIterator(mergedPatientsList);
        while (canReplaceActualPatient && patientsIterator.hasNext())
        {
            Patient *patient = patientsIterator.next();
            QListIterator<QApplicationMainWindow*> mainAppsIterator(QApplicationMainWindow::getQApplicationMainWindows());
            while (canReplaceActualPatient && mainAppsIterator.hasNext())
            {
                QApplicationMainWindow *mainApp = mainAppsIterator.next();
                m_patientComparerMutex.lock();
                canReplaceActualPatient = !PatientComparerSingleton::instance()->areSamePatient(mainApp->getCurrentPatient(), patient);
                m_patientComparerMutex.unlock();
            }
        }
    }

    bool firstPatient = true;

    /// We add correctly loaded patients
    foreach (Patient *patient, mergedPatientsList)
    {
        generatePatientVolumes(patient, QString());
        QApplicationMainWindow *mainApp = this->addPatientToWindow(patient, canReplaceActualPatient, loadOnly);

        if (mainApp)
        {
            if (mainApp->isMinimized())
            {
                /// Sand the Starviewer window is minimized we return it to its original state when viewing the study
                ScreenManager().restoreFromMinimized(mainApp);
            }

            if (firstPatient)
            {
                mainApp->activateWindow();
            }
        }
        firstPatient = false;

        /// Once a patient is loaded, we can no longer replace him
        canReplaceActualPatient = false;
    }
}

QList<Patient*> ExtensionHandler::mergePatients(const QList<Patient*> &patientList)
{
    QList<Patient*> mergedList;
    Patient *currentPatient = 0;
    QList<Patient*> workList = patientList;

    while (!workList.isEmpty())
    {
        currentPatient = workList.takeFirst();
        foreach (Patient *patientToCheck, workList)
        {
            if (currentPatient->compareTo(patientToCheck) == Patient::SamePatients)
            {
                *currentPatient += *patientToCheck;
                workList.removeAll(patientToCheck);
            }
        }
        mergedList << currentPatient;
    }

    return mergedList;
}

void ExtensionHandler::generatePatientVolumes(Patient *patient, const QString &defaultSeriesUID)
{
    Q_UNUSED(defaultSeriesUID);
    foreach (Study *study, patient->getStudies())
    {
        // For each series, if your images are multiframe or of different sizes from each other they will go in separate volumes
        foreach (Series *series, study->getViewableSeries())
        {
            int currentVolumeNumber;
            QMap<int, QList<Image*> > volumesImages;
            foreach (Image *image, series->getImages())
            {
                currentVolumeNumber = image->getVolumeNumberInSeries();
                if (volumesImages.contains(currentVolumeNumber))
                {
                    volumesImages[currentVolumeNumber] << image;
                }
                else
                {
                    QList<Image*> newImageList;
                    newImageList << image;
                    volumesImages.insert(currentVolumeNumber, newImageList);
                }
            }
            typedef QList<Image*> ImageListType;
            foreach (ImageListType imageList, volumesImages)
            {
                int numberOfPhases = 1;
                bool found = false;
                int i = 0;
                while (!found && i<imageList.count() - 1)
                {
                    if (imageList.at(i + 1)->getPhaseNumber() > imageList.at(i)->getPhaseNumber())
                    {
                        numberOfPhases++;
                    }
                    else
                    {
                        found = true;
                    }
                    i++;
                }
                int numberOfSlicesPerPhase = imageList.count() / numberOfPhases;

                Volume *volume = new Volume;
                volume->setImages(imageList);
                volume->setNumberOfPhases(numberOfPhases);
                volume->setNumberOfSlicesPerPhase(numberOfSlicesPerPhase);
                volume->setThumbnail(imageList.at(imageList.count() / 2)->getThumbnail(true));
                series->addVolume(volume);
            }
        }
    }
    DEBUG_LOG(QString("Patient:\n%1").arg(patient->toString()));
}

QApplicationMainWindow* ExtensionHandler::addPatientToWindow(Patient *patient, bool canReplaceActualPatient, bool loadOnly)
{
    QApplicationMainWindow *usedMainApp = NULL;

    if (canReplaceActualPatient && !loadOnly)
    {
        m_mainApp->setPatient(patient);
        usedMainApp = m_mainApp;
    }
    else
    {
        QApplicationMainWindow *mainApp;
        bool found = false;

        QListIterator<QApplicationMainWindow*> mainAppsIterator(QApplicationMainWindow::getQApplicationMainWindows());
        while (!found && mainAppsIterator.hasNext())
        {
            mainApp = mainAppsIterator.next();
            m_patientComparerMutex.lock();
            found = PatientComparerSingleton::instance()->areSamePatient(mainApp->getCurrentPatient(), patient);
            m_patientComparerMutex.unlock();
        }

        if (found)
        {
            mainApp->connectPatientVolumesToNotifier(patient);
            *(mainApp->getCurrentPatient()) += *patient;
            DEBUG_LOG("We already had data on this patient. We merge information");

            // Let's see if there are any open extensions, but open the default one
            if (mainApp->getExtensionWorkspace()->count() == 0)
            {
                openDefaultExtension();
            }

            if (!loadOnly)
            {
                QMap<QWidget*, QString> extensions = mainApp->getExtensionWorkspace()->getActiveExtensions();
                QMapIterator<QWidget*, QString> iterator(extensions);

                while (iterator.hasNext())
                {
                    iterator.next();
                    ExtensionMediator *mediator = ExtensionMediatorFactory::instance()->create(iterator.value());
                    mediator->viewNewStudiesFromSamePatient(iterator.key(), patient->getStudies().first()->getInstanceUID());
                }
            }

            usedMainApp = mainApp;
        }
        else
        {
            if (!loadOnly)
            {
                usedMainApp = m_mainApp->setPatientInNewWindow(patient);
                DEBUG_LOG("We have a patient and they don’t let us replace him. We open it in a new window.");
            }
            else
            {
                /// \ TODO Given the problems explained in ticket # 1087,
                /// deleting objects here can make the application crash if you request studies
                /// without explicitly going through the QueryScreen. Since no volume has
                /// been loaded into memory at this point,
                /// therefore there will be no major memory leaks if we do not delete it,
                /// we get to not delete any object and thus minimize the problem.
            }
        }
    }
    return usedMainApp;
}

void ExtensionHandler::openDefaultExtension()
{
    if (m_mainApp->getCurrentPatient())
    {
        // TODO If there are no images nor documents it would be better to inform the
        // user than to open an extension, but a message box doesn't let the tests
        // continue, so for the moment we keep the old behaviour for this scenario.
        if (m_extensionContext.hasImages() || !m_extensionContext.hasEncapsulatedDocuments())
        {
            Settings settings;
            QString defaultExtension = settings.getValue(InterfaceSettings::DefaultExtension).toString();
            if (!createExtension(defaultExtension))
            {
                WARN_LOG("The request for the default extension called failed: " + defaultExtension
                         + ". We start default 2D extension (hardcoded)");
                createExtension("Q2DViewerExtension");
            }
        }

        if (m_extensionContext.hasEncapsulatedDocuments())
        {
            createExtension("PdfExtension");
        }
    }
    else
    {
        DEBUG_LOG("No patient data!");
    }
}


///////////////////////////2020-12-07///////////////////////////////////////////////

void  ExtensionHandler::newClientConnection()
{
    m_clientSocket = m_localserver->nextPendingConnection();

    connect(m_clientSocket, SIGNAL(readyRead()), this, SLOT(fromClientNotify()));
    connect(m_clientSocket, SIGNAL(connected()), this, SLOT(socketConnect()));
    connect(m_clientSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnect()));
    connect(m_clientSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
    connect(m_clientSocket, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), this, SLOT(socketStateChanged(QLocalSocket::LocalSocketState)));

}

void ExtensionHandler::httpServerDownAllDcm()
{
    if (m_httpclient)
    {
        QStringList list = *m_httpclient->getListStudyUID();
        httpServerInput(list);
    }
}

void ExtensionHandler::httpServerInput(const QStringList &inputFiles)
{
    processInput(inputFiles);
}

void ExtensionHandler::fromClientNotify()
{
    if(!m_clientSocket)
    {
        return;
    }
    QString msg = m_clientSocket->readAll();

    //QMessageBox::information(NULL, tr("STUDY"),msg);
    QString receive = "receive StudyUID:" + msg + " \n on time:" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh:mm:ss-zzz");
    m_clientSocket->write(receive.toUtf8());
    m_clientSocket->flush();

    //-----------------------------------------------------------------------------
    QStringList list = msg.split("&");
    QString HttpServerHost, DownDir,Studyuid;
    if (list.size() >3)
    {
        HttpServerHost = list[0];
        DownDir = list[1];
        Studyuid = list[2]+"&"+list[3];//http://127.0.0.1:80&D:/bak/HealthApp2023/MHealthReport/cache&1.2.826.0.1.3680043.9.7604..16050010109105933.20000&studyDate=20010109105933
        QDir downdir(DownDir);
        if(!downdir.exists())
        {
            QDir createDir(DownDir); // 注意
            createDir.setPath("");
            if (!createDir.mkpath(DownDir))
            {
                // error!
            }
        }
        if (!m_httpclient)
        {
            m_httpclient = new HttpClient(NULL,DownDir/*"F:/log/down"*/);
            connect(m_httpclient, SIGNAL(allFilesFinished()), this,  SLOT(httpServerDownAllDcm()));
        }
        m_httpclient->setHttpServerHost(HttpServerHost/*"http://127.0.0.1:8080"*/);
        m_httpclient->getStudyImageFile(QUrl(HttpServerHost/*m_httpclient->getHttpServerHost()*/), Studyuid/*msg*/, "", "");
        ///---HWND_TOPMOST--->HWND_NOTOPMOST
        ::SetWindowPos(HWND(m_mainApp->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        ::SetWindowPos(HWND(m_mainApp->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        m_mainApp->show();
        m_mainApp->activateWindow();
        //------------------------------------------------------------------------------
    }
    INFO_LOG("fromClientNotify: " + msg);
}

void ExtensionHandler::socketConnect()
{
    INFO_LOG("A new connection");
}

void ExtensionHandler::socketDisconnect()
{
    INFO_LOG("Disconnected");
}

void ExtensionHandler::socketError(/*QLocalSocket::LocalSocketError*/int socketError)
{
    INFO_LOG("socketError");
}

void ExtensionHandler::socketStateChanged(/*QLocalSocket::LocalSocketState*/int socketState)
{
    INFO_LOG("socketStateChanged");
}


void ExtensionHandler::readFromServer(int fd)
{
    INFO_LOG("readFromServer");
}

void  ExtensionHandler::closeCurrentPatient()
{
    if (m_mainApp)
    {
       m_mainApp->closePatient();
    }

}

void ExtensionHandler::processCommandInput(const QStringList &inputFiles)
{
	processInput(inputFiles);
}
}   // end namespace udg
