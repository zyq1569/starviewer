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

#ifndef UDGQ2DVIEWEREXTENSION_H
#define UDGQ2DVIEWEREXTENSION_H

#include "ui_q2dviewerextensionbase.h"
#include <QWidget>

// FWD declarations
class QAction;

namespace udg {

// FWD declarations
class Volume;
class ToolManager;
class ToolConfiguration;
class HangingProtocol;
class TransferFunctionModel;
// Estructura pacient
class Patient;
class Study;
// Visualitzador
class Q2DViewerWidget;
class Q2DViewer;
// Menus
class QHangingProtocolsWidget;
class TableMenu;
class QDICOMDumpBrowser;
class StatsWatcher;
class LayoutManager;
class ExtensionHandler;

#ifndef STARVIEWER_LITE
class QRelatedStudiesWidget;
class RelatedStudiesManager;
class AutomaticSynchronizationManager;
class SyncActionManager;
class ViewersLayoutToSyncActionManagerAdapter;
class SyncActionsConfigurationHandler;
#endif

/**
   Extension that will run by default when opening a model
  */
class Q2DViewerExtension : public QWidget, private Ui::Q2DViewerExtensionBase {
    Q_OBJECT
public:
    Q2DViewerExtension(QWidget *parent = 0);
    ~Q2DViewerExtension();

#ifdef STARVIEWER_LITE
    /// Assigns the main volume to it
    void setInput(Volume *input);
#endif

    /// Method for assigning a patient
    void setPatient(Patient *patient);

    ///Method for obtaining the patient
    Patient* getPatient() const;

    /// Sets the study from the current patient with the given studyUID as the current patient.
    void setCurrentStudy(const QString &studyUID);

private:
    ///Creates connections between signals and slots
    void createConnections();

    ///Read / Write the application settings
    void readSettings();
    void writeSettings();

    ///Initialize the tools that the extension will have
    void initializeTools();

    /// Configure the tools to be activated by default according to the given list of modes
    void setupDefaultToolsForModalities(const QStringList &modalities);
    
    ///Depending on the data of the loaded patient will choose which
    /// is the most appropriate left button tool to be activated by default
    void setupDefaultLeftButtonTool();

    /// Sets up layout manager and its connections properly
    void setupLayoutManager();

    /// Shows the given widget below the button, as if it was the button's menu
    void showWidgetBelowButton(QWidget *widget, QAbstractButton *button);

    /// Sets the given transfer function model to the transfer function combo
    ///  box and selects the index of the transfer function applied in the current viewer.
    void updateTransferFunctionComboBox(TransferFunctionModel *transferFunctionModel);

private slots:
    /// Enable or disable the DICOM info info Dump button in the view it is in
    /// the currently selected viewer. If no viewfinder is active, it will disable the button.
    void updateDICOMInformationButton();

    /// Enables or disables the exporter tool button according to the status of the selected viewer.
    void updateExporterToolButton();

    ///Method that treats the new viewer added
    void activateNewViewer(Q2DViewerWidget *newViewerWidget);

    /// Method that handles the selected viewer change
    void changeSelectedViewer(Q2DViewerWidget *viewerWidget);

    /// If enable is true, enable maximum thickness in all viewers where some thickslab mode is active
    void enableMaximumThicknessMode(bool enable);

    /// Shows a widget with the available Hanging Protocols for the current Patient below its button
    void showAvailableHangingProtocolsMenu();

    /// Shows a widget with an interactive grid table to select the desired viewers layout below its corresponding button
    void showViewersLayoutGridTable();

    /// Show or hide the overlays in each viewer
    void showImageOverlays(bool show);

    ///Shows or hides the display shutters of each viewer
    void showDisplayShutters(bool show);

    ///Shows or hides the textual information of each viewer
    void showViewersTextualInformation(bool show);
    
    ///Display the Dicom dump screen, with
    ///  information from the image currently displayed
    void showDicomDumpCurrentDisplayedImage();

#ifndef STARVIEWER_LITE
    /// Display the dialog to export the selected viewer series.
    void showScreenshotsExporterDialog();
#endif

    /// TODO Method to troubleshoot for synchronization
    /// is not yet adapted to the rest of the tools
    /// Disable the sync tool.
    void disableSynchronization();

#ifndef STARVIEWER_LITE
    ///Initialize automatic synchronization by reference to the viewer we have active (selected)
    void enableAutomaticSynchronizationToViewer(bool enable);

    /// Methods used to modify the related studies button icon to
    /// see if studies are being downloaded without having to open the widget.
    void changeToRelatedStudiesDownloadingIcon();
    void changeToRelatedStudiesDefaultIcon();

    ///Displays the widget to download studies related to the current study
    void showRelatedStudiesWidget();
    
    /// Enable manual synchronization in all viewers
    void activateManualSynchronizationInAllViewers();

    //Disables manual synchronization of all viewers
    void deactivateManualSynchronizationInAllViewers();

    //Treats the event when manual synchronization has been selected / disabled in any of the viewers
    void manualSynchronizationActivated(bool enable);

    ///Sets up if propagation must be enabled or not depending on the current patient modalities
    void setupPropagation();
#endif

    /// Apply a regular grid to the layout, and remove the label if any related studies are in download
    void setGrid(int rows, int columns);

    /// Sets the transfer function model of the current viewer to
    /// the transfer function combo box and selects the index of the transfer function applied in the
    /// current viewer.
    void updateTransferFunctionComboBoxWithCurrentViewerModel();

    /// Updates external application parameters from the currently active view.
    void updateExternalApplicationsWithCurrentView(Volume* volume);

    /// Sets the transfer function at the given index in the current model to the current viewer.
    void setTransferFunctionToCurrentViewer(int transferFunctionIndex);

    /// When a viewer receives a double click, this method checks which tools are active and
    /// if none of them processes single or double clicks then maximizes or demaximizes the viewer.
    void handleViewerDoubleClick(Q2DViewerWidget *viewerWidget);

    /// Set the working studies (current and prior studies).
    void setWorkingStudies(const QString &currentStudyUID, const QString &priorStudyUID);

    /// Sets a 2x1 CT+fused fusion layout with the given volumes and anatomical plane.
    void setFusionLayout2x1First(const QList<Volume*> &volumes, const AnatomicalPlane &anatomicalPlane);
    /// Sets a 2x1 fused+PT/NM fusion layout with the given volumes and anatomical plane.
    void setFusionLayout2x1Second(const QList<Volume*> &volumes, const AnatomicalPlane &anatomicalPlane);
    /// Sets a 3x1 fusion layout with the given volumes and anatomical plane.
    void setFusionLayout3x1(const QList<Volume*> &volumes, const AnatomicalPlane &anatomicalPlane);
    /// Sets a 2x3 CT+fused fusion layout with the given volumes.
    void setFusionLayout2x3First(const QList<Volume*> &volumes);
    /// Sets a 2x3 fused+PT/NM fusion layout with the given volumes.
    void setFusionLayout2x3Second(const QList<Volume*> &volumes);
    /// Sets a 3x3 fusion layout with the given volumes.
    void setFusionLayout3x3(const QList<Volume*> &volumes);
    /// Sets a MPR R fusion layout with the given volumes.
    void setFusionLayoutMprRight(const QList<Volume*> &volumes);

private:
    /// Accions
    QAction *m_singleShotAction;
    QAction *m_multipleShotAction;
    QAction *m_screenShotTriggerAction;
    QAction *m_showOverlaysAction;
    QAction *m_showViewersTextualInformationAction;
    QAction *m_showDisplayShuttersAction;
    QAction *m_synchronizeAllAction;
    QAction *m_desynchronizeAllAction;
    QAction *m_propagationAction;

#ifndef STARVIEWER_LITE
    QAction *m_sagitalViewAction;
    QAction *m_coronalViewAction;
#endif

    /// The dialog to choose a user-adjusted window level
    QCustomWindowLevelDialog *m_customWindowLevelDialog;

    /// Opens the dicomdump window for the image we have currently selected
    QDICOMDumpBrowser *m_dicomDumpCurrentDisplayedImage;

    ///Patient we are treating
    Patient *m_patient;

    /// Menu to select hanging protocols
    QHangingProtocolsWidget *m_hangingProtocolsMenu;

    /// Widget to choose a user-defined viewer distribution
    TableMenu *m_viewersLayoutGrid;

    /// ToolManager to configure the environment of ToolConfiguration * configuration = getConfiguration ();
    ToolManager *m_toolManager;

    /// Profile to display the interface
    QString m_profile;

    /// Last viewer selected
    Q2DViewerWidget *m_lastSelectedViewer;

    /// To make usability statistics
    StatsWatcher *m_statsWatcher;

    ///Handles hanging protocols and automatic layouts
    LayoutManager *m_layoutManager;

#ifndef STARVIEWER_LITE
    /// Widget to be able to select related studies
    QRelatedStudiesWidget *m_relatedStudiesWidget;

    /// Manager for related studies
    RelatedStudiesManager *m_relatedStudiesManager;

    SyncActionsConfigurationHandler *m_syncActionsConfigurationHandler;
    SyncActionManager *m_syncActionManager;
    ViewersLayoutToSyncActionManagerAdapter *m_layoutToSyncActionManagerAdapter;
#endif

    /// Transfer function model to use when a viewer returns a null model or an empty viewer is selected.
    TransferFunctionModel *m_emptyTransferFunctionModel;

    /// Study Instance UID of current study.
    QString m_currentStudyUID;



//----------------------add 2020-09-18-------------------------------------------------------------------
public:
    void  updateQ2DViewer(Volume* volume);

    ExtensionHandler *m_extensionHandler;
private slots:
    void start3DViewer();
    void startMPR2DViewer();
};

} // end namespace udg

#endif
