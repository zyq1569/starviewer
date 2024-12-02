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

#include "q3dviewerextension.h"
#include "volume.h"
#include "toolmanager.h"
#include "transferfunctionio.h"
#include "renderingstyle.h"
#include "starviewerapplication.h"
#include "q3dviewerextensionsettings.h"
#include "screenshottool.h"
#include "toolproxy.h"
#include "qexportertool.h"
#include "series.h"
#include "voilut.h"
// Qt
#include <QAction>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QTimer>
// Vtk
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
// Actualització ràpida
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
// Actualització ràpida
#include <vtkRenderWindowInteractor.h>
#include <vtkCornerAnnotation.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
namespace udg {

Q3DViewerExtension::Q3DViewerExtension(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    Q3DViewerExtensionSettings().init();

    //We create the timer (must be done before createConnections ())
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    /// \todo I put 1000 ms arbitrarily.
    m_timer->setInterval(1000);

    initializeTools();
    loadClutPresets();
    loadRenderingStyles();
    createConnections();
    updateUiForBlendMode(m_blendModeComboBox->currentIndex());

    m_firstInput = true;

    hideClutEditor();
    m_screenshotsExporterToolButton->setToolTip(tr("Export viewer image to DICOM and send it to a PACS server"));
    m_customStyleToolButton->setToolTip(tr("Show/Hide advanced colour options"));
    setWindowTitle("3DViewer");

    m_firstRemoveBed = false;
    m_saveVtkdata = NULL;
	m_lastInput = NULL;

	///-20241022--------------
	m_cornerAnnotations = vtkCornerAnnotation::New();
	m_cornerAnnotations->GetTextProperty()->SetFontFamilyToArial();
	m_cornerAnnotations->GetTextProperty()->ShadowOn();
	//for (int i = 0; i < 4; i++)
	//{
	//	m_patientOrientationTextActor[i] = vtkTextActor::New();
	//	m_patientOrientationTextActor[i]->SetTextScaleModeToNone();
	//	m_patientOrientationTextActor[i]->GetTextProperty()->SetFontSize(18);
	//	m_patientOrientationTextActor[i]->GetTextProperty()->BoldOn();
	//	m_patientOrientationTextActor[i]->GetTextProperty()->SetFontFamilyToArial();
	//	m_patientOrientationTextActor[i]->GetTextProperty()->ShadowOn();
    //
	//	m_patientOrientationTextActor[i]->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
	//	m_patientOrientationTextActor[i]->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
	//}
	//// Place each actor on its corresponding place. 0-3, counter-clockwise direction, starting at 0 = left of the viewer
	//m_patientOrientationTextActor[0]->GetTextProperty()->SetJustificationToLeft();
	//m_patientOrientationTextActor[0]->SetPosition(0.01, 0.5);
    //
	//m_patientOrientationTextActor[1]->GetTextProperty()->SetJustificationToCentered();
	//m_patientOrientationTextActor[1]->SetPosition(0.5, 0.01);
    //
	//m_patientOrientationTextActor[2]->GetTextProperty()->SetJustificationToRight();
	//m_patientOrientationTextActor[2]->SetPosition(0.99, 0.5);
    //
	//m_patientOrientationTextActor[3]->GetTextProperty()->SetJustificationToCentered();
	//m_patientOrientationTextActor[3]->GetTextProperty()->SetVerticalJustificationToTop();
	//m_patientOrientationTextActor[3]->SetPosition(0.5, 0.99);

}

Q3DViewerExtension::~Q3DViewerExtension()
{
    // What we want to do here is force all tools first before deleting the viewer
    // EVERYTHING might need to refactor the name of this method or create one for this task
    m_toolManager->disableAllToolsTemporarily();

    if (m_saveVtkdata)
    {
        m_saveVtkdata->Delete();
    }
	if (m_cornerAnnotations)
	{
		m_cornerAnnotations->Delete();
		//for (int i = 0; i < 4; ++i)
		//{
		//	m_patientOrientationTextActor[i]->Delete();
		//}
	}
}

void Q3DViewerExtension::initializeTools()
{
    m_toolManager = new ToolManager(this);
    //We get the actions of each tool we want
    m_zoomToolButton->setDefaultAction(m_toolManager->registerTool("ZoomTool"));
    m_rotate3DToolButton->setDefaultAction(m_toolManager->registerTool("Rotate3DTool"));
    m_windowLevelToolButton->setDefaultAction(m_toolManager->registerTool("WindowLevelTool"));
    m_toolManager->registerTool("TranslateTool");
    m_clippingBoxToolButton->setDefaultAction(m_toolManager->registerTool("ClippingPlanesTool"));   
    //m_removeBed->setDefaultAction(m_toolManager->registerTool("ClippingPlanesTool"));
    //m_removeBed->setText("Remove Bed");
    //m_removeBed->setToolTip("Auto Remove Bed");

    m_toolManager->registerTool("ScreenShotTool");
    m_screenShotToolButton->setToolTip(m_toolManager->getRegisteredToolAction("ScreenShotTool")->toolTip());

    // Action tools
    m_axialOrientationButton->setDefaultAction(m_toolManager->registerActionTool("AxialViewActionTool"));
    m_sagitalOrientationButton->setDefaultAction(m_toolManager->registerActionTool("SagitalViewActionTool"));
    m_coronalOrientationButton->setDefaultAction(m_toolManager->registerActionTool("CoronalViewActionTool"));

    // We activate the tools we want to have by default, this is as if we clicked on each of the ToolButton
    QStringList defaultTools;
    defaultTools << "WindowLevelTool"  << "TranslateTool" << "Rotate3DTool" << "ScreenShotTool";
    m_toolManager->triggerTools(defaultTools);

    // We register to the manager the tools that go with the main viewer
    m_toolManager->setupRegisteredTools(m_3DView);
    m_toolManager->enableRegisteredActionTools(m_3DView);

    QStringList rightButtonExclusiveTools;
    rightButtonExclusiveTools << "Rotate3DTool"<< "ZoomTool";
    m_toolManager->addExclusiveToolsGroup("LeftButtonGroup", rightButtonExclusiveTools);

    //Let's make a screen shot when the button is clicked
    ScreenShotTool *screenShotTool = dynamic_cast<ScreenShotTool*>(m_3DView->getToolProxy()->getTool("ScreenShotTool"));
    connect(m_screenShotToolButton, SIGNAL(clicked()), screenShotTool, SLOT(singleCapture()));
    //connect(m_removeBed, SIGNAL(clicked()), this, SLOT(removeBed()));
    m_removeBed->hide();
}

void Q3DViewerExtension::loadClutPresets()
{
    DEBUG_LOG("loadClutPresets()");

    disconnect(m_clutPresetsComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(applyPresetClut(const QString&)));

    m_clutPresetsComboBox->clear();

    QString path = QDir::toNativeSeparators(UserDataRootPath + "cluts/");
    m_clutsDir.setPath(path);

    if (!m_clutsDir.exists())
    {
        m_clutsDir.mkpath(path);
    }
    if (!m_clutsDir.isReadable())
    {
        // Unable to access directory
        return;
    }

    QStringList nameFilters;
    nameFilters << "*.tf";

    QStringList clutList = m_clutsDir.entryList(nameFilters);

    foreach (const QString &clutName, clutList)
    {
        TransferFunction *transferFunction = TransferFunctionIO::fromFile(m_clutsDir.absoluteFilePath(clutName));
        if (transferFunction)
        {
            m_clutNameToFileName[transferFunction->name()] = clutName;
            delete transferFunction;
        }
    }

    m_clutPresetsComboBox->addItems(m_clutNameToFileName.keys());
    m_clutPresetsComboBox->setCurrentIndex(-1);

    connect(m_clutPresetsComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(applyPresetClut(const QString&)));
}

void Q3DViewerExtension::loadRenderingStyles()
{
    m_renderingStyleModel = new QStandardItemModel(this);

    // For the first version we create all this by default.
    // For each style we create an item and assign some data that will be used to apply it.
    QStandardItem *item;
    RenderingStyle renderingStyle;
    TransferFunction *transferFunction;

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/spine2.png"), tr("Spine"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/spine2.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/thorax1.png"), tr("Thorax"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/thorax1.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/pelvis2.png"), tr("Pelvis"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/pelvis2.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/cow1.png"), tr("Circle of Willis"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(0.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/cow1.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/carotids2.png"), tr("Carotids"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/carotids2.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/bones4.png"), tr("Bones"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/bones4.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/bonehires.png"), tr("Bones 2"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(0.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/bonehires.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/abdomenbones.png"), tr("Abdomen bones"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/abdomenbones.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/abdomenrunoff1.png"), tr("Abdomen run-off"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/abdomenrunoff1.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/extensions/Q3DViewerExtension/renderingstyles/abdomenslab.png"), tr("Abdomen slab"));
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/extensions/Q3DViewerExtension/renderingstyles/abdomenslab.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    m_renderingStyleListView->setModel(m_renderingStyleModel);
}

void Q3DViewerExtension::createConnections()
{
    //Render method update
    connect(m_blendModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Q3DViewerExtension::updateUiForBlendMode);
    connect(m_renderModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Q3DViewerExtension::updateUiForBlendMode);

    enableAutoUpdate();

    // Clut editor
    connect(m_loadClutPushButton, SIGNAL(clicked()), SLOT(loadClut()));
    connect(m_saveClutPushButton, SIGNAL(clicked()), SLOT(saveClut()));
    connect(m_switchEditorPushButton, SIGNAL(clicked()), SLOT(switchEditor()));
    connect(m_applyPushButton, SIGNAL(clicked()), SLOT(applyEditorClut()));

    connect(m_customStyleToolButton, SIGNAL(clicked()), SLOT(toggleClutEditor()));

    connect(m_3DView, SIGNAL(transferFunctionChanged()), SLOT(changeViewerTransferFunction()));
    connect(m_3DView, &Q3DViewer::volumeChanged, [this](Volume *volume) {
        double range[2];
        volume->getScalarRange(range);
        this->setScalarRange(range[0], range[1]);
    });

    // Rendering styles
    connect(m_renderingStyleListView, SIGNAL(activated(const QModelIndex&)), SLOT(applyRenderingStyle(const QModelIndex&)));

    connect(m_editorSplitter, SIGNAL(splitterMoved(int, int)), SLOT(setCustomStyleButtonStateBySplitter()));

    // Temporitzador
    connect(m_timer, SIGNAL(timeout()), SLOT(render()));

    //To display export
    connect(m_screenshotsExporterToolButton, SIGNAL(clicked()), SLOT(showScreenshotsExporterDialog()));
}

void Q3DViewerExtension::removeBed()
{
    if (m_firstRemoveBed)
    {
        return;//only first;
    }
    if (!m_saveVtkdata)
    {
        m_saveVtkdata = vtkImageData::New();
    }
    vtkImageData *data = m_input->getVtkData();
    vtkImageThreshold* threshold = vtkImageThreshold::New();
    threshold->SetInputData(data);
    threshold->ThresholdBetween(-500,500); // Adjust the threshold value based on your data
    threshold->ReplaceInOn();
    threshold->SetInValue(0); // Set the value of the bed pixels to 0 Bitter@2live
    threshold->Update();
    m_saveVtkdata->DeepCopy(threshold->GetOutput());

    m_3DView->changevtkImageData(m_saveVtkdata);
    m_3DView->getRenderWindow()->Render();
    threshold->Delete();
    m_firstRemoveBed = true;
}

void Q3DViewerExtension::setInput(Volume *input)
{
	m_lastInput = input;
    m_input = input;
    m_3DView->setInput(m_input);

	m_3DView->getRenderer()->AddViewProp(m_cornerAnnotations);
	//m_3DView->getRenderer()->AddViewProp(m_patientOrientationTextActor[0]);
	//m_3DView->getRenderer()->AddViewProp(m_patientOrientationTextActor[1]);
	//m_3DView->getRenderer()->AddViewProp(m_patientOrientationTextActor[2]);
	//m_3DView->getRenderer()->AddViewProp(m_patientOrientationTextActor[3]);

    applyClut(m_currentClut);
    this->render();
}

void Q3DViewerExtension::setScalarRange(double min, double max)
{
    int minimum = qRound(min);
    int maximum = qRound(max);
    m_gradientEditor->setMinimum(minimum);
    m_gradientEditor->setMaximum(maximum);
    m_editorByValues->setMinimum(minimum);
    m_editorByValues->setMaximum(maximum);
    m_isoValueSpinBox->setMinimum(minimum);
    m_isoValueSpinBox->setMaximum(maximum);

    if (m_firstInput)
    {
        m_currentClut.set(min, Qt::black, 0.0);
        m_currentClut.set(max, Qt::white, 1.0);
        m_firstInput = false;
    }
}

void Q3DViewerExtension::applyPresetClut(const QString &clutName)
{
    DEBUG_LOG("applyPresetClut()");

    const QString &fileName = m_clutNameToFileName[clutName];
    TransferFunction *transferFunction = TransferFunctionIO::fromFile(m_clutsDir.absoluteFilePath(QDir::toNativeSeparators(fileName)));
    if (transferFunction)
    {
        //m_currentClut = *transferFunction;
        applyClut(*transferFunction, true);
        this->render();
        //m_3DView->setTransferFunction(transferFunction);
        delete transferFunction;
    }
    //this->render();
}

void Q3DViewerExtension::applyClut(const TransferFunction &clut, bool preset)
{
    DEBUG_LOG("applyClut()");

    m_currentClut = clut;
    if (!preset)
    {
        // You need to disconnect to avoid an infinite loop
        disconnect(m_clutPresetsComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(applyPresetClut(const QString&)));
        m_clutPresetsComboBox->setCurrentIndex(-1);
        connect(m_clutPresetsComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(applyPresetClut(const QString&)));
    }
    m_gradientEditor->setTransferFunction(m_currentClut);
    m_editorByValues->setTransferFunction(m_currentClut);
    m_3DView->setTransferFunction(m_currentClut);

	updateAnnotations();
    //     this->render();
}

void Q3DViewerExtension::changeViewerTransferFunction()
{
    //We update the clutter editor when changing the function for the w / l viewer
    m_gradientEditor->setTransferFunction(m_3DView->getTransferFunction());
    m_editorByValues->setTransferFunction(m_3DView->getTransferFunction());
	updateAnnotations();
}

void Q3DViewerExtension::render()
{
    this->setCursor(QCursor(Qt::WaitCursor));
    m_3DView->applyCurrentRenderingMethod();
    this->unsetCursor();

}

void Q3DViewerExtension::loadClut()
{
    Settings settings;

    QString customClutsDirPath = settings.getValue(Q3DViewerExtensionSettings::CustomClutsDirPath).toString();

    QString transferFunctionFileName =
            QFileDialog::getOpenFileName(this, tr("Load CLUT"),
                                         customClutsDirPath, tr("Transfer function files (*.tf);;All files (*)"));

    if (!transferFunctionFileName.isNull())
    {
        TransferFunction *transferFunction = TransferFunctionIO::fromFile(transferFunctionFileName);
        QTransferFunctionEditor *currentEditor = qobject_cast<QTransferFunctionEditor*>(m_editorsStackedWidget->currentWidget());
        currentEditor->setTransferFunction(*transferFunction);
        delete transferFunction;

        QFileInfo transferFunctionFileInfo(transferFunctionFileName);
        settings.setValue(Q3DViewerExtensionSettings::CustomClutsDirPath, transferFunctionFileInfo.absolutePath());
    }
}

void Q3DViewerExtension::saveClut()
{
    Settings settings;

    QString customClutsDirPath = settings.getValue(Q3DViewerExtensionSettings::CustomClutsDirPath).toString();

    QFileDialog saveDialog(this, tr("Save CLUT"), customClutsDirPath, tr("Transfer function files (*.tf);;All files (*)"));
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setDefaultSuffix("tf");

    if (saveDialog.exec() == QDialog::Accepted)
    {
        QString transferFunctionFileName = saveDialog.selectedFiles().first();
        QTransferFunctionEditor *currentEditor = qobject_cast<QTransferFunctionEditor*>(m_editorsStackedWidget->currentWidget());
        TransferFunctionIO::toFile(transferFunctionFileName, currentEditor->getTransferFunction());

        QFileInfo transferFunctionFileInfo(transferFunctionFileName);
        settings.setValue(Q3DViewerExtensionSettings::CustomClutsDirPath, transferFunctionFileInfo.absolutePath());
    }
}

void Q3DViewerExtension::switchEditor()
{
    QTransferFunctionEditor *currentEditor = qobject_cast<QTransferFunctionEditor*>(m_editorsStackedWidget->currentWidget());
    const TransferFunction &currentTransferFunction = currentEditor->getTransferFunction();
    m_editorsStackedWidget->setCurrentIndex(1 - m_editorsStackedWidget->currentIndex());
    currentEditor = qobject_cast<QTransferFunctionEditor*>(m_editorsStackedWidget->currentWidget());
    currentEditor->setTransferFunction(currentTransferFunction);
}

void Q3DViewerExtension::applyEditorClut()
{
    QTransferFunctionEditor *currentEditor = qobject_cast<QTransferFunctionEditor*>(m_editorsStackedWidget->currentWidget());
    applyClut(currentEditor->getTransferFunction());
    this->render();
	updateAnnotations();
}

void Q3DViewerExtension::toggleClutEditor()
{
    if (m_editorSplitter->sizes()[1] == 0)
    {
        // Show
        m_editorSplitter->setSizes(QList<int>() << 1 << 1);
        m_gradientEditor->setTransferFunction(m_currentClut);
        m_editorByValues->setTransferFunction(m_currentClut);
    }
    else
    {
        // Hide
        hideClutEditor();
    }
}

void Q3DViewerExtension::hideClutEditor()
{
    m_editorSplitter->setSizes(QList<int>() << 1 << 0);
}

void Q3DViewerExtension::setCustomStyleButtonStateBySplitter()
{
    m_customStyleToolButton->setChecked(m_editorSplitter->sizes()[1] != 0);
}

void Q3DViewerExtension::applyRenderingStyle(const QModelIndex &index)
{
    disableAutoUpdate();

    QStandardItem *item = m_renderingStyleModel->itemFromIndex(index);
    RenderingStyle renderingStyle = RenderingStyle::fromVariant(item->data());

    switch (renderingStyle.getMethod())
    {
    case RenderingStyle::RayCasting:
        m_blendModeComboBox->setCurrentIndex(0);
        applyClut(renderingStyle.getTransferFunction());
        m_shadingGroupBox->setChecked(renderingStyle.getShading());

        if (renderingStyle.getShading())
        {
            m_ambientCoefficientDoubleSpinBox->setValue(renderingStyle.getAmbientCoefficient());
            m_diffuseCoefficientDoubleSpinBox->setValue(renderingStyle.getDiffuseCoefficient());
            m_specularCoefficientDoubleSpinBox->setValue(renderingStyle.getSpecularCoefficient());
            m_specularPowerDoubleSpinBox->setValue(renderingStyle.getSpecularPower());
        }

        break;

    case RenderingStyle::MIP:
        m_blendModeComboBox->setCurrentIndex(1);
        break;

    case RenderingStyle::IsoSurface:
        m_blendModeComboBox->setCurrentIndex(5);
        m_isoValueSpinBox->setValue(static_cast<int>(qRound(renderingStyle.getIsoValue())));
        break;
    }

    updateView(false);
    enableAutoUpdate();
}

void Q3DViewerExtension::showScreenshotsExporterDialog()
{
    QExporterTool exporter(m_3DView);
    exporter.exec();
}

void Q3DViewerExtension::updateUiForBlendMode(int blendModeIndex)
{
    auto blendMode = Q3DViewer::BlendMode(blendModeIndex);

    m_shadingGroupBox->setVisible(blendMode == Q3DViewer::BlendMode::Composite);
    m_rayCastingOptionsWidget->setVisible(blendMode != Q3DViewer::BlendMode::Isosurface);
    m_isosurfaceOptionsWidget->setVisible(blendMode == Q3DViewer::BlendMode::Isosurface);
    m_transferFunctionsWidget->setVisible(blendMode == Q3DViewer::BlendMode::Composite ||
                                          blendMode == Q3DViewer::BlendMode::MaximumIntensity ||
                                          blendMode == Q3DViewer::BlendMode::MinimumIntensity);
    m_noTransferFunctionsWidget->setVisible(m_transferFunctionsWidget->isHidden());
}

void Q3DViewerExtension::updateView(bool fast)
{
    m_timer->stop();

    this->setCursor(QCursor(Qt::WaitCursor));

    m_3DView->setBlendMode(Q3DViewer::BlendMode(m_blendModeComboBox->currentIndex()));
    m_3DView->setRenderMode(Q3DViewer::RenderMode(m_renderModeComboBox->currentIndex()));
    m_3DView->setIndependentComponents(!m_colorImageCheckBox->isChecked());
    m_3DView->setShading(m_shadingGroupBox->isChecked());
    m_3DView->setAmbientCoefficient(m_ambientCoefficientDoubleSpinBox->value());
    m_3DView->setDiffuseCoefficient(m_diffuseCoefficientDoubleSpinBox->value());
    m_3DView->setSpecularCoefficient(m_specularCoefficientDoubleSpinBox->value());
    m_3DView->setSpecularPower(m_specularPowerDoubleSpinBox->value());
    m_3DView->setIsoValue(m_isoValueSpinBox->value());

    if (fast)
    {
        /// \todo Now we do it before rendering. It should be checked in slow systems whether it is better before or after.
        m_timer->start();
        m_3DView->getRenderWindow()->SetDesiredUpdateRate(m_3DView->getInteractor()->GetDesiredUpdateRate());
    }

    m_3DView->applyCurrentRenderingMethod();

    if (fast)
    {
        m_3DView->getRenderWindow()->SetDesiredUpdateRate(m_3DView->getInteractor()->GetStillUpdateRate());
    }

    this->unsetCursor();
}

void Q3DViewerExtension::enableAutoUpdate()
{
    connect(m_blendModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Q3DViewerExtension::updateView);
    connect(m_renderModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Q3DViewerExtension::updateView);
    connect(m_colorImageCheckBox, &QCheckBox::toggled, this, &Q3DViewerExtension::updateView);

    // Shading
    connect(m_shadingGroupBox, SIGNAL(toggled(bool)), this, SLOT(updateView()));
    connect(m_ambientCoefficientDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));
    connect(m_diffuseCoefficientDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));
    connect(m_specularCoefficientDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));
    connect(m_specularPowerDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));

    // Isosurfaces
    connect(m_isoValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateView()));
}

void Q3DViewerExtension::disableAutoUpdate()
{
    disconnect(m_blendModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Q3DViewerExtension::updateView);
    disconnect(m_renderModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Q3DViewerExtension::updateView);
    disconnect(m_colorImageCheckBox, &QCheckBox::toggled, this, &Q3DViewerExtension::updateView);

    // Shading
    disconnect(m_shadingGroupBox, SIGNAL(toggled(bool)), this, SLOT(updateView()));
    disconnect(m_ambientCoefficientDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));
    disconnect(m_diffuseCoefficientDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));
    disconnect(m_specularCoefficientDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));
    disconnect(m_specularPowerDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateView()));

    //Isosurfaces
    disconnect(m_isoValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateView()));
}

void Q3DViewerExtension::updateInput(Volume *input)
{
	if (m_lastInput != input)
	{
		m_input = input;
		m_3DView->setInput(m_input);
		m_lastInput = input;
		m_3DView->getRenderer()->ResetCamera();
		m_3DView->getRenderWindow()->Render();
		Series* ser = input->getSeries();
		QString title = ser->getSeriesNumber();
		setWindowTitle(title);
		updateAnnotations();
	}
}


void  Q3DViewerExtension::updateAnnotations()
{
	if (!m_lastInput)
	{
		return;
	}
	VoiLut voiLut = m_3DView->getCurrentVoiLut();
	WindowLevel windowLevel = voiLut.getWindowLevel();
	auto printWindowLevel = [](double x)
	{
		if (std::abs(x) >= 100)     // for |x| >= 100 -> 0 decimals
		{
			return QString::number(MathTools::roundToNearestInteger(x));
		}
		else if (std::abs(x) >= 10) // for 10 <= |x| < 100 -> at most 2 decimals
		{
			return QString::number(x, 'g', 4);
		}
		else if (std::abs(x) >= 1)  // for 1 <= |x| < 10 -> at most 2 decimals
		{
			return QString::number(x, 'g', 3);
		}
		else                        // for |x| < 1 -> 5 decimals
		{
			return QString::number(x, 'f', 5);
		}
	};
	QString windowLevelPart = QObject::tr("Ser: %1 WW: %2 WL: %3").arg(m_lastInput->getSeries()->getSeriesNumber()).arg(printWindowLevel(windowLevel.getWidth())).arg(printWindowLevel(windowLevel.getCenter()));
	m_cornerAnnotations->SetText(0, windowLevelPart.toLatin1().constData());
}

}
