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

#ifndef UDGQ2DVIEWERWIDGET_H
#define UDGQ2DVIEWERWIDGET_H

#include "ui_q2dviewerwidgetbase.h"

// FWD declarations
class QAction;

namespace udg {

class StatsWatcher;
class QFusionBalanceWidget;
class QFusionLayoutWidget;
class QViewerCommand;

/**
This class is the generic interface of a Q2DViewer along with its
corresponding slider and spinbox. It contains the method to put such an input
     as the methods for modifying the value of the slider and spinbox.
 */
class Q2DViewerWidget : public QStackedWidget, private ::Ui::Q2DViewerWidgetBase {
    Q_OBJECT
public:
    Q2DViewerWidget(QWidget *parent = 0);
    ~Q2DViewerWidget();

    /// Mark the widget as selected
    void setSelected(bool option);

    /// Returns the viewer
    Q2DViewer* getViewer();

    /// Retorna cert si el model té fases, fals altrament.
    bool hasPhases();

    /// Action by sync button
    void setDefaultAction(QAction *synchronizeAction);

    void setInput(Volume *input);
    void setInputAsynchronously(Volume *input, QViewerCommand *command = 0);

    ///Enables or disables the button to enable or disable the sync tool
    void enableSynchronizationButton(bool enable);

public slots:
    /// Enables or disables the synchronization tool in the viewer, if it is registered
    /// This method can be invoked by clicking on the sync button or by calling it directly
    void enableSynchronization(bool enable);

signals:
    ///This signal will be emitted when the mouse enters the widget
    void selected(Q2DViewerWidget *viewer);

    void manualSynchronizationStateChanged(bool enable);

    /// Emitted when the underlying viewer is double-clicked.
    void doubleClicked(Q2DViewerWidget *viewer);

    /// Emitted when one of the options of the fusion layout tool button has been selected
    /// to request a 2x1 CT+fused fusion layout with the given volumes and anatomical plane.
    void fusionLayout2x1FirstRequested(const QList<Volume*> &volumes, const AnatomicalPlane &anatomicalPlane);
    /// Emitted when one of the options of the fusion layout tool button has been selected
    /// to request a 2x1 fused+PT/NM fusion layout with the given volumes and anatomical plane.
    void fusionLayout2x1SecondRequested(const QList<Volume*> &volumes, const AnatomicalPlane &anatomicalPlane);
    /// Emitted when one of the options of the fusion layout tool button has been selected
    /// to request a 3x1 fusion layout with the given volumes and anatomical plane.
    void fusionLayout3x1Requested(const QList<Volume*> &volumes, const AnatomicalPlane &anatomicalPlane);
    /// Emitted when one of the options of the fusion layout tool button
    /// has been selected to request a 2x3 CT+fused fusion layout with given volumes.
    void fusionLayout2x3FirstRequested(const QList<Volume*> &volumes);
    /// Emitted when one of the options of the fusion layout tool button
    /// has been selected to request a 2x3 fused+PT/NM fusion layout with the given volumes.
    void fusionLayout2x3SecondRequested(const QList<Volume*> &volumes);
    /// Emitted when one of the options of the fusion layout tool button
    /// has been selected to request a 3x3 fusion layout with the given volumes.
    void fusionLayout3x3Requested(const QList<Volume*> &volumes);
    /// Emitted when one of the options of the fusion layout tool button
    /// has been selected to request a MPR R fusion layout with the given volumes.
    void fusionLayoutMprRightRequested(const QList<Volume*> &volumes);

protected:
    /// Event overload that occurs when the mouse clicks within the widget scope
    void mousePressEvent(QMouseEvent *mouseEvent);

private:
    /// Creates connections between signals and slots
    void createConnections();

    ///Enables / Disables all widgets in the slider bar, that is, all but the viewer.
    void setSliderBarWidgetsEnabled(bool enabled);

private slots:
    /// This slot will be called when some action is taken on the slider
    /// and depending on the action received will update the value of the slice in the viewfinder
    /// So when the user moves the slider, either with the
    /// mouse or clicking on the widget, we will give the correct value to the viewer
    void updateViewerSliceAccordingToSliderAction(int action);

    void updateInput(Volume *input);

    /// We make the viewer active
    void setAsActiveViewer();
    
    /// Quan el visualitzador s'ha seleccionat, emet el senyal amb aquest widget
    void emitSelectedViewer();

    /// Assigns the corresponding range and value of the slider according to the viewer's input
    void resetSliderRangeAndValue();

    /// Updates the value of the label that tells us which one
    /// projection of the plan we are seeing
    void updateProjectionLabel();

    /// Actualitza l'estat d'habilitat dels widgets de la barra amb l'slider a partir de l'estat del viewer
    void setSliderBarWidgetsEnabledFromViewerStatus();

    /// If the viewer has 2 inputs shows the fusion balance and fusion layout tool buttons
    /// and resets the fusion balance to 50% for each input.
    /// Otherwise hides the buttons.
    void resetFusionOptions();

    /// Emits the doubleClicked() signal.
    void emitDoubleClicked();

    /// Emits the fusionLayout2x1Requested() signal with CT and fused if the contained viewer has 2 volumes.
    void requestFusionLayout2x1First();
    /// Emits the fusionLayout2x1Requested() signal with fused and PT/NM if the contained viewer has 2 volumes.
    void requestFusionLayout2x1Second();
    /// Emits the fusionLayout3x1Requested() signal if the contained viewer has 2 volumes.
    void requestFusionLayout3x1();
    /// Emits the fusionLayout2x3Requested() signal with CT and fused if the contained viewer has 2 volumes.
    void requestFusionLayout2x3First();
    /// Emits the fusionLayout2x3Requested() signal with fused and PT/NM if the contained viewer has 2 volumes.
    void requestFusionLayout2x3Second();
    /// Emits the fusionLayout3x3Requested() signal if the contained viewer has 2 volumes.
    void requestFusionLayout3x3();
    /// Emits the fusionLayoutMprRightRequested() signal if the contained viewer has 2 volumes.
    void requestFusionLayoutMprRight();

private:
    /// Acció del boto de sincronitzar
    QAction *m_synchronizeButtonAction;

    /// Per fer estadístiques
    StatsWatcher *m_statsWatcher;

    /// Widget to adjust the fusion balance. Will be shown as the menu of the fusion balance tool button.
    QFusionBalanceWidget *m_fusionBalanceWidget;

    /// Widget to select and apply a fusion layout. Will be shown as the menu of the fusion layout tool button.
    QFusionLayoutWidget *m_fusionLayoutWidget;

};

};

#endif
