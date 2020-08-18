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


#ifndef UDGQINPUTOUTPUTPACSWIDGET_H
#define UDGQINPUTOUTPUTPACSWIDGET_H

#include "ui_qinputoutputpacswidgetbase.h"

#include <QMenu>
#include <QHash>

#include "pacsdevice.h"
#include "pacsjob.h"

// Fordward declarations
class QString;

namespace udg {

// Fordward declarations
class DicomMask;
class Patient;
class StatsWatcher;
class Status;
class Study;
class QOperationStateScreen;
class PacsManager;
class QueryPacsJob;

/**
    Widget en el que controla les operacions d'entrada/sortida del PACS
  */
class QInputOutputPacsWidget : public QWidget, private Ui::QInputOutputPacsWidgetBase {
Q_OBJECT
public:
    /// Definició d'accions que podem fer d'haver descarregat estudis
    enum ActionsAfterRetrieve { None = 0, View = 1, Load = 2 };

    /// Constructor de la classe
    QInputOutputPacsWidget(QWidget *parent = 0);
    ~QInputOutputPacsWidget();

    /// Especifiquem l'instància de PacsManager utilitza per les operacions amb el PACS
    void setPacsManager(PacsManager *pacsManager);

    /// Consulta els estudis al dicomdir obert que compleixin la màscara de cerca
    void queryStudy(DicomMask queryMask, QList<PacsDevice> pacsToQuery);

    /// Neteja els resultats de la última cerca
    void clear();

    /// Descarrega una estudi del pacs
    void retrieve(const PacsDevice &pacsDevice, ActionsAfterRetrieve actionAfterRetrieve, Study *studyToRetrieve, const QString &seriesInstanceUIDToRetrieve = "", const QString &sopInstanceUIDToRetrieve = "");

signals:
    /// Signal que s'emet per indicar que es netegin els camps de cerca
    void clearSearchTexts();

    /// Signal que s'emet per indicar que s'ha demanat visualitzar un estudi
    void viewRetrievedStudy(QString studyInstanceUID);

    /// Signal que s'emet per indicar que un cop descarregat l'estudi s'ha de carregar únicament
    void loadRetrievedStudy(QString studyInstanceUID);

    // TODO: Els mètodes studyRetrieved han de desapareixer cada tool ha de començar a utilitzar la PACSManager
    /// Signal que s'emet per indicar que un estudi s'ha començat a descarregar
    void studyRetrieveStarted(QString studyInstanceUID);

    /// Signal que indica que s'ha produït un error descarregant l'estudi indicat
    void studyRetrieveFailed(QString studyInstanceUID);

    /// Signal que s'emet per indica que un estudi ha estat descarregat
    void studyRetrieveFinished(QString studyInstanceUID);

    /// Signal que s'emet per indicar que la descàrrgea d'un estudi s'ha cencel·lat
    void studyRetrieveCancelled(QString studyInstanceUID);

private:
    /// Crea les connexions entre signals i slots
    void createConnections();

    /// Genera el menú contextual que apareix quan clickem amb el botó dret a sobre d'un item del StudyTreeWidget
    void createContextMenuQStudyTreeWidget();

    /// Comprova que els paràmetres per la cerca siguin correctes, que no es tractir d'un consulta pesada i que ens hagin seleccionat
    /// algun PACS per consultar
    bool AreValidQueryParameters(DicomMask *maskToQuery, QList<PacsDevice> pacsToQuery);

    /// Construeix la màscara de cerca per cercar les sèries d'un estudi
    DicomMask buildSeriesDicomMask(QString studyInstanceUID);

    /// Construeix la màscara de cerca per cercar les imatges d'una sèrie
    DicomMask buildImageDicomMask(QString studyInstanceUID, QString seriesInstanceUID);

    /// Mostra per pantalla els resultats de la consulta al PACS d'un Job
    void showQueryPACSJobResults(PACSJobPointer queryPACSJob);

    /// Mostrar un QMessageBox indicant que s'ha produït un error consultant a un PACS
    void showErrorQueringPACS(PACSJobPointer queryPACSJob);

    /// Ens encua el QueryPACSJob al PACSManager i ens connecta amb els seus signals per poder processar els resultats. També afegeix el Job en una taula
    /// de hash on es guarden tots els QueryPACSJobs demanats per aquesta classe que estant pendents d'executar-se o s'estan executant
    void enqueueQueryPACSJobToPACSManagerAndConnectSignals(PACSJobPointer queryPacsJob);

private slots:
    /// Mostra les sèries d'un estudi, les consulta al dicomdir i les mostra al tree widget
    void requestedSeriesOfStudy(Study *study);

    /// Mostra les imatges d'un estudi, les consulta al dicomdir i les mostra al tree widget
    void requestedImagesOfSeries(Series *series);

    /// Importa cap a la base de dades local els estudis seleccionats
    void retrieveSelectedItemsFromQStudyTreeWidget();

    /// Importa cap a la base de dades local els estudis seleccionats indicant
    /// que s'han de visualitzar immediatament un cop descarregats
    void retrieveAndViewSelectedItemsFromQStudyTreeWidget();

    /// Cancel·la els QueryPACSJob que s'han llançat des d'aquesta classe i que encara no han finalitzat (cancel·la els que s'estan executant i els pendents
    /// d'executar).
    /// La idea és donar mètode que es pugui invocar per cancel·lar les consultes actuals, per exemple s'ha llançat una consulta a 3 PACS
    /// per cercar tots els estudis amb ID 1, si l'usuari canvia la consulta i diu que ara vol tots els estudis amb ID 2, no cal seguir endavant amb
    /// l'anterior consulta doncs aquest mètode està pensat per aquests casos per poder cancel·lar les consultes actuals llançades des d'aquesta classe
    /// que s'estan realitzant.
    void cancelCurrentQueriesToPACS();

    /// Fa signal de studyRetrieveStarted, Important!!! aquest mètode una vegada cada Tool utiltizi la PacsManager ha de desapareixer
    void retrieveDICOMFilesFromPACSJobStarted(PACSJobPointer pacsJob);

    /// Slot que s'activa quan finalitza un job de descàrrega d'imatges
    void retrieveDICOMFilesFromPACSJobFinished(PACSJobPointer pacsJob);

    /// Slot que s'activa quan es cancel·la un job de descàrrega d'imatges
    void retrieveDICOMFilesFromPACSJobCancelled(PACSJobPointer pacsJob);
    
    /// Slot que s'activa quan finalitza un job de consulta al PACS
    void queryPACSJobFinished(PACSJobPointer pacsJob);

    /// Slot que s'activa quan un job de consulta al PACS és cancel·lat
    void queryPACSJobCancelled(PACSJobPointer pacsJob);

private:
    QMenu m_contextMenuQStudyTreeWidget;
    PacsManager *m_pacsManager;
    /// Per cada job de descàrrega guardem quina acció hem de fer quan ha acabat la descàrrega
    QHash<int, ActionsAfterRetrieve> m_actionsWhenRetrieveJobFinished;

    /// Hash que ens guarda tots els QueryPACSJob pendent d'executar o que s'estan executant llançats des d'aquesta classe
    QHash<int, PACSJobPointer> m_queryPACSJobPendingExecuteOrExecuting;

    StatsWatcher *m_statsWatcher;

    /// Amaga/mostra que hi ha una query en progress i habilitat/deshabilitat el botó de cancel·lar la query actual
    void setQueryInProgress(bool queryInProgress);

    /// Descarrega els estudis seleccionats dels QStudyTreeWidget, i una vegada descarregats por a terme l'acció passada per paràmetre
    void retrieveSelectedItemsFromQStudyTreeWidget(ActionsAfterRetrieve _actionsAfterRetrieve);

};

};// end namespace udg

#endif
