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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// QT
#include <QMainWindow>
#include <QStringList>

// Forward declarations
class QAction;
class QMenu;
class QCloseEvent;
class QShowEvent;
class QSignalMapper;
class QLabel;
class QProgressDialog;
class QWidgetAction;
class QShortcut;
class QSplitter;



namespace udg {
// Forward declarations
class ExtensionWorkspace;
class ExtensionHandler;
class ExtensionContext;
class QLogViewer;
class Patient;
class StatsWatcher;
class ExternalApplication;
class ImageThumbnailDockWidget;

class QApplicationMainWindow : public QMainWindow {
    Q_OBJECT
public:
    /// The builder of the Mainwindow. This is where the entire interface skeleton is created: Menus,
    ///  Toolbars, toolbox, working area, etc.
    QApplicationMainWindow(QWidget *parent = 0);

    ~QApplicationMainWindow();

    /// Checks for a new version and shows release notes, if applicable.
    void checkNewVersionAndShowReleaseNotes();

    /// Assign a patient to the window.
    /// It will do the relevant actions depending on whether you already had a patient or are the first patient assigned
    /// @param patient The patient
    void setPatient(Patient *patient);

    /// Create a new window and open it. If we provide you with patient data, add them
    /// @param context Patient data that we want the new window to contain
    QApplicationMainWindow* setPatientInNewWindow(Patient *patient);

    /// It returns the pointer to the patient it has now
    Patient* getCurrentPatient();

    /// Returns the number of windows with different models we have open, the num. of QApplicationMainWindow
    unsigned int getCountQApplicationMainWindow();

    /// Returns all windows we have open
    static QList<QApplicationMainWindow*> getQApplicationMainWindows();

    /// Returns the current active window
    static QApplicationMainWindow* getActiveApplicationMainWindow();

    /// Method that returns the workspace to which you can add extensions
    ExtensionWorkspace* getExtensionWorkspace();

    /// Connects a patient's volumes to the method that reports volume loading
    void connectPatientVolumesToNotifier(Patient *patient);

protected:
    /// This event occurs when you close the window.
    /// It is the time when some tasks like performing the configuration are performed
    virtual void closeEvent(QCloseEvent *event);

    virtual void resizeEvent(QResizeEvent *event);

private:
    /// Create and initialize the actions of the application
    void createActions();

    /// Create menus.
    void createMenus();

    /// Create the menu to choose the language of the application
    void createLanguageMenu();

    /// Create an action to change the language by passing
    /// the language (eg Catalan) and the locale to be changed (eg ca_ES)
    QAction* createLanguageAction(const QString &language, const QString &locale);

    /// Removes all extensions contained in the workspace and removes the volumes of the current patient
    /// EVERYTHING: Patient, Study, Series, etc. data must be deleted. too.
    void killBill();

    /// Reads the initial configuration with which the program starts
    void readSettings();

    /// Enter the configuration with which the program will start next time.
    /// \ TODO: should it be called also when we open a new window?
    void writeSettings();

    /// Make extensions accessible through menus. It should be called when already
    /// we have valid input and volumes can be loaded
    void enableExtensions();

    /// Mark this application as a beta application, showing information to the user.
    void markAsBetaVersion();

    /// Updates the information displayed to the user in the menu as a beta version.
    void updateBetaVersionTextPosition();

    /// Send a request to download a study through your accession number
    void sendRequestRetrieveStudyWithAccessionNumberToLocalStarviewer(QString accessionNumber);


private slots:
    /// Generic method that finds out the loading progress of a volume and notifies it in some way in the interface
    /// such as a QProgressDialog or a label
    /// @param progress value of load progress
    void updateVolumeLoadProgressNotification(int progress);

    /// Displays the dialog explaining that it is a beta version.
    void showBetaVersionDialog();

    /// Create a new window and display it. Returns a reference to the created window.
    QApplicationMainWindow* openBlankWindow();

    ///Show the out form
    void about();

    /// Switch to the indicated language
    void switchToLanguage(QString locale);

    /// Launches the corresponding external application. (Where the number is
    /// the position on the list). If application launch has failed, shows an
    /// error message box.
    void launchExternalApplication(int i);

    ///Maximize as many screens as you can
    void maximizeMultipleScreens();

    /// Move the screen window
    void moveToDesktop(int screenIndex);

    /// Move the window to the previous screen
    void moveToPreviousDesktop();

    ///Move the window to the next screen
    void moveToNextDesktop();

    /// Displays the application configuration dialog.
    /// EVERYTHING Improve, at the moment it's pretty "sloppy"
    void showConfigurationDialog();

    void newCommandLineOptionsToRun();

    ///Opens the user manual
    void openUserGuide();

    /// Open the quick guide
    void openQuickStartGuide();

    /// Open the shortcuts scheme
    void openShortcutsGuide();

    ///Open release notes
    void openReleaseNotes();

    ///Displays the dialog that runs the diagnostic tests
    void showDiagnosisTestDialog();

    /// @brief External applications submenu with the defined external applications.
    ///
    /// When called multiple times, deletes the previous menu and regenerates a new one.
    ///
    /// If no external applications are defined, the menu is not created.
    void createExternalApplicationsMenu();

private:
    ///The mini-applications area
    ExtensionWorkspace *m_extensionWorkspace;

    ///Service and applet manager
    ExtensionHandler *m_extensionHandler;

    bool m_flag;
    /// Menus
    QMenu *m_fileMenu;
    QMenu *m_visualizationMenu;
    QMenu *m_windowMenu;
    QMenu *m_languageMenu;
    QMenu *m_externalApplicationsMenu;
    QMenu *m_helpMenu;
    QMenu *m_toolsMenu;
    QMenu *m_moveWindowToDesktopMenu;

    /// Accions
    QAction *m_localDatabaseAction;
    QAction *m_pacsAction;
    QAction *m_openDICOMDIRAction;

    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_openDirAction;
    QAction *m_closeAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_maximizeAction;
    QAction *m_moveToPreviousDesktopAction;
    QAction *m_moveToNextDesktopAction;
    QWidgetAction *m_moveToDesktopAction;

    QAction *m_logViewerAction;
    QAction *m_configurationAction;
    QAction *m_openUserGuideAction;
    QAction *m_openQuickStartGuideAction;
    QAction *m_openShortcutsGuideAction;
    QAction *m_openReleaseNotesAction;
    QAction *m_runDiagnosisTestsAction;

    QLabel *m_betaVersionMenuText;

    /// Signal mapping
    QSignalMapper *m_signalMapper;
    QList<QAction*> m_actionsList;

    /// To see the logs
    QLogViewer *m_logViewer;

    ///The patient who corresponds to this instance
    Patient *m_patient;

    bool m_isBetaVersion;

    ///Progress dialog to show the loading progress of the volumes
    QProgressDialog *m_progressDialog;

    ///Usability statistics
    StatsWatcher *m_statsWatcher;

    //------------------------------------------------------------
public:
    /// widows
    //QWidget *m_thumbnailWidget;

    //-----------------------------------------------------//
    //-------QDockWidget-----------------------------------||
    //QSplitter *m_splitter;///////////////////////////////||
    ImageThumbnailDockWidget *m_DockImageThumbnail;////////||
    //-----------------------------------------------------||
    //-----------------------------------------------------//

public:
    void clearImageThumbnailDockWidget();
    void addPatientsThumbnail(QList<Patient*> patientsList);
    ExtensionHandler *getExtensionHandler();
};

}; // fi namespace udg

#endif
