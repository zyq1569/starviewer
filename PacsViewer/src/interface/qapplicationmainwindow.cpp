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

#include "extensionhandler.h"
#include "extensionworkspace.h"
#include "logging.h"
#include "qlogviewer.h"
#include "patient.h"
#include "qconfigurationdialog.h"
#include "volume.h"
#include "settings.h"
#include "extensionfactory.h"
#include "extensionmediatorfactory.h"
#include "starviewerapplication.h"
#include "statswatcher.h"
#include "databaseinstallation.h"
#include "interfacesettings.h"
#include "starviewerapplicationcommandline.h"
#include "risrequestwrapper.h"
#include "qaboutdialog.h"
#include "externalapplication.h"
#include "externalapplicationsmanager.h"
#include "queryscreen.h"
#include "risrequestmanager.h"
#include "qviewer.h"
//------------------------------------
#include "imagethumbnaildockwidget.h"
#include "patientbrowsermenu.h"
//------------------------------------
// Pel LanguageLocale
#include "coresettings.h"
#include "inputoutputsettings.h"
#include "applicationversionchecker.h"
#include "screenmanager.h"
#include "qscreendistribution.h"
#include "volumerepository.h"
#include "qdiagnosistest.h"

// Amb starviewer lite no hi haurà hanging protocols, per tant no els carregarem
#ifndef STARVIEWER_LITE
#include "hangingprotocolsloader.h"
#include "customwindowlevelsloader.h"
#include "studylayoutconfigsloader.h"
#endif

// Qt
#include <QAction>
#include <QSignalMapper>
#include <QMenuBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QApplication>
#include <QLocale>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QPair>
#include <QWidgetAction>
#include <QShortcut>
#include <QToolBar>
/////////
#include <QSplitter>
#include <QDesktopWidget>
// Shortucts
#include "shortcuts.h"
#include "shortcutmanager.h"

namespace udg
{

/// To process the options entered by command line we have to use a Singleton of
/// StarviewerApplicationCommandLine, this comes because
/// of QApplicationMainWindow instances we have as many as Starviewer open
/// windows we have. Instances of QApplicationMainWindow are created
/// and are destroyed as a new window opens or a Starviewer window closes
/// so we can't hold anyone responsible
/// QApplicationMainWindow to handle requests received via arguments or
/// received from other instances of Starviewer via
/// from QtSingleApplication, because we can't guarantee that no QApplicationMainWindow
/// will be alive throughout the execution of Starviewer, to take care of
/// to process command line arugments.

/// That's why all QApplicationMainWindow is connected to a signal from the same instance of
/// StarviewerSingleApplicationCommandLineSingleton, this signal is newOptionsToRun ()
/// that is output each time new arguments are received
/// proceed from the same instance at startup or from other instances via QtSingleApplication.
/// Once the signal has been issued the instances
/// from QApplicationMainWindow as they respond to the signal with the takeOptionToRun ()
/// method they process all the arguments until they don't
/// there is none left to process.

/// The option that processes an instance of QApplicationMainWindow obtained through the
/// takeOptionToRun () method disappears from the list of options
/// to process StarviewerApplicationCommandLine, so even though all instances
///  of QApplicationMainWindow can process
/// options received, each option will only be processed by the first instance
///  that takes it through the takeOptionToRun () method.
///
/// template<typename T>
/// class SingletonPointer {...}
typedef SingletonPointer<StarviewerApplicationCommandLine> StarviewerSingleApplicationCommandLineSingleton;

QApplicationMainWindow::QApplicationMainWindow(QWidget *parent)
    : QMainWindow(parent), m_patient(0), m_isBetaVersion(false)
{
    connect(StarviewerSingleApplicationCommandLineSingleton::instance(), SIGNAL(newOptionsToRun()), SLOT(newCommandLineOptionsToRun()));

    this->setAttribute(Qt::WA_DeleteOnClose);
    ///class ExtensionWorkspace : public QTabWidget
    m_extensionWorkspace = new ExtensionWorkspace(this);
    this->setCentralWidget(m_extensionWorkspace);

	//setAutoFillBackground(true);
	//setPalette(QPalette(QColor(0, 0, 0)));
    ///-------add QDockWidget-----------------------------------------------------
	//setWindowFlags(Qt::FramelessWindowHint);
    m_DockImageThumbnail = new ImageThumbnailDockWidget("",this);//("Thumbnail");
    addDockWidget(Qt::LeftDockWidgetArea,m_DockImageThumbnail);
    //m_DockImageThumbnail->setFeatures(QDockWidget::DockWidgetMovable);
    m_DockImageThumbnail->setObjectName("ImageThumbnail");

#ifdef DOCKRIGHT
	m_DockImageThumbnailRight = new ImageThumbnailDockWidget("", this);//("Thumbnail");
	addDockWidget(Qt::RightDockWidgetArea, m_DockImageThumbnailRight);
	m_DockImageThumbnailRight->setObjectName("ImageThumbnailRight");
#endif
    /// addd connect(m_tab, SIGNAL(currentChanged(int)), SLOT(refreshTab(int)));
    //connect(m_extensionWorkspace, SIGNAL(currentChanged(int)),m_DockImageThumbnail, SLOT(refreshTab(int)));
    //-----------------------------------------------------------------------------

    DatabaseInstallation databaseInstallation;
    if (!databaseInstallation.checkDatabase())
    {
        QString errorMessage = databaseInstallation.getErrorMessage();
        QMessageBox::critical(0, ApplicationNameString, tr("There have been some errors:") + "\n" + errorMessage + "\n\n" +
                              tr("You can resolve this error at Tools > Configuration > Local Database."));
    }

    m_extensionHandler = new ExtensionHandler(this);

    m_logViewer = new QLogViewer(this);

	//20240729
    //createActions();
    //createMenus();

	//add m_mainToolbar
	m_mainToolbar = new QToolBar(this);
	this->addToolBar(Qt::TopToolBarArea, m_mainToolbar);
	m_mainToolbar->setIconSize(QSize(30, 30));
	m_mainToolbar->layout()->setSpacing(10);
	m_mainToolbar->setFloatable(false);
	m_mainToolbar->setMovable(false);
	m_mainToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	//m_mainToolbar->setStyleSheet("background-color:rgb(150,150,150)}");
	//m_mainToolbar->setStyleSheet("background-color:lightgray;");
	//m_mainToolbar->setStyleSheet("background-color:rgb(128,128,128);");
	//m_mainToolbar->setStyleSheet("QToolButton:!hover {background-color:lightgray} QToolBar {background: rgb(150,150,150)}");
	m_mainToolbar->setStyleSheet("QToolButton:!hover {background-color:lightgray} QToolBar {background:lightgray}");
	//this->setStyleSheet("background-color:lightgray}");
	//QAction *actionHide = new QAction(QIcon(":/images/showhide.png"), "show or hide Thumbnail ...", this);
	//m_mainToolbar->addAction(actionHide);
	//connect(actionHide, SIGNAL(triggered()), SLOT(showhideDockImage()));//Open an existing DICOM folder

    QAction *actionFile = new QAction(QIcon(":/images/folderopen.png"), "Open Files from a Directory...", this);
	m_mainToolbar->addAction(actionFile);
	connect(actionFile, &QAction::triggered, [this] { m_extensionHandler->request(6); });//Open an existing DICOM folder
	m_mainToolbar->insertSeparator(actionFile);	

	QAction *actionFileMHD = new QAction(QIcon(":/images/file_MHD.png"), "MetaIO Image (*.mhd)", this);
	m_mainToolbar->addAction(actionFileMHD);
	connect(actionFileMHD, &QAction::triggered, [this] { m_extensionHandler->request(1); });//Open an existing MetaIO Image(*.mhd)"
	m_mainToolbar->insertSeparator(actionFileMHD);

	QAction *action3D = new QAction(QIcon(":/images/icons/3D.svg"), "3D Viewer", this);
	m_mainToolbar->addAction(action3D);
	connect(action3D, &QAction::triggered, [this] { m_extensionHandler->request("Q3DViewerExtension"); });


	QAction *actionMPR = new QAction(QIcon(":/images/icons/MPR3D.svg"), "MPR-3D Viewer", this);
	m_mainToolbar->addAction(actionMPR);
	connect(actionMPR, &QAction::triggered, [this] { m_extensionHandler->request("QMPR3DExtension"); });

	QAction *actionDicomPrint = new QAction(QIcon(":/images/icons/printer.svg"), "Dicom Print", this);
	m_mainToolbar->addAction(actionDicomPrint);
	connect(actionDicomPrint, &QAction::triggered, [this] { m_extensionHandler->request("QDicomPrintExtension"); });
	//QAction *actionMultScreens = new QAction(QIcon(":/images/icons/Monitor.svg"), "MultiScreens", this);
	//m_mainToolbar->addAction(actionMultScreens);
	//connect(actionMultScreens, SIGNAL(triggered(bool)), this, SLOT(maximizeMultipleScreens()));

	//QAction *actionNextScreens = new QAction(QIcon(":/images/icons/MonitorNext.svg"), "Next Desktop", this);
	//m_mainToolbar->addAction(actionNextScreens);
	//actionNextScreens->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::MoveToNextDesktop));
	//actionNextScreens->setToolTip("Next Desktop|Ctrl + Shift + Right");
	//connect(actionNextScreens, SIGNAL(triggered(bool)), this, SLOT(moveToNextDesktop()));

	QMenu* menu = new QMenu("windows", this);
	QAction* nextScreens = menu->addAction("NextScreens");
	QMenu* menuSub = new QMenu("...", this); //创建第二个menu对象
	nextScreens->setMenu(menuSub);
	nextScreens->setIcon(QIcon(":/images/icons/MonitorNext.svg"));
	nextScreens->setToolTip("Next Desktop | Ctrl + Shift + Right");
	nextScreens->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::MoveToNextDesktop));
	connect(nextScreens, SIGNAL(triggered(bool)), this, SLOT(moveToNextDesktop()));
	QScreenDistribution *screen = new QScreenDistribution(this);
	QWidgetAction* subDesk = new QWidgetAction(this);
	subDesk->setDefaultWidget(screen);
	menuSub->addAction(subDesk);
	connect(screen, SIGNAL(screenClicked(int)), this, SLOT(moveToDesktop(int)));
	m_mainToolbar->addAction(nextScreens);

	QAction *actionPACS = new QAction(QIcon(":/images/pacsNodes"), "PACS Images", this);
	m_mainToolbar->addAction(actionPACS);
	connect(actionPACS, &QAction::triggered, [this] { m_extensionHandler->request(7); });

	QAction *actionConfig = new QAction(QIcon(":/images/preferences.png"), "&Configuration...", this);
	actionConfig->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::Preferences));
	m_mainToolbar->addAction(actionConfig);
	connect(actionConfig, SIGNAL(triggered()), SLOT(showConfigurationDialog()));
	
	QMenu* menuhelp = new QMenu("AppHelp", this);
	QAction* aboutAction = menuhelp->addAction("&About");
	QMenu* menuhelpSub = new QMenu("...", this); //创建第二个menu对象
	aboutAction->setMenu(menuhelpSub);
	aboutAction->setIcon(QIcon(":/images/help.ico"));
	aboutAction->setToolTip("Show the application's About box");
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
	
	QAction *sysInfoTest = new QAction(QIcon(":/images/help.ico"), "SysTest", this);
	menuhelpSub->addAction(sysInfoTest);
	connect(sysInfoTest, SIGNAL(triggered()), this, SLOT(showDiagnosisTestDialog()));
	m_mainToolbar->addAction(aboutAction);
	//---------------------------------------------------------------------------------------------------
    // We read the application settings, window status, position, etc.
    readSettings();
    // Application icon
    this->setWindowIcon(QIcon(":/images/logo/logo.ico"));
    this->setWindowTitle(ApplicationNameString/*+"-MainWindow"*/);

    //With starviewer lite there will be no hanging protocols, so we will not load them
#ifndef STARVIEWER_LITE
    // Load the repositories we need to have loaded throughout the application
    // We will only load once per session / instance of starviewer
    static bool repositoriesLoaded = false;
    if (!repositoriesLoaded)
    {
        HangingProtocolsLoader hangingProtocolsLoader;
        hangingProtocolsLoader.loadDefaults();

        CustomWindowLevelsLoader customWindowLevelsLoader;
        customWindowLevelsLoader.loadDefaults();

        StudyLayoutConfigsLoader layoutConfigsLoader;
        layoutConfigsLoader.load();

        repositoriesLoaded = true;
    }
#endif

    // We create the progress dialog that will notify the volume loading
    m_progressDialog = new QProgressDialog(this);
    m_progressDialog->setModal(true);
    m_progressDialog->setRange(0, 100);
    m_progressDialog->setMinimumDuration(1);
    m_progressDialog->setWindowTitle(tr("Loading"));
    m_progressDialog->setLabelText(tr("Loading data, please wait..."));
    m_progressDialog->setCancelButton(0);
    m_progressDialog->setValue(100);

#ifdef BETA_VERSION
    markAsBetaVersion();
    showBetaVersionDialog();
#endif

    m_statsWatcher = new StatsWatcher("Menu triggering", this);
    m_statsWatcher->addTriggerCounter(m_fileMenu);
    m_statsWatcher->addTriggerCounter(m_visualizationMenu);
    m_statsWatcher->addTriggerCounter(m_toolsMenu);
    m_statsWatcher->addTriggerCounter(m_helpMenu);
    m_statsWatcher->addTriggerCounter(m_languageMenu);
    m_statsWatcher->addTriggerCounter(m_windowMenu);

	//20240816
	connect(QViewer::getStaticBrowserMenu(), SIGNAL(selectedVolumes(QList<Volume*>)), this, SLOT(updateActiveFromStaticViewerMenu(QList<Volume*>)));
	//connect(QViewer::getStaticBrowserMenu(), SIGNAL(selectedVolume(Volume*)), this, SLOT(updateActiveFromStaticViewerMenu(QList<Volume*>)));
	connect(QViewer::getStaticBrowserMenu(), &PatientBrowserMenu::selectedVolume, [=](Volume* vl)
																				{
																					QList<Volume*> vols;
																					updateActiveFromStaticViewerMenu(vols<<vl);
																				}
	);
}

QApplicationMainWindow::~QApplicationMainWindow()
{
    writeSettings();
    this->killBill();
    delete m_extensionWorkspace;
    delete m_extensionHandler;
}

void QApplicationMainWindow::checkNewVersionAndShowReleaseNotes()
{
    ApplicationVersionChecker::checkAndShowReleaseNotes();
}

void QApplicationMainWindow::createActions()
{
    m_newAction = new QAction(this);
    m_newAction->setText(tr("&New Window"));
    m_newAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::NewWindow));
    m_newAction->setStatusTip(tr("Open a new working window"));
    m_newAction->setIcon(QIcon(":/images/icons/window-new.svg"));
    connect(m_newAction, SIGNAL(triggered()), SLOT(openBlankWindow()));

    m_openAction = new QAction(this);
    m_openAction->setText(tr("&Open Files..."));
    m_openAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::OpenFile));
    m_openAction->setStatusTip(tr("Open one or several existing volume files"));
    m_openAction->setIcon(QIcon(":/images/icons/document-open.svg"));
    connect(m_openAction, &QAction::triggered, [this] { m_extensionHandler->request(1); });

    m_openDirAction = new QAction(this);
    m_openDirAction->setText(tr("Open Files from a Directory..."));
    m_openDirAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::OpenDirectory));
    m_openDirAction->setStatusTip(tr("Open an existing DICOM folder"));
    m_openDirAction->setIcon(QIcon(":/images/icons/document-open.svg"));
    connect(m_openDirAction, &QAction::triggered, [this] { m_extensionHandler->request(6); });

    m_pacsAction = new QAction(this);
#ifdef STARVIEWER_LITE
    // The "PACS" menu will be called "Exams"
    m_pacsAction->setText(tr("&Exams..."));
    m_pacsAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::OpenExams));
    m_pacsAction->setStatusTip(tr("Browse exams"));
#else
    m_pacsAction->setText(tr("&PACS..."));
    m_pacsAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::OpenPACS));
    m_pacsAction->setStatusTip(tr("Open PACS Query Screen"));

    m_localDatabaseAction = new QAction(this);
    m_localDatabaseAction->setText(tr("&Local Database Studies..."));
    m_localDatabaseAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::OpenLocalDatabaseStudies));
    m_localDatabaseAction->setStatusTip(tr("Browse local database studies"));
    m_localDatabaseAction->setIcon(QIcon(":/images/icons/database-local.svg"));
    connect(m_localDatabaseAction, &QAction::triggered, [this] { m_extensionHandler->request(10); });
#endif
    // TODO maybe at least for the Lite version the icon would have to be changed
    m_pacsAction->setIcon(QIcon(":/images/icons/document-open-remote.svg"));
    connect(m_pacsAction, &QAction::triggered, [this] { m_extensionHandler->request(7); });

    m_openDICOMDIRAction = new QAction(this);
    m_openDICOMDIRAction->setText(tr("Open DICOMDIR..."));
    m_openDICOMDIRAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::OpenDICOMDIR));
    m_openDICOMDIRAction->setStatusTip(tr("Open DICOMDIR from CD, DVD, USB flash drive or hard disk"));
    m_openDICOMDIRAction->setIcon(QIcon(":/images/icons/document-open-dicomdir.svg"));
    connect(m_openDICOMDIRAction, &QAction::triggered, [this] { m_extensionHandler->request(8); });

    QStringList extensionsMediatorNames = ExtensionMediatorFactory::instance()->getFactoryIdentifiersList();
    extensionsMediatorNames.removeAll("ExampleExtension");
    extensionsMediatorNames.removeAll("RectumSegmentationExtension");
    extensionsMediatorNames.removeAll("PerfusionMapReconstructionExtension");
    foreach(const QString &name, extensionsMediatorNames)
    {
        ///20200924 add   name == ???  delete other QAction-------------------------------------------------------------------------
        ///Comment out uncommon functions temporarily , keep follow  four QAction
        //if (name != "Example" || name != "Rectum Segmentation" || name != "Perfusion Map Reconstruction")// || name != "Q3DViewerExtension")
        //{
        ExtensionMediator *mediator = ExtensionMediatorFactory::instance()->create(name);
        if (mediator)
        {
            QAction *action = new QAction(this);
            const DisplayableID &extensionId = mediator->getExtensionID();
            QString lable = extensionId.getLabel();
            action->setText(lable);
            action->setStatusTip(tr("Open the %1 Application").arg(mediator->getExtensionID().getLabel()));
            action->setEnabled(false);
            //m_signalMapper->setMapping(action, mediator->getExtensionID().getID());
            //connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
            connect(action, &QAction::triggered, [=] { m_extensionHandler->request(extensionId.getID()); });
            m_actionsList.append(action);
            delete mediator;
        }
        else
        {
            ERROR_LOG("Error loading mediator from " + name);
        } //}
    }

    m_maximizeAction = new QAction(this);
    m_maximizeAction->setText(tr("Maximize to Multiple Screens"));
    m_maximizeAction->setStatusTip(tr("Maximize the window to as many screens as possible"));
    m_maximizeAction->setCheckable(false);
    m_maximizeAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::MaximizeMultipleScreens));
    connect(m_maximizeAction, SIGNAL(triggered(bool)), this, SLOT(maximizeMultipleScreens()));

    m_moveToDesktopAction = new QWidgetAction(this);
    QScreenDistribution *screenDistribution = new QScreenDistribution(this);
    m_moveToDesktopAction->setDefaultWidget(screenDistribution);
    m_moveToDesktopAction->setText(tr("Move to Screen"));
    m_moveToDesktopAction->setStatusTip(tr("Move the window to the screen..."));
    m_moveToDesktopAction->setCheckable(false);
    connect(screenDistribution, SIGNAL(screenClicked(int)), this, SLOT(moveToDesktop(int)));

    m_moveToPreviousDesktopAction = new QAction(this);
    m_moveToPreviousDesktopAction->setText(tr("Move to previous screen"));
    m_moveToPreviousDesktopAction->setStatusTip(tr("Move the window to the previous screen"));
    m_moveToPreviousDesktopAction->setCheckable(false);
    m_moveToPreviousDesktopAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::MoveToPreviousDesktop));
    connect(m_moveToPreviousDesktopAction, SIGNAL(triggered(bool)), SLOT(moveToPreviousDesktop()));

    m_moveToNextDesktopAction = new QAction(this);
    m_moveToNextDesktopAction->setText(tr("Move to next screen"));
    m_moveToNextDesktopAction->setStatusTip(tr("Move the window to the next screen"));
    m_moveToNextDesktopAction->setCheckable(false);
    m_moveToNextDesktopAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::MoveToNextDesktop));
    connect(m_moveToNextDesktopAction, SIGNAL(triggered(bool)), SLOT(moveToNextDesktop()));

    m_openUserGuideAction = new QAction(this);
    m_openUserGuideAction->setText(tr("User Guide"));
    m_openUserGuideAction->setStatusTip(tr("Open user guide"));
    connect(m_openUserGuideAction, SIGNAL(triggered()), this, SLOT(openUserGuide()));

    m_openQuickStartGuideAction = new QAction(this);
    m_openQuickStartGuideAction->setText(tr("Quick Start Guide"));
    m_openQuickStartGuideAction->setStatusTip(tr("Open quick start guide"));
    connect(m_openQuickStartGuideAction, SIGNAL(triggered()), this, SLOT(openQuickStartGuide()));

    m_openShortcutsGuideAction = new QAction(this);
    m_openShortcutsGuideAction->setText(tr("Shortcuts Guide"));
    m_openShortcutsGuideAction->setStatusTip(tr("Open shortcuts guide"));
    connect(m_openShortcutsGuideAction, SIGNAL(triggered()), this, SLOT(openShortcutsGuide()));

    m_logViewerAction = new QAction(this);
    m_logViewerAction->setText(tr("Show Log File"));
    m_logViewerAction->setStatusTip(tr("Show log file"));
    m_logViewerAction->setIcon(QIcon(":/images/icons/show-log.svg"));
    connect(m_logViewerAction, SIGNAL(triggered()), m_logViewer, SLOT(updateData()));
    connect(m_logViewerAction, SIGNAL(triggered()), m_logViewer, SLOT(exec()));

    m_openReleaseNotesAction = new QAction(this);
    m_openReleaseNotesAction->setText(tr("&Release Notes"));
    m_openReleaseNotesAction->setStatusTip(tr("Show the application's release notes for current version"));
    connect(m_openReleaseNotesAction, SIGNAL(triggered()), SLOT(openReleaseNotes()));

    m_aboutAction = new QAction(this);
    m_aboutAction->setText(tr("&About"));
    m_aboutAction->setStatusTip(tr("Show the application's About box"));
    m_aboutAction->setIcon(QIcon(":/images/logo/logo.ico"));
    connect(m_aboutAction, SIGNAL(triggered()), SLOT(about()));

    m_closeAction = new QAction(this);
    m_closeAction->setText(tr("&Close"));
    m_closeAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::CloseCurrentExtension));
    m_closeAction->setStatusTip(tr("Close current extension page"));
    m_closeAction->setIcon(QIcon(":/images/icons/project-development-close.svg"));
    //connect(m_closeAction, SIGNAL(triggered()), m_extensionWorkspace, SLOT(closeCurrentApplication()));
    connect(m_closeAction, SIGNAL(triggered()), this, SLOT(closeCurrentPatient()));

    //----------20200921---------------------------------------------------------------------------------
    //connect(m_closeAction, SIGNAL(triggered()), m_DockImageThumbnail, SLOT(mainAppclearThumbnail()));
    //----------------------------------------------------------------------------------------------------
    m_exitAction = new QAction(this);
    m_exitAction->setText(tr("E&xit"));
    m_exitAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::CloseApplication));
    m_exitAction->setStatusTip(tr("Exit the application"));
    m_exitAction->setIcon(QIcon(":/images/icons/application-exit.svg"));
    connect(m_exitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    m_configurationAction = new QAction(this);
    m_configurationAction->setText(tr("&Configuration..."));
    m_configurationAction->setShortcuts(ShortcutManager::getShortcuts(Shortcuts::Preferences));
    m_configurationAction->setStatusTip(tr("Modify %1 configuration").arg(ApplicationNameString));
    m_configurationAction->setIcon(QIcon(":/images/icons/configure.svg"));
    connect(m_configurationAction, SIGNAL(triggered()), SLOT(showConfigurationDialog()));

    m_runDiagnosisTestsAction = new QAction(this);
    m_runDiagnosisTestsAction->setText(tr("&Run Diagnosis Tests"));
    m_runDiagnosisTestsAction->setStatusTip(tr("Run %1 diagnosis tests").arg(ApplicationNameString));
    connect(m_runDiagnosisTestsAction, SIGNAL(triggered()), SLOT(showDiagnosisTestDialog()));

}

void QApplicationMainWindow::maximizeMultipleScreens()
{
	static bool bflag = true;
	if (bflag)
	{
		ScreenManager screenManager;
		screenManager.maximize(this);
		bflag = false;
	}
	else
	{
		ScreenManager screenManager;
		screenManager.moveToNextDesktop(this);
		bflag = true;
	}

}

void QApplicationMainWindow::moveToDesktop(int screenIndex)
{
    ScreenManager screenManager;
    screenManager.moveToDesktop(this, screenIndex);
}

void QApplicationMainWindow::moveToPreviousDesktop()
{
    ScreenManager screenManager;
    screenManager.moveToPreviousDesktop(this);
}

void QApplicationMainWindow::moveToNextDesktop()
{
    ScreenManager screenManager;
    screenManager.moveToNextDesktop(this);
}

void QApplicationMainWindow::showConfigurationDialog()
{
    QConfigurationDialog configurationDialog;
    configurationDialog.exec();
}

void QApplicationMainWindow::createMenus()
{
    // File menu
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAction);
#ifndef STARVIEWER_LITE
    m_fileMenu->addAction(m_localDatabaseAction);
#endif
    m_fileMenu->addAction(m_pacsAction);
    m_fileMenu->addAction(m_openDICOMDIRAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addAction(m_openDirAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_closeAction);
    m_fileMenu->addAction(m_exitAction);

#ifdef STARVIEWER_LITE
    // We do not add display menus
#else
    // Actions related to visualization
    m_visualizationMenu = menuBar()->addMenu(tr("&Visualization"));

    foreach (QAction *action, m_actionsList)
    {
        m_visualizationMenu->addAction(action);
    }
#endif

    //Tools menu
    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    m_languageMenu = m_toolsMenu->addMenu(tr("&Language"));
    createLanguageMenu();
    m_toolsMenu->addAction(m_configurationAction);
    m_toolsMenu->addAction(m_runDiagnosisTestsAction);
    m_externalApplicationsMenu = 0;
    createExternalApplicationsMenu();
    connect(ExternalApplicationsManager::instance(), SIGNAL(onApplicationsChanged()), this, SLOT(createExternalApplicationsMenu()));

    // Menu 'window'
    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    m_moveWindowToDesktopMenu = m_windowMenu->addMenu(tr("Move to Screen"));
    m_moveWindowToDesktopMenu->addAction(m_moveToDesktopAction);
    m_windowMenu->addAction(m_maximizeAction);
    m_windowMenu->addAction(m_moveToPreviousDesktopAction);
    m_windowMenu->addAction(m_moveToNextDesktopAction);

    menuBar()->addSeparator();

    // Help and support menu
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_openUserGuideAction);
    m_helpMenu->addAction(m_openQuickStartGuideAction);
    m_helpMenu->addAction(m_openShortcutsGuideAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_logViewerAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_openReleaseNotesAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_aboutAction);
}

void QApplicationMainWindow::createLanguageMenu()
{
    QMap<QString, QString> languages;
    languages.insert("ca_ES", tr("Catalan"));
    languages.insert("es_ES", tr("Spanish"));
    languages.insert("en_GB", tr("English"));

    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(switchToLanguage(QString)));

    QActionGroup *actionGroup = new QActionGroup(this);

    QMapIterator<QString, QString> i(languages);
    while (i.hasNext())
    {
        i.next();

        QAction *action = createLanguageAction(i.value(), i.key());
        signalMapper->setMapping(action, i.key());
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));

        actionGroup->addAction(action);
        m_languageMenu->addAction(action);
    }
}

void QApplicationMainWindow::createExternalApplicationsMenu()
{
    QList<ExternalApplication> externalApplications = ExternalApplicationsManager::instance()->getApplications();
    delete m_externalApplicationsMenu;

    ///If no external applications are defined, do not create the menu;
    if (externalApplications.length() == 0)
    {
        m_externalApplicationsMenu = 0;
        return;
    }

    m_externalApplicationsMenu = m_toolsMenu->addMenu(tr("&External applications"));
    m_externalApplicationsMenu->setIcon(QIcon(":/images/icons/system-run.svg"));

    QSignalMapper *signalMapper = new QSignalMapper(m_externalApplicationsMenu);
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(launchExternalApplication(int)));

    QVector<QList<QKeySequence>> shortcutVector(12);
    shortcutVector[0]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication1);
    shortcutVector[1]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication2);
    shortcutVector[2]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication3);
    shortcutVector[3]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication4);
    shortcutVector[4]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication5);
    shortcutVector[5]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication6);
    shortcutVector[6]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication7);
    shortcutVector[7]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication8);
    shortcutVector[8]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication9);
    shortcutVector[9]  = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication10);
    shortcutVector[10] = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication11);
    shortcutVector[11] = ShortcutManager::getShortcuts(Shortcuts::ExternalApplication12);

    QListIterator<ExternalApplication> i(externalApplications);
    int position = 0;
    while (i.hasNext())
    {
        const ExternalApplication& extApp = i.next();
        ///When added to a QMenu, that menu becomes the parent.
        QAction* action = new QAction(extApp.getName(),0);
        if (position < shortcutVector.size())
        {
            action->setShortcuts(shortcutVector[position]);
        }

        m_externalApplicationsMenu->addAction(action);
        signalMapper->setMapping(action, position);
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
        position++;
    }
}

QAction* QApplicationMainWindow::createLanguageAction(const QString &language, const QString &locale)
{
    Settings settings;
    QString defaultLocale = settings.getValue(CoreSettings::LanguageLocale).toString();

    QAction *action = new QAction(this);
    action->setText(language);
    action->setStatusTip(tr("Switch to %1 language").arg(language));
    action->setCheckable(true);
    action->setChecked(defaultLocale == locale);

    return action;
}

void QApplicationMainWindow::killBill()
{
    // We remove all extensions
    this->getExtensionWorkspace()->killThemAll();
    // TODO download all the volumes that the patient has in this window
    // when we destroy ourselves we release all the volumes in memory
    if (this->getCurrentPatient() != NULL)
    {
        foreach (Study *study, this->getCurrentPatient()->getStudies())
        {
            foreach (Series *series, study->getSeries())
            {
                foreach (Identifier id, series->getVolumesIDList())
                {
                    VolumeRepository::getRepository()->deleteVolume(id);
                }
            }
        }
    }
}

void QApplicationMainWindow::switchToLanguage(QString locale)
{
    Settings settings;
    settings.setValue(CoreSettings::LanguageLocale, locale);

    QMessageBox::information(this, tr("Language Switch"), tr("Changes will take effect the next time you start the application"));
}

void QApplicationMainWindow::launchExternalApplication(int i)
{
    QList<ExternalApplication> externalApplications = ExternalApplicationsManager::instance()->getApplications();
    if (i < 0 && i >= externalApplications.size())
    {
        ERROR_LOG("Trying to launch an unexistant external application");
    }
    const ExternalApplication &app = externalApplications.at(i);
    if (!ExternalApplicationsManager::instance()->launch(app))
    {
        //Launch failed.
        QMessageBox::critical(this, tr("External application launch error"), tr("There has been an error launching the external application."));
    }
}

QApplicationMainWindow* QApplicationMainWindow::setPatientInNewWindow(Patient *patient)
{
    QApplicationMainWindow *newMainWindow = openBlankWindow();
    newMainWindow->setPatient(patient);
	QList<Patient*> patientsList;
	patientsList << patient;
	newMainWindow->addPatientsThumbnail(patientsList);
    return newMainWindow;
}

QApplicationMainWindow* QApplicationMainWindow::openBlankWindow()
{
    QApplicationMainWindow *newMainWindow = new QApplicationMainWindow(0);
    newMainWindow->show();

    return newMainWindow;
}

void QApplicationMainWindow::setPatient(Patient *patient)
{
    // If patient data is null, we do nothing
    if (!patient)
    {
        DEBUG_LOG("NULL Patient, maybe creating a blank new window");
        return;
    }

    if (this->getCurrentPatient())
    {
        //First we charge the patient
        this->killBill();
        delete m_patient;
        m_patient = NULL;
        DEBUG_LOG("We already had a patient, we deleted him.");
    }

    m_patient = patient;
    connectPatientVolumesToNotifier(patient);

    ///set mainwindows Title (info of patient)
    this->setWindowTitle("patientID:"+m_patient->getID() + " : PatientName " + m_patient->getFullName() +"/"+m_patient->getSex());
    enableExtensions();
    m_extensionHandler->getContext().setPatient(patient);
    m_extensionHandler->openDefaultExtension();
}

Patient* QApplicationMainWindow::getCurrentPatient()
{
    return m_patient;
}

unsigned int QApplicationMainWindow::getCountQApplicationMainWindow()
{
    unsigned int count = 0;
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        if (qobject_cast<QApplicationMainWindow*>(widget))
        {
            ++count;
        }
    }

    return count;
}

QList<QApplicationMainWindow*> QApplicationMainWindow::getQApplicationMainWindows()
{
    QList<QApplicationMainWindow*> mainApps;
    foreach (QWidget *widget, qApp->topLevelWidgets())
    {
        QApplicationMainWindow *window = qobject_cast<QApplicationMainWindow*>(widget);
        if (window)
        {
            mainApps << window;
            /// debug test
            QSize size = window->size();
            DEBUG_LOG("window:height-"+QString("%1").arg(size.height())+"-width-"+QString("%1").arg(size.width()));
        }
    }
    return mainApps;
}

QApplicationMainWindow* QApplicationMainWindow::getActiveApplicationMainWindow()
{
    return qobject_cast<QApplicationMainWindow*>(QApplication::activeWindow());
}


ExtensionWorkspace* QApplicationMainWindow::getExtensionWorkspace()
{
    return m_extensionWorkspace;
}

void QApplicationMainWindow::closeEvent(QCloseEvent *event)
{
    /// \ TODO here we should check if the application is doing
    /// other pending tasks that should be completed before closing
    /// the application such as downloaded images from PACS or similar.
    /// It should be done centrally.
    event->accept();
}

void QApplicationMainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_isBetaVersion)
    {
        updateBetaVersionTextPosition();
    }
    QMainWindow::resizeEvent(event);
}

void QApplicationMainWindow::about()
{
    QAboutDialog *about = new QAboutDialog(this);
    about->exec();
}

void QApplicationMainWindow::writeSettings()
{
    Settings settings;
    settings.saveGeometry(InterfaceSettings::ApplicationMainWindowGeometry, this);
}

void QApplicationMainWindow::enableExtensions()
{
    foreach (QAction *action, m_actionsList)
    {
        action->setEnabled(true);
    }
}

void QApplicationMainWindow::markAsBetaVersion()
{
    m_isBetaVersion = true;
    m_betaVersionMenuText = new QLabel(menuBar());
    m_betaVersionMenuText->setText("<a href='beta'><img src=':/images/icons/emblem-warning.svg'></a>&nbsp;<a href='beta'>Beta Version</a>");
    m_betaVersionMenuText->setAlignment(Qt::AlignVCenter);
    connect(m_betaVersionMenuText, SIGNAL(linkActivated(const QString&)), SLOT(showBetaVersionDialog()));
    updateBetaVersionTextPosition();
}

void QApplicationMainWindow::updateBetaVersionTextPosition()
{
    m_betaVersionMenuText->move(this->size().width() - (m_betaVersionMenuText->sizeHint().width() + 10), 5);
}

void QApplicationMainWindow::showBetaVersionDialog()
{
    QMessageBox::warning(this, tr("Beta Version"),
                         tr("<h2>%1</h2>"
                            "<p align='justify'>This is a preview release of %1 used exclusively for testing purposes.</p>"
                            "<p align='justify'>This version is intended for radiologists and our test-team members. "
                            "Users of this version should not expect extensions to function properly.</p>"
                            "<p align='justify'>If you want to help us to improve %1, please report any found bug or "
                            "any feature request you may have by sending an e-mail to: <a href=\"mailto:%2\">%2</a></p>"
                            "<h3>We really appreciate your feedback!</h3>").arg(ApplicationNameString).arg(OrganizationEmailString));
}

void QApplicationMainWindow::readSettings()
{
    Settings settings;
    if (!settings.contains(InterfaceSettings::ApplicationMainWindowGeometry))
    {
        this->showMaximized();
    }
    else
    {
        settings.restoreGeometry(InterfaceSettings::ApplicationMainWindowGeometry, this);
    }
}

void QApplicationMainWindow::connectPatientVolumesToNotifier(Patient *patient)
{
    foreach (Study *study, patient->getStudies())
    {
        foreach (Series *series, study->getSeries())
        {
            foreach (Volume *volume, series->getVolumesList())
            {
                connect(volume, SIGNAL(progress(int)), SLOT(updateVolumeLoadProgressNotification(int)));
            }
        }
    }
}

void QApplicationMainWindow::newCommandLineOptionsToRun()
{
    QPair<StarviewerApplicationCommandLine::StarviewerCommandLineOption, QString> optionValue;

    //As long as there are options to process
    while (StarviewerSingleApplicationCommandLineSingleton::instance()->takeOptionToRun(optionValue))
    {
        switch (optionValue.first)
        {
            case StarviewerApplicationCommandLine::openBlankWindow:
                INFO_LOG("Received command line argument to open new window");
                openBlankWindow();
                break;
            case StarviewerApplicationCommandLine::retrieveStudyFromAccessioNumber:
                INFO_LOG("Received command line argument to download a study through its accession number");
                sendRequestRetrieveStudyWithAccessionNumberToLocalStarviewer(optionValue.second);
                break;
            default:
                INFO_LOG("Invalid command line argument");
                break;
        }
    }
}

void QApplicationMainWindow::sendRequestRetrieveStudyWithAccessionNumberToLocalStarviewer(QString accessionNumber)
{
    Settings settings;
    if (settings.getValue(udg::InputOutputSettings::ListenToRISRequests).toBool())
    {
        RISRequestWrapper().sendRequestToLocalStarviewer(accessionNumber);
    }
    else
    {
        //EVERYTHING: A more generic message should be made
        QMessageBox::information(this, ApplicationNameString,
                                 tr("Please activate \"Listen to RIS requests\" option in %1 configuration to retrieve studies from SAP.")
                                 .arg(ApplicationNameString));
    }
}

void QApplicationMainWindow::updateVolumeLoadProgressNotification(int progress)
{
    m_progressDialog->setValue(progress);
}

void QApplicationMainWindow::openUserGuide()
{
    QString userGuideFilePath = QCoreApplication::applicationDirPath() + "/Starviewer_User_guide.pdf";
    QDesktopServices::openUrl(QUrl::fromLocalFile(userGuideFilePath));
}

void QApplicationMainWindow::openQuickStartGuide()
{
    QString userGuideFilePath = QCoreApplication::applicationDirPath() + "/Starviewer_Quick_start_guide.pdf";
    QDesktopServices::openUrl(QUrl::fromLocalFile(userGuideFilePath));
}

void QApplicationMainWindow::openShortcutsGuide()
{
    QString userGuideFilePath = QCoreApplication::applicationDirPath() + "/Starviewer_Shortcuts_guide.pdf";
    QDesktopServices::openUrl(QUrl::fromLocalFile(userGuideFilePath));
}

void QApplicationMainWindow::showDiagnosisTestDialog()
{
    QDiagnosisTest qDiagnosisTest;
    qDiagnosisTest.execAndRunDiagnosisTest();
}

void QApplicationMainWindow::openReleaseNotes()
{
    ApplicationVersionChecker::showLocalReleaseNotes();
}

///add -------------------------------------------------------------------------------------------------------
///
void QApplicationMainWindow::clearImageThumbnailDockWidget()
{
    m_DockImageThumbnail->clearThumbmailList();
#ifdef DOCKRIGHT
	m_DockImageThumbnailRight->clearThumbmailList();
#endif
}

void QApplicationMainWindow::addPatientsThumbnail(QList<Patient*> patientsList)
{
    m_DockImageThumbnail->addPatientsThumbmailList(patientsList);
#ifdef DOCKRIGHT
	m_DockImageThumbnailRight->addPatientsThumbmailList(patientsList);
#endif
}

void QApplicationMainWindow::closePatient()
{
    if (m_DockImageThumbnail)
    {
        m_DockImageThumbnail->mainAppclearThumbnail();
    }
#ifdef DOCKRIGHT
	if (m_DockImageThumbnailRight)
	{
		m_DockImageThumbnailRight->mainAppclearThumbnail();
	}
#endif
    this->killBill();
    this->setWindowTitle("NULL");
    if (m_patient)
    {
        m_patient->clearAllStudy();
        delete m_patient;
        m_patient = NULL;
    }
}

//ExtensionHandler* QApplicationMainWindow::getExtensionHandler()
//{
//    return  m_extensionHandler;
//}

QWidget *QApplicationMainWindow::currentWidgetOfExtensionWorkspace()
{
    return m_extensionWorkspace->currentWidget();
}

void QApplicationMainWindow::closeCurrentPatient()
{
	//connect(m_closeAction, SIGNAL(triggered()), m_extensionWorkspace, SLOT(closeCurrentApplication()));
	//----------20200921---------------------------------------------------------------------------------
	//connect(m_closeAction, SIGNAL(triggered()), m_DockImageThumbnail, SLOT(mainAppclearThumbnail()));
	//----------------------------------------------------------------------------------------------------
	if (m_extensionWorkspace->currentWidget())
	{
		m_extensionWorkspace->closeCurrentApplication();
		m_DockImageThumbnail->mainAppclearThumbnail();
#ifdef DOCKRIGHT
		m_DockImageThumbnailRight->mainAppclearThumbnail();
#endif 
		this->killBill();
        this->setWindowTitle("NULL");
        //m_patient->setID("NULL");
        //m_patient->setFullName("NULL");
		if (m_patient)
		{
			//m_patient->clearAllStudy();
            delete m_patient;
            m_patient = NULL;
		}		
		
	}
}
///------------------------------------------------------------------------------------------------------------

void QApplicationMainWindow::showhideDockImage()
{
	static bool flag = true;
	if (flag)
	{
		m_DockImageThumbnail->hide();
		flag = false;
	}
	else
	{
		m_DockImageThumbnail->show();
		flag = true;
	}
}

void QApplicationMainWindow::updateActiveFromStaticViewerMenu(const QList<Volume*> &volumes)
{
	QWidget* widget = currentWidgetOfExtensionWorkspace();
	QString className = widget->metaObject()->className();
	QString str = className.section("::", 1, 1);
	ExtensionMediator *mediator = ExtensionMediatorFactory::instance()->create(str);
	if (mediator)
	{
		if (widget)
		{
			Volume *volume = volumes.at(0);
			ExtensionWorkspace *extensionWorkspace = getExtensionWorkspace();
			int extensionIndex = extensionWorkspace->currentIndex();
			if (extensionWorkspace->tabText(extensionIndex).contains("3D-Viewer"))
			{
				if (!volume)
				{
					QMessageBox::warning(0, "3D-Viewer", ("3D-Viewer: No image is selected!!"));
					delete mediator;
					return;
				}
				if (!volume->is3Dimage())
				{
					QMessageBox::warning(0, "3D-Viewer", ("The selected item : 3D-Viewer fail!!! images < 5 or SliceThickness = 0.0"));
					delete mediator;
					return;
				}
				extensionWorkspace->setTabText(extensionIndex, "3D-Viewer#Series:" + volume->getSeries()->getSeriesNumber());
			}
			mediator->executionCommand(widget, volume);
		}
		delete mediator;
	}
}

void QApplicationMainWindow::openCommandDirDcm(QString rootPath)
{
	QStringList dirsList, filenames;
	QDir rootDir(rootPath);
	if (rootDir.exists())
	{
		// We add the current directory to the list
		dirsList << rootPath;
		foreach(const QString &dirName, dirsList)
		{
			filenames << generateFilenames(dirName);
		}
	}
	if (!filenames.isEmpty())
	{
		m_extensionHandler->closeCurrentPatient();
		m_extensionHandler->processCommandInput(filenames);
	}
}

QStringList QApplicationMainWindow::generateFilenames(const QString &dirPath)
{
	QStringList list;
	//We check that the directory has files
	QDir dir(dirPath);
	QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);

	QString suffix;
	//We add to the list each of the absolute paths of the files contained in the directory
	foreach(const QFileInfo &fileInfo, fileInfoList)
	{
		suffix = fileInfo.suffix();
		if ((suffix.length() > 0 && suffix.toLower() == "dcm") || suffix.length() == 0)
		{
			list << fileInfo.absoluteFilePath();
		}
	}

	return list;
}

} // end namespace udg
