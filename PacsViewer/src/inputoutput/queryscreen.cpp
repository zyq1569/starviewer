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

#include "queryscreen.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QMovie>

#include "qpacslist.h"
#include "inputoutputsettings.h"
#include "pacsdevicemanager.h"
#include "logging.h"
#include "qcreatedicomdir.h"
#include "dicommask.h"
#include "qoperationstatescreen.h"
#include "localdatabasemanager.h"
#include "patient.h"
#include "starviewerapplication.h"
#include "statswatcher.h"
#include "pacsdevice.h"
#include "risrequestmanager.h"
#include "pacsmanager.h"
#include "retrievedicomfilesfrompacsjob.h"
#include "portinuse.h"

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#endif

namespace udg {

QueryScreen::QueryScreen(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowFlags(Qt::Widget);

    // Inicialitzem les variables necessàries
    initialize();
    // Connectem signals i slots
    createConnections();
    // Fa les comprovacions necessaries per poder executar la QueryScreen de forma correcte
    checkRequirements();

    readSettings();
    // Fem que per defecte mostri els estudis de la cache
    m_qInputOutputLocalDatabaseWidget->queryStudy(DicomMask());

    // Configuració per Starviewer Lite
#ifdef STARVIEWER_LITE
    m_showPACSNodesPushButton->hide();
    m_operationListPushButton->hide();
    m_createDICOMDIRPushButton->hide();
    m_advancedSearchPushButton->hide();
    // Tab de "PACS" fora
    m_tab->removeTab(1);
#else
    // L'engeguem després d'haver fet els connects, no es pot fer abans, perquè per exemple en el cas que tinguem un error
    // perquè el port ja està en us, si l'engeguem abans es faria signal indicant error de port en ús i no hi hauria hagut
    // temps d'haver fet el connect del signal d'error, per tant el signal s'hauria perdut sense poder avisar de l'error
    if (Settings().getValue(InputOutputSettings::ListenToRISRequests).toBool())
    {
        m_risRequestManager->listen();
    }
#endif

    m_statsWatcher = new StatsWatcher("QueryScreen", this);
    m_statsWatcher->addClicksCounter(m_operationListPushButton);
    m_statsWatcher->addClicksCounter(m_showPACSNodesPushButton);
    m_statsWatcher->addClicksCounter(m_createDICOMDIRPushButton);
    m_statsWatcher->addClicksCounter(m_advancedSearchPushButton);
    m_statsWatcher->addClicksCounter(m_clearPushButton);
    m_statsWatcher->addClicksCounter(m_createDICOMDIRPushButton);

    //HACK: Per traduir el botó Yes dels QMessageBox, aquesta no és la solució bona, s'hauria d'agafar les traduccions de sistema de Qt,
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Yes");
}

QueryScreen::~QueryScreen()
{
#ifndef STARVIEWER_LITE

    Settings settings;
    settings.setValue(InputOutputSettings::QueryScreenPACSListIsVisible, m_showPACSNodesPushButton->isChecked());

    if (m_pacsManager->isExecutingPACSJob())
    {
        // Si hi ha PacsJob executant-se demanem cancel·lar
        m_pacsManager->requestCancelAllPACSJobs();
    }

    delete m_risRequestManager;
    delete m_pacsManager;
#endif

    // Sinó fem un this.close i tenim la finestra queryscreen oberta al tancar l'starviewer, l'starviewer no finalitza
    // desapareixen les finestres, però el procés continua viu
    this->close();
}

void QueryScreen::initialize()
{
    m_qcreateDicomdir = new udg::QCreateDicomdir(this);
#ifndef STARVIEWER_LITE
    m_pacsManager = new PacsManager();
    // Posem com a pare el pare de la queryscreen, d'aquesta manera quan es tanqui el pare de la queryscreen
    // el QOperationStateScreen també es tancarà
    m_operationStateScreen = new udg::QOperationStateScreen(this);
    m_qInputOutputLocalDatabaseWidget->setPacsManager(m_pacsManager);
    m_qInputOutputPacsWidget->setPacsManager(m_pacsManager);
    m_operationStateScreen->setPacsManager(m_pacsManager);
    if (Settings().getValue(InputOutputSettings::ListenToRISRequests).toBool())
    {
        m_risRequestManager = new RISRequestManager(m_pacsManager);
    }
    else
    {
        m_risRequestManager = NULL;
    }
#endif
    // Indiquem quin és la intefície encara de crear dicomdir per a que es puguin comunicar
    m_qInputOutputLocalDatabaseWidget->setQCreateDicomdir(m_qcreateDicomdir);

    QMovie *operationAnimation = new QMovie(this);
    operationAnimation->setFileName(":/images/animations/loader.gif");
    m_operationAnimation->setMovie(operationAnimation);
    operationAnimation->start();

    m_qadvancedSearchWidget->hide();
    m_operationAnimation->hide();
    m_labelOperation->hide();
    refreshTab(LocalDataBaseTab);

    m_PACSJobsPendingToFinish = 0;
}

void QueryScreen::createConnections()
{
    // Connectem els butons
    connect(m_searchButton, SIGNAL(clicked()), SLOT(searchStudy()));
    connect(m_clearPushButton, SIGNAL(clicked()), SLOT(clearTexts()));
#ifndef STARVIEWER_LITE
    connect(m_operationListPushButton, SIGNAL(clicked()), SLOT(showOperationStateScreen()));
    connect(m_showPACSNodesPushButton, SIGNAL(toggled(bool)), SLOT(updatePACSNodesVisibility()));
    connect(m_pacsManager, SIGNAL(newPACSJobEnqueued(PACSJobPointer)), SLOT(newPACSJobEnqueued(PACSJobPointer)));
    if (m_risRequestManager != NULL)
    {
        // Potser que no tinguem activat escoltar peticions del RIS
        connect(m_risRequestManager, SIGNAL(viewStudyRetrievedFromRISRequest(QString)), SLOT(viewStudyFromDatabase(QString)));
        connect(m_risRequestManager, SIGNAL(loadStudyRetrievedFromRISRequest(QString)), SLOT(loadStudyFromDatabase(QString)));
    }
#endif
    connect(m_createDICOMDIRPushButton, SIGNAL(clicked()), m_qcreateDicomdir, SLOT(show()));

    // Es canvia de pestanya del TAB
    connect(m_tab, SIGNAL(currentChanged(int)), SLOT(refreshTab(int)));

    // Amaga o ensenya la cerca avançada
    connect(m_advancedSearchPushButton, SIGNAL(toggled(bool)), SLOT(setAdvancedSearchVisible(bool)));

    connect(m_qInputOutputDicomdirWidget, SIGNAL(clearSearchTexts()), SLOT(clearTexts()));
    connect(m_qInputOutputDicomdirWidget, SIGNAL(viewPatients(QList<Patient*>)), SLOT(viewPatients(QList<Patient*>)));
    connect(m_qInputOutputDicomdirWidget, SIGNAL(studyRetrieved(QString)), m_qInputOutputLocalDatabaseWidget, SLOT(addStudyToQStudyTreeWidget(QString)));

    connect(m_qInputOutputLocalDatabaseWidget, SIGNAL(viewPatients(QList<Patient*>, bool)), SLOT(viewPatients(QList<Patient*>, bool)));

    connect(m_qInputOutputPacsWidget, SIGNAL(viewRetrievedStudy(QString)), SLOT(viewStudyFromDatabase(QString)));
    connect(m_qInputOutputPacsWidget, SIGNAL(loadRetrievedStudy(QString)), SLOT(loadStudyFromDatabase(QString)));

    /// Ens informa quan hi hagut un canvi d'estat en alguna de les operacions
    connect(m_qInputOutputPacsWidget, SIGNAL(studyRetrieveFinished(QString)), m_qInputOutputLocalDatabaseWidget, SLOT(addStudyToQStudyTreeWidget(QString)));

    connect(m_qInputOutputPacsWidget, SIGNAL(studyRetrieveFinished(QString)), SLOT(studyRetrieveFinishedSlot(QString)));
    connect(m_qInputOutputPacsWidget, SIGNAL(studyRetrieveFailed(QString)), SLOT(studyRetrieveFailedSlot(QString)));
    connect(m_qInputOutputPacsWidget, SIGNAL(studyRetrieveStarted(QString)), SLOT(studyRetrieveStartedSlot(QString)));
    connect(m_qInputOutputPacsWidget, SIGNAL(studyRetrieveCancelled(QString)), SLOT(studyRetrieveCancelledSlot(QString)));
}

void QueryScreen::checkRequirements()
{
    // Comprova que la base de dades d'imatges estigui consistent, comprovant que no haguessin quedat estudis a mig descarregar l'última
    // vegada que es va tancar l'starviewer, i si és així esborra les imatges i deixa la base de dades en un estat consistent
    checkDatabaseImageIntegrity();
    // Comprova que el port pel qual es reben les descàrregues d'objectes dicom del PACS no estigui ja en ús
    checkIncomingConnectionsPacsPortNotInUse();
}

void QueryScreen::checkIncomingConnectionsPacsPortNotInUse()
{
    int localPort = Settings().getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toInt();

    if (PortInUse().isPortInUse(localPort))
    {
        QString message = tr("Port %1 for incoming connections from PACS is already in use by another application.").arg(localPort);
        message += "\n\n";
        message += tr("Unable to retrieve studies from PACS if the port is in use, please close the application that is using port %1 or change "
                      "the port for incoming connections from PACS in the configuration screen.").arg(localPort);

        QMessageBox::warning(this, ApplicationNameString, message);
    }
}

void QueryScreen::checkDatabaseImageIntegrity()
{
    LocalDatabaseManager localDatabaseManager;

    localDatabaseManager.deleteStudyBeingRetrieved();

    if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
    {
        ERROR_LOG("S'ha produït un error esborrant un estudi que no s'havia acabat de descarregar en la última execució");
    }
}

void QueryScreen::setAdvancedSearchVisible(bool visible)
{
    m_qadvancedSearchWidget->setVisible(visible);

    if (visible)
    {
        m_advancedSearchPushButton->setText(m_advancedSearchPushButton->text().replace(">>", "<<"));
    }
    else
    {
        m_qadvancedSearchWidget->clear();
        m_advancedSearchPushButton->setText(m_advancedSearchPushButton->text().replace("<<", ">>"));
    }
}

void QueryScreen::clearTexts()
{
    m_qbasicSearchWidget->clear();
    m_qadvancedSearchWidget->clear();
}

void QueryScreen::updateConfiguration(const QString &configuration)
{
    if (configuration == "Pacs/ListChanged")
    {
        m_PACSNodes->refresh();
    }
    else if (configuration == "Pacs/CacheCleared")
    {
        m_qInputOutputLocalDatabaseWidget->clear();
    }
}

void QueryScreen::bringToFront()
{
    this->show();
    if (this->isMinimized())
    {
        this->setWindowState(this->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    }
    else
    {
        this->raise();
        this->activateWindow();
    }
}

void QueryScreen::showPACSTab()
{
    m_tab->setCurrentIndex(PACSQueryTab);
    bringToFront();
}

void QueryScreen::showLocalExams()
{
    m_tab->setCurrentIndex(LocalDataBaseTab);
    m_qbasicSearchWidget->clear();
    m_qbasicSearchWidget->setDefaultDate(QBasicSearchWidget::AnyDate);
    m_qadvancedSearchWidget->clear();
    m_qInputOutputLocalDatabaseWidget->queryStudy(DicomMask());
    bringToFront();
}

void QueryScreen::searchStudy()
{
    switch (m_tab->currentIndex())
    {
    case LocalDataBaseTab:
        m_qInputOutputLocalDatabaseWidget->queryStudy(buildDicomMask());
        break;

    case PACSQueryTab:
        m_qInputOutputPacsWidget->queryStudy(buildDicomMask(), m_PACSNodes->getSelectedPacs());
        break;

    case DICOMDIRTab:
        m_qInputOutputDicomdirWidget->queryStudy(buildDicomMask());
        break;
    }
}

void QueryScreen::viewStudyFromDatabase(QString studyInstanceUID)
{
    //We indicate that we want to see a study that is stored in the database
    m_qInputOutputLocalDatabaseWidget->view(studyInstanceUID);
}

void QueryScreen::loadStudyFromDatabase(QString studyInstanceUID)
{
    //We indicate that we want to see a study that is stored in the database
    m_qInputOutputLocalDatabaseWidget->view(studyInstanceUID, true);
}

void QueryScreen::sendDicomObjectsToPacs(PacsDevice pacsDevice, QList<Image*> images)
{
    m_qInputOutputLocalDatabaseWidget->sendDICOMFilesToPACS(pacsDevice, images);
}

void QueryScreen::refreshTab(int index)
{
    switch (index)
    {
    case LocalDataBaseTab:
        m_qbasicSearchWidget->setEnabledSeriesModality(true);
        m_qadvancedSearchWidget->setEnabled(false);
        break;

    case PACSQueryTab:
        m_qbasicSearchWidget->setEnabledSeriesModality(true);
        m_qadvancedSearchWidget->setEnabled(true);
        break;

    case DICOMDIRTab:
        m_qbasicSearchWidget->setEnabledSeriesModality(false);
        m_qadvancedSearchWidget->setEnabled(false);
        break;
    }
    updatePACSNodesVisibility();
}

void QueryScreen::updatePACSNodesVisibility()
{
    if (m_tab->currentIndex() == PACSQueryTab)
    {
        m_showPACSNodesPushButton->show();
        m_PACSNodes->setVisible(m_showPACSNodesPushButton->isChecked());
    }
    else
    {
        m_showPACSNodesPushButton->hide();
        m_PACSNodes->hide();
    }
}

void QueryScreen::viewPatients(QList<Patient*> listPatientsToView, bool loadOnly)
{
    // If we do a "view" we will hide the QueryScreen windows because
    // the application and extensions are visible, otherwise we will not hide anything
    if (!loadOnly)
    {
        // It is hidden to be able to visualize the series
        this->close();
    }

    emit selectedPatients(listPatientsToView, loadOnly);
    emit setPatientsThumbnail(listPatientsToView, loadOnly);
}

#ifndef STARVIEWER_LITE
void QueryScreen::showOperationStateScreen()
{
    if (!m_operationStateScreen->isVisible())
    {
        m_operationStateScreen->setVisible(true);
    }
    else
    {
        m_operationStateScreen->raise();
        m_operationStateScreen->activateWindow();
    }
}
#endif

void QueryScreen::openDicomdir()
{
    if (m_qInputOutputDicomdirWidget->openDicomdir())
    {
        this->bringToFront();
        // Mostra el tab del dicomdir
        m_tab->setCurrentIndex(DICOMDIRTab);
    }
}

DicomMask QueryScreen::buildDicomMask()
{
    return m_qbasicSearchWidget->buildDicomMask() + m_qadvancedSearchWidget->buildDicomMask();
}

void QueryScreen::closeEvent(QCloseEvent *event)
{
    // We save the settings
    writeSettings();

    /// TODO: From the ExtensionHander descriptor when no more is left
    /// QApplicationMaingWindow open invokes the QueryScreen close method because
    /// close all windows dependent on the QueryScreen, this causes
    /// that the lastWindowClosed signal is thrown which we respond by invoking
    /// the method quit from main.cpp. This is why when the close () method is invoked
    /// of the QueryScreen it is necessary to close all open windows
    /// from the QueryScreen for Starviewer to close in case there is no QApplicationMainWindow viewer.
#ifndef STARVIEWER_LITE
    // We close the QOperationStateScreen when we close the QueryScreen
    m_operationStateScreen->close();
#endif
    m_qcreateDicomdir->close();

    event->accept();

    emit closed();
}

void QueryScreen::readSettings()
{
    Settings settings;
    settings.restoreGeometry(InputOutputSettings::QueryScreenGeometry, this);
    /// This key replaces the obsolete "queryScreenWindowPositionX",
    /// "queryScreenWindowPositionY", "queryScreenWindowWidth" and "queryScreenWindowHeight"
    /// that had the keys / interface / queryscreen / + windowPositionX,
    /// windowPositionY, windowWidth and windowHeigth respectively
    /// TODO clean up these old keys with data migration

#ifndef STARVIEWER_LITE
    m_showPACSNodesPushButton->setChecked(settings.getValue(InputOutputSettings::QueryScreenPACSListIsVisible).toBool());
#endif
}

void QueryScreen::writeSettings()
{
    /// We only save the settings when the interface has been visible,
    /// since there are settings like QSplitter that if the first time
    /// that we run the starviewer we don't open the QueryScreen returns a
    /// incorrect values so what we do is check the QueryScreen
    /// has been visible to save the settings
    if (this->isVisible())
    {
        Settings settings;
        settings.saveGeometry(InputOutputSettings::QueryScreenGeometry, this);
    }
}

void QueryScreen::retrieveStudy(QInputOutputPacsWidget::ActionsAfterRetrieve actionAfterRetrieve,
                                const PacsDevice &pacsDevice, Study *study)
{
    // QueryScreen receives a signal every time any studies in the download process change status,
    // in principle you only need to retransmit these signals out when it is a signal that affects a study
    // requested through this public method, so we keep this list that indicates the studies
    // pending download requested from this method
    m_studyRequestedToRetrieveFromPublicMethod.append(study->getInstanceUID());

    m_qInputOutputPacsWidget->retrieve(pacsDevice, actionAfterRetrieve, study);
}

void QueryScreen::studyRetrieveFailedSlot(QString studyInstanceUID)
{
    if (m_studyRequestedToRetrieveFromPublicMethod.contains(studyInstanceUID))
    {
        //It is a study of those who have asked us from the public method
        m_studyRequestedToRetrieveFromPublicMethod.removeOne(studyInstanceUID);

        emit studyRetrieveFailed(studyInstanceUID);
    }
}

void QueryScreen::studyRetrieveFinishedSlot(QString studyInstanceUID)
{
    if (m_studyRequestedToRetrieveFromPublicMethod.contains(studyInstanceUID))
    {
        // It is a study of those who have asked us from the public method
        m_studyRequestedToRetrieveFromPublicMethod.removeOne(studyInstanceUID);

        emit studyRetrieveFinished(studyInstanceUID);
    }
}

void QueryScreen::studyRetrieveStartedSlot(QString studyInstanceUID)
{
    if (m_studyRequestedToRetrieveFromPublicMethod.contains(studyInstanceUID))
    {
        //It is a study of those who have asked us from the public method
        emit studyRetrieveStarted(studyInstanceUID);
    }
}

void QueryScreen::studyRetrieveCancelledSlot(QString studyInstanceUID)
{
    if (m_studyRequestedToRetrieveFromPublicMethod.contains(studyInstanceUID))
    {
        // It is a study of those who have asked us from the public method
        m_studyRequestedToRetrieveFromPublicMethod.removeOne(studyInstanceUID);

        emit studyRetrieveCancelled(studyInstanceUID);
    }
}
void QueryScreen::newPACSJobEnqueued(PACSJobPointer pacsJob)
{
    if (pacsJob->getPACSJobType() == PACSJob::SendDICOMFilesToPACSJobType ||
            pacsJob->getPACSJobType() == PACSJob::RetrieveDICOMFilesFromPACSJobType)
    {
        m_operationAnimation->show();
        m_labelOperation->show();
        connect(pacsJob.data(), SIGNAL(PACSJobFinished(PACSJobPointer)), SLOT(pacsJobFinishedOrCancelled(PACSJobPointer)));
        connect(pacsJob.data(), SIGNAL(PACSJobCancelled(PACSJobPointer)), SLOT(pacsJobFinishedOrCancelled(PACSJobPointer)));

        // We indicate that we have a PACSJob more pending completion
        m_PACSJobsPendingToFinish++;
    }
}

void QueryScreen::pacsJobFinishedOrCancelled(PACSJobPointer)
{
    /// We can't use isExecutingPACSJob to check for pending jobs, because sometimes we
    /// we found that even though we don't have any jobs pending execution, the method responds that there are some jobs running.
    /// This happens sometimes when the PACSJobFinished signal is addressed
    /// of a job immediately and asks the isExecutingPACSJob method
    /// if there are jobs running, it would seem that despite having finished
    /// the last pending ThreadWeaver job is just done
    /// some action and therefore indicates that there are jobs running
    m_PACSJobsPendingToFinish--;

    if (m_PACSJobsPendingToFinish == 0)
    {
        m_operationAnimation->hide();
        m_labelOperation->hide();
    }
}

};
