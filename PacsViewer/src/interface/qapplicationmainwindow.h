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

namespace udg {
// Forward declarations
class ExtensionWorkspace;
class ExtensionHandler;
class ExtensionContext;
class QLogViewer;
class Patient;
class StatsWatcher;
class ApplicationVersionChecker;
class ExternalApplication;

class QApplicationMainWindow : public QMainWindow {
    Q_OBJECT
public:
    /// The builder of the Mainwindow. Here the whole skeleton of the interface is created: Menus, Toolbars, toolbox, working area, etc...
    QApplicationMainWindow(QWidget *parent = 0);

    ~QApplicationMainWindow();

    /// Assigna un pacient a la finestra.
    /// Farà les accions pertinents segons si ja es tenia un pacient o bé és el primer pacient que s'assigna
    /// @param patient El pacient
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

    ///Method that returns the workspace to which you can add extensions
    ExtensionWorkspace* getExtensionWorkspace();

    /// Connects a patient’s volumes to the method that reports volume loading
    void connectPatientVolumesToNotifier(Patient *patient);

protected:
    ///This event occurs when you close the window. It is the time when some tasks like performing the configuration are performed
    virtual void closeEvent(QCloseEvent *event);

    virtual void resizeEvent(QResizeEvent *event);

    virtual void showEvent(QShowEvent *event);

private:
    /// Creates and initializes application actions
    void createActions();

    /// Create menus.
    void createMenus();

    /// Create the menu to choose the language of the application
    void createLanguageMenu();

    /// Create an action to change the language by passing the language (eg Catalan) and the locale to be changed (eg ca_ES)
    QAction* createLanguageAction(const QString &language, const QString &locale);

    /// Removes all extensions contained in the workspace and removes the volumes of the current patient
    /// EVERYTHING: Patient, Study, Series, etc. data must be deleted. too.
    void killBill();

    /// Reads the initial configuration with which the program starts
    void readSettings();

    ///Enter the configuration with which the program will start next time. \ TODO: should it also be called when we open a new window?
    void writeSettings();

    /// Make extensions accessible through menus. It should be called when already
    /// we have valid input and volumes can be loaded
    void enableExtensions();

    /// Marca aquesta aplicació com a aplicació beta, mostrant informació a l'usuari.
    void markAsBetaVersion();

    ///Updates the information displayed to the user in the menu as a beta version.
    void updateBetaVersionTextPosition();

    /// Envia una petició per descarregar un estudi a través del seu accession number
    void sendRequestRetrieveStudyWithAccessionNumberToLocalStarviewer(QString accessionNumber);

    /// Calculate the font size of the tools text if CoreSettings :: AutoToolTextSize is true.
    /// Otherwise the value that was already in CoreSettings :: DefalutToolTextSize will be taken.
    void computeDefaultToolTextSize();

private slots:
    /// Generic method that finds out the loading progress of a volume and notifies it in some way in the interface
    /// such as a QProgressDialog or a label
    /// @param progress value of load progress
    void updateVolumeLoadProgressNotification(int progress);

    /// Displays the dialog explaining that it is a beta version.
    void showBetaVersionDialog();

    ///Create a new window and display it. Returns a reference to the created window.
    QApplicationMainWindow* openBlankWindow();

    /// Show the out form
    void about();

    ///Switch to the indicated language
    void switchToLanguage(QString locale);

    /// Launches the corresponding external application. (Where the number is
    /// the position on the list). If application launch has failed, shows an
    /// error message box.
    void launchExternalApplication(int i);

    /// Maximize as many screens as you can
    void maximizeMultipleScreens();

    /// Moure la finestra de pantalla
    void moveToDesktop(int screenIndex);
    /// Moure la finestra a la pantalla anterior
    void moveToPreviousDesktop();
    /// Moure la finestra a la següent pantalla
    void moveToNextDesktop();

    /// Mostra el diàleg de configuració de l'aplicació.
    /// TODO Millorar, de moment és bastant "xapussa"
    void showConfigurationDialog();

    void newCommandLineOptionsToRun();

    /// Obre el manual d'usuari
    void openUserGuide();

    /// Obre la guia rapida
    void openQuickStartGuide();

    /// Obre l'esquema de shortcuts
    void openShortcutsGuide();

    /// Obre les release notes
    void openReleaseNotes();

    /// Mostra el diàleg que executa els diagnosis test
    void showDiagnosisTestDialog();

    /// @brief External applications submenu with the defined external applications.
    ///
    /// When called multiple times, deletes the previous menu and regenerates a new one.
    ///
    /// If no external applications are defined, the menu is not created.
    void createExternalApplicationsMenu();

private:
    /// L'àrea de mini-aplicacions
    ExtensionWorkspace *m_extensionWorkspace;

    /// Gestor de serveis i miniaplicacions
    ExtensionHandler *m_extensionHandler;

    /// Menús
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

    /// Mapeig de signals
    QSignalMapper *m_signalMapper;
    QList<QAction*> m_actionsList;

    /// Per veure els logs
    QLogViewer *m_logViewer;

    /// El pacient que li correspon a aquesta instància
    Patient *m_patient;

    bool m_isBetaVersion;

    /// Progress dialog per mostrar el progrés de càrrega dels volums
    QProgressDialog *m_progressDialog;

    /// Estadístiques d'usabilitat
    StatsWatcher *m_statsWatcher;

    /// Les Release Notes o les notes de la nova versió
    ApplicationVersionChecker *m_applicationVersionChecker;
};

}; // fi namespace udg

#endif
