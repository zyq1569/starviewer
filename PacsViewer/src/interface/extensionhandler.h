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

#ifndef UDGEXTENSIONHANDLER_H
#define UDGEXTENSIONHANDLER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMutex>


class QLocalServer;
class HttpClient;
class QLocalSocket;


#include "extensioncontext.h"
#include "appimportfile.h"


#define ImageAppName  "StarViewer"


namespace udg
{
// Fordward Declarations
class QApplicationMainWindow;

/**
   Manager of mini-applications and services of the main application
*/
class ExtensionHandler : public QObject
{
    Q_OBJECT
public:
    ExtensionHandler(QApplicationMainWindow *mainApp, QObject *parent = 0);

    ~ExtensionHandler();

public slots:
    /// Receive the request for a service / mini-application and do what is necessary
    void request(int who);
    bool request(const QString &who);

    /// It will open the default extension. If there is no valid patient data, it will do nothing.
    void openDefaultExtension();

    /// Assigns the context of the extension
    /// @param context context
    void setContext(const ExtensionContext &context);

    /// We get the context of the extension
    /// @return The context of the extension can be modified
    ExtensionContext& getContext();

    ///add----------/////////////////////////////////////////////////////////
    void patientsInput(QList<Patient*> patientsList, bool loadOnly = false);

    ///20201205
    void setPatientsThumbnail(QList<Patient*> patientsList, bool loadOnly = false);
    ///20201207
    void newClientConnection();
    void fromClientNotify();
    void readFromServer(int fd);
    void socketConnect();
    void socketDisconnect();
    //void socketError(QLocalSocket::LocalSocketError socketError);
    //void socketStateChanged(QLocalSocket::LocalSocketState socketState);
    void socketError(int socketError);
    void socketStateChanged(int socketState);
private slots:
    ///20201206 add
    /// Processes a set of httpServer input files and processes them to decide what to do with them, such as
    /// create new patient, open windows, add data to current patient, etc
    /// @param inputFiles Files to process, which may or may not be supported by the application
    void httpServerInput(const QStringList &inputFiles);
    void httpServerDownAllDcm();

    /// Processes a set of input files and processes them to decide what to do with them, such as
    /// create new patient, open windows, add data to current patient, etc
    /// @param inputFiles Files to process, which may or may not be supported by the application
    void processInput(const QStringList &inputFiles);

    /// Given a list of inpatients, he is in charge of setting up
    /// these and assign them the appropriate window, deciding whether to open new windows
    /// and / or merge or mash the current patient in this window.
    /// The list is assumed to have no separate equal patients, i.e.
    /// it is assumed that each entry corresponds to a different patient, with one or more studies
    /// We add a second parameter that tells us if patients just need to be loaded or "viewed".
    void processInput(QList<Patient*> patientsList, bool loadOnly = false);

    ///It will be called when the QueryScreen closes
    void queryScreenIsClosed();

private:
    ///Creates signal and slot connections
    void createConnections();

    /// Add a patient to a mainwindow considering whether to merge or
    /// not and whether to replace the currently loaded current patient
    /// We add a second parameter that tells us if patients just need to be loaded or "viewed".
    /// Returns the mainwindow to which the patient was added.
    QApplicationMainWindow* addPatientToWindow(Patient *patient, bool canReplaceActualPatient, bool loadOnly = false);

    /// Process the given patient so that it can be used for extensions
    // Creates the volumes in the repository and assigns the default string
    void generatePatientVolumes(Patient *patient, const QString &defaultSeriesUID);

    /// Scans the list and returns a list where the patients which are the same are merged.
    /// Patients are only considered to be the same if Patient::compareTo() returns SamePatients value.
    QList<Patient*> mergePatients(const QList<Patient*> &patientList);

    ///----add creat Extension object 20200925-------

private:
    bool createExtension(const QString &who, QString tableText = "");
    //-----------------------------------------------
    ///----add  20201207-------

private:
    QLocalServer *m_localserver;
    QLocalSocket *m_clientSocket;
    HttpClient *m_httpclient;
    //-----------------------------------------------

private:
    /// Pointer to the main application
    QApplicationMainWindow *m_mainApp;

    /// Import file system models into the volume repository
    AppImportFile m_importFileApp;

    ///Context of the extension
    ExtensionContext m_extensionContext;

    /// Whether or not to close the QueryScreen for this window
    bool m_haveToCloseQueryScreen;

    ///Mutex to avoid concurrent access to the patient comparer singleton.
    QMutex m_patientComparerMutex;

 public:
    void closeCurrentPatient();
	void processCommandInput(const QStringList &inputFiles);
};

};  // end namespace udg

#endif
