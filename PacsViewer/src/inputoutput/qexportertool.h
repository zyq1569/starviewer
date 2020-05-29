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

#ifndef QEXPORTERTOOL_H
#define QEXPORTERTOOL_H

#include "ui_qexporterbase.h"
#include <QDialog>

class vtkImageData;

#include <vtkSmartPointer.h>

namespace udg {

class QViewer;
/**
    Classe encarregada de generar noves sèries a partir del que es captura per pantalla. Les series generades es poden guardar
    només a la base de dades local o poden ser enviades també al PACS seleccionat a través del widget.
    Actualment, les sèries que es generen són de tipus Secondary Capture.
  */
class QExporterTool : public QDialog, private Ui::QExporterBase {
Q_OBJECT
public:
    /// @pre el viewer no pot ser null
    QExporterTool(QViewer *viewer, QWidget *parent = 0);

    ~QExporterTool();

private:
    /// Crear les connexions
    void createConnections();
    /// Inicialitza les opcions de captura segons el tipus de d'estudi. També inicialitza la llista de PACS.
    void initialize();
    /// Captura l'estat actual del visor passat al constructor @sa QExporterTool.
    vtkSmartPointer<vtkImageData> captureCurrentView();

    /// Mètodes utilitzats per la generació de previsualitzacions
    void generateCurrentPreview();
    void generate2DPreview(int slice, int phase);
    void generatePreview();

    /// Mètodes per saber si tenim suficient espai pel volum que volem generar
    bool canAllocateEnoughMemory();
    bool canAllocateEnoughMemory(int numberOfScreenshots);

private slots:
    /// Generar i guardar la nova sèrie a la bdd i enviar-la al PACS si és el cas.
    void generateAndStoreNewSeries();

    /// Slots utilitzats per actualitzar el label del número d'imatges que es generaran
    void currentImageRadioButtonClicked();
    void allImagesRadioButtonClicked();
    void imageOfCurrentPhaseRadioButtonClicked();
    void phasesOfCurrentImageRadioButtonClicked();

private:
    /// Visor passat en el constructor.
    QViewer *m_viewer;

};

}
#endif // UDGEXPORTERTOOL_H
