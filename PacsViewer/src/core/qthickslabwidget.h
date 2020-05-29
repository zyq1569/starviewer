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

#ifndef UDGQTHICKSLABWIDGET_H
#define UDGQTHICKSLABWIDGET_H

#include "ui_qthickslabwidgetbase.h"

namespace udg {

class Q2DViewer;

/**
    Widget per controlar el Thick Slab d'un Q2DViewer
  */
class QThickSlabWidget : public QWidget, private Ui::QThickSlabWidgetBase {
Q_OBJECT
public:
    QThickSlabWidget(QWidget *parent = 0);

    ~QThickSlabWidget();

    void setSlabThickness(int thickness);
    // TODO int o QString?
    void setProjectionMode(int mode);

    /// Enllacem aquest controlador amb un Q2DViewer. De moment només està previst que
    /// s'enllaci amb un sol visor. Tenir més d'un visor linkat pot no tenir massa sentit ja que
    /// els thickness seran diferents i tampoc es veu cap utilitat de tenir més d'un thickslab
    /// "sincronitzat" alhora
    /// @param viewer Visualitzador al que linkem el control de thick slab
    void link(Q2DViewer *viewer);

    /// Desvinculem el widget del visor que tingui vinculat
    void unlink();

signals:
    void thicknessChanged(int thickness);
    // TODO enviar int o QString?
    void projectionModeChanged(int mode);

    /// S'emet cada cop que canvia l'estat del checkbox del mode de gruix màxim indicant si està marcat o no
    void maximumThicknessModeToggled(bool checked);

protected slots:
    /// Aquest slot està connectat als canvis en el combo i segons el valor
    /// escollit aplica sobre el visor el tipus de projecció adequat
    /// @param comboItem
    void applyProjectionMode(int comboItem);

    /// Actualiza el valor màxim de thickness que es mostra en el combo
    /// d'acord amb les llesques que té el volum del visor actual
    void updateMaximumThickness();

    /// Actualitza el valor de thickness que mostra el label. Anirà connectat amb l'slider
    /// @param value Valor de thickness
    void updateThicknessLabel(int value);

    /// Reseteja el viewer associat. Útil per quan ens canvien l'input, per exemple
    void reset();

    /// Aplica el thick slab segons el valor de l'slider.
    void applyThickSlab();

    /// Mètodes per controlar les connexions de l'slider amb l'actualització de l'slab
    void turnOnDelayedUpdate();
    void turnOffDelayedUpdate();
    void onSliderReleased();

    /// Es cridarà quan es canvïi la vista al visor associat.
    /// Segons les opcions marcades per l'usuari desactivarà o no la projecció aplicada.
    void onViewChanged();

    /// Si cert, posa el thickness al màxim i el manté encara que es canvïi de vista
    void enableVolumeMode(bool enable);

private:
    /// Desconnecta els signals i slots associats al viewer vinculat si en tenim
    void disconnectSignalsAndSlots();

protected:
    /// Visualitzador linkat al widget
    Q2DViewer *m_currentViewer;

};

}

#endif
