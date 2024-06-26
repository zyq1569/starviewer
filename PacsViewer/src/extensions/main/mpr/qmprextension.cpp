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

#include "qmprextension.h"

#include "drawer.h"
#include "drawerpoint.h"
#include "logging.h"
//By intersection calculations
#include "mathtools.h"
#include "mprsettings.h"
#include "patientbrowsermenu.h"
#include "q3dviewer.h"
#include "qexportertool.h"
#include "screenshottool.h"
#include "toolmanager.h"
#include "toolproxy.h"
#include "volume.h"
#include "voilutpresetstooldata.h"
// Qt
#include <QMessageBox>
#include <QMenu>
#include <QVector3D>
// VTK
#include <vtkAxisActor2D.h>
// For events
#include <vtkCommand.h>
// To take to the origin
#include <vtkImageChangeInformation.h>
#include <vtkImageReslice.h>
#include <vtkPlaneSource.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

namespace udg {

const double QMPRExtension::PickingDistanceThreshold = 7.0;

QMPRExtension::QMPRExtension(QWidget *parent)
    : QWidget(parent), m_axialZeroSliceCoordinate(.0)
{
    setupUi(this);
    MPRSettings().init();

    init();
    createActions();
    createConnections();
    createActors();
    readSettings();
    // Window level adjustments for the combo box
    m_voiLutComboBox->setPresetsData(m_axial2DView->getVoiLutData());
    m_sagital2DView->setVoiLutData(m_axial2DView->getVoiLutData());
    m_coronal2DView->setVoiLutData(m_axial2DView->getVoiLutData());
    m_voiLutComboBox->selectPreset(m_axial2DView->getVoiLutData()->getCurrentPresetName());

    initializeTools();

    m_thickSlab = 0.0;

    // TODO We hide buttons that are not absolutely necessary or will not be used
    // With "visible" we make available or not the 3D MIP
    bool visible = false;
    m_thickSlabLabel->setVisible(visible);
    m_thickSlabSlider->setVisible(visible);
    m_thickSlabSpinBox->setVisible(visible);
    m_mipToolButton->setVisible(visible);

    m_screenshotsExporterToolButton->setToolTip(tr("Export viewer image(s) to DICOM and send them to a PACS server"));
    m_viewerInformationToolButton->setToolTip(tr("Show/Hide viewer's textual information"));
    m_voiLutComboBox->setToolTip(tr("Choose a VOI LUT preset"));
}

QMPRExtension::~QMPRExtension()
{
    writeSettings();
    //Doing this or not seems to free up the same memory thanks to smart pointers
    if (m_sagitalReslice)
    {
        m_sagitalReslice->Delete();
    }
    if (m_coronalReslice)
    {
        m_coronalReslice->Delete();
    }

    m_transform->Delete();

    m_sagitalOverAxialAxisActor->Delete();
    m_axialOverSagitalIntersectionAxis->Delete();
    m_coronalOverAxialIntersectionAxis->Delete();
    m_coronalOverSagitalIntersectionAxis->Delete();
    m_thickSlabOverAxialActor->Delete();
    m_thickSlabOverSagitalActor->Delete();

    m_axialViewSagitalCenterDrawerPoint->decreaseReferenceCount();
    delete m_axialViewSagitalCenterDrawerPoint;
    m_axialViewCoronalCenterDrawerPoint->decreaseReferenceCount();
    delete m_axialViewCoronalCenterDrawerPoint;
    m_sagitalViewAxialCenterDrawerPoint->decreaseReferenceCount();
    delete m_sagitalViewAxialCenterDrawerPoint;
    m_sagitalViewCoronalCenterDrawerPoint->decreaseReferenceCount();
    delete m_sagitalViewCoronalCenterDrawerPoint;

    m_axialPlaneSource->Delete();
    m_sagitalPlaneSource->Delete();
    m_coronalPlaneSource->Delete();
    m_thickSlabPlaneSource->Delete();

    if (m_pickedActorReslice)
    {
        m_pickedActorReslice->Delete();
    }

    if (m_mipViewer)
    {
        delete m_mipViewer;
    }
    delete m_coronal2DView;
}

void QMPRExtension::init()
{
    m_axialPlaneSource = vtkPlaneSource::New();
    //This is how they are configured in vtkImagePlaneWidget
    m_axialPlaneSource->SetXResolution(1);
    m_axialPlaneSource->SetYResolution(1);

    m_sagitalPlaneSource = vtkPlaneSource::New();
    m_sagitalPlaneSource->SetXResolution(1);
    m_sagitalPlaneSource->SetYResolution(1);

    m_coronalPlaneSource = vtkPlaneSource::New();
    m_coronalPlaneSource->SetXResolution(1);
    m_coronalPlaneSource->SetYResolution(1);

    m_thickSlabPlaneSource = vtkPlaneSource::New();
    m_thickSlabPlaneSource->SetXResolution(1);
    m_thickSlabPlaneSource->SetYResolution(1);

    m_sagitalReslice = 0;
    m_coronalReslice = 0;

    // Configurem les annotacions que volem veure
    m_sagital2DView->removeAnnotation(PatientOrientationAnnotation | MainInformationAnnotation | SliceAnnotation);
    m_coronal2DView->removeAnnotation(PatientOrientationAnnotation | MainInformationAnnotation | SliceAnnotation);
    showViewerInformation(m_viewerInformationToolButton->isChecked());

    m_sagital2DView->disableContextMenu();
    m_coronal2DView->disableContextMenu();

    // Per defecte isomètric
    m_axialSpacing[0] = 1.;
    m_axialSpacing[1] = 1.;
    m_axialSpacing[2] = 1.;

    m_state = None;

    m_transform = vtkTransform::New();

    m_pickedActorPlaneSource = 0;
    m_pickedActorReslice = 0;
    m_mipViewer = 0;

    m_extensionToolsList << "ZoomTool" << "SlicingMouseTool" << "TranslateTool" << "VoxelInformationTool" << "WindowLevelTool" << "ScreenShotTool"
                         << "DistanceTool" << "PolylineROITool" << "EllipticalROITool" << "EraserTool";
}

void QMPRExtension::createActions()
{
    m_horizontalLayoutAction = new QAction(0);
    m_horizontalLayoutAction->setText(tr("Switch horizontal layout"));
    m_horizontalLayoutAction->setStatusTip(tr("Switch horizontal layout"));
    m_horizontalLayoutAction->setIcon(QIcon(":/images/icons/view-split-left-right.svg"));
    m_horizontalLayoutToolButton->setDefaultAction(m_horizontalLayoutAction);

    m_mipAction = new QAction(0);
    m_mipAction->setText(tr("&MIP"));
    m_mipAction->setShortcut(tr("Ctrl+M"));
    m_mipAction->setStatusTip(tr("Maximum Intensity Projection"));
    m_mipAction->setIcon(QIcon(":/images/icons/thick-slab.svg"));
    m_mipAction->setCheckable(true);
    m_mipToolButton->setDefaultAction(m_mipAction);
}

void QMPRExtension::initializeZoomTools()
{
    Q_ASSERT(m_toolManager);
    
    m_zoomToolButton->setDefaultAction(m_toolManager->registerTool("ZoomTool"));
    // We add a menu to the zoom button to incorporate the focused zoom tool
    m_zoomToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu *zoomToolMenu = new QMenu(this);
    m_zoomToolButton->setMenu(zoomToolMenu);
    zoomToolMenu->addAction(m_toolManager->registerTool("MagnifyingGlassTool"));

    connect(m_toolManager->getRegisteredToolAction("ZoomTool"), SIGNAL(triggered()), SLOT(rearrangeZoomToolsMenu()));
    connect(m_toolManager->getRegisteredToolAction("MagnifyingGlassTool"), SIGNAL(triggered()), SLOT(rearrangeZoomToolsMenu()));
}

void QMPRExtension::initializeROITools()
{
    Q_ASSERT(m_toolManager);
    
    m_ROIToolButton->setDefaultAction(m_toolManager->registerTool("EllipticalROITool"));
    // We add a menu to the PolylineROI button to incorporate the elliptical ROI tool
    m_ROIToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu *roiToolMenu = new QMenu(this);
    m_ROIToolButton->setMenu(roiToolMenu);
    roiToolMenu->addAction(m_toolManager->registerTool("MagicROITool"));
    roiToolMenu->addAction(m_toolManager->registerTool("PolylineROITool"));
    roiToolMenu->addAction(m_toolManager->registerTool("CircleTool"));
    
    connect(m_toolManager->getRegisteredToolAction("EllipticalROITool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
    connect(m_toolManager->getRegisteredToolAction("MagicROITool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
    connect(m_toolManager->getRegisteredToolAction("PolylineROITool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
    connect(m_toolManager->getRegisteredToolAction("CircleTool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
}

void QMPRExtension::initializeDistanceTools()
{
    Q_ASSERT(m_toolManager);

    m_distanceToolButton->setDefaultAction(m_toolManager->registerTool("DistanceTool"));
    //We add a menu to the distance button to incorporate the perpendicular distance tool
    m_distanceToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu *distanceToolMenu = new QMenu(this);
    m_distanceToolButton->setMenu(distanceToolMenu);
    distanceToolMenu->addAction(m_toolManager->registerTool("PerpendicularDistanceTool"));
    connect(m_toolManager->getRegisteredToolAction("DistanceTool"), SIGNAL(triggered()), SLOT(rearrangeDistanceToolsMenu()));
    connect(m_toolManager->getRegisteredToolAction("PerpendicularDistanceTool"), SIGNAL(triggered()), SLOT(rearrangeDistanceToolsMenu()));
}

void QMPRExtension::initializeAngleTools()
{
    Q_ASSERT(m_toolManager);

    m_angleToolButton->setDefaultAction(m_toolManager->registerTool("AngleTool"));
    // We add a menu to the angle button to incorporate the open angle tool
    m_angleToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu *angleToolMenu = new QMenu(this);
    m_angleToolButton->setMenu(angleToolMenu);
    angleToolMenu->addAction(m_toolManager->registerTool("NonClosedAngleTool"));
    connect(m_toolManager->getRegisteredToolAction("AngleTool"), SIGNAL(triggered()), SLOT(rearrangeAngleToolsMenu()));
    connect(m_toolManager->getRegisteredToolAction("NonClosedAngleTool"), SIGNAL(triggered()), SLOT(rearrangeAngleToolsMenu()));
}

void QMPRExtension::initializeTools()
{
    // Creem el tool manager
    m_toolManager = new ToolManager(this);
    //We configure the different tools
    initializeZoomTools();
    initializeROITools();
    initializeDistanceTools();
    initializeAngleTools();
    m_slicingToolButton->setDefaultAction(m_toolManager->registerTool("SlicingMouseTool"));
    m_toolManager->registerTool("TranslateTool");
    m_toolManager->registerTool("WindowLevelTool");
    m_voxelInformationToolButton->setDefaultAction(m_toolManager->registerTool("VoxelInformationTool"));
    m_toolManager->registerTool("ScreenShotTool");
    m_screenShotToolButton->setToolTip(m_toolManager->getRegisteredToolAction("ScreenShotTool")->toolTip());
    
    m_eraserToolButton->setDefaultAction(m_toolManager->registerTool("EraserTool"));
    m_toolManager->registerTool("VoiLutPresetsTool");
    m_toolManager->registerTool("SlicingKeyboardTool");
    m_toolManager->registerTool("SlicingWheelTool");

    // We define exclusive groups
    QStringList leftButtonExclusiveTools;
    leftButtonExclusiveTools << "ZoomTool" << "SlicingMouseTool" << "PolylineROITool" << "DistanceTool" << "PerpendicularDistanceTool" << "EraserTool" << "AngleTool"
                             << "NonClosedAngleTool" << "Cursor3DTool" << "EllipticalROITool" << "MagicROITool" << "CircleTool" << "MagnifyingGlassTool";
    m_toolManager->addExclusiveToolsGroup("LeftButtonGroup", leftButtonExclusiveTools);

    // We activate the tools we want to have by default, this is as if we clicked on each of the ToolButton
    QStringList defaultTools;
    defaultTools << "VoiLutPresetsTool" << "SlicingMouseTool" << "SlicingWheelTool" << "WindowLevelTool" << "TranslateTool" << "ScreenShotTool" << "SlicingKeyboardTool";
    m_toolManager->triggerTools(defaultTools);

    //Registrem al manager les tools que van als diferents viewers
    m_toolManager->setupRegisteredTools(m_axial2DView);
    // On the other viewers we want all the registered tools but slicing (mouse and keyboard modes)
    QStringList toolsList = m_toolManager->getRegisteredToolsList();
    toolsList.removeAt(toolsList.indexOf("SlicingMouseTool"));
    toolsList.removeAt(toolsList.indexOf("SlicingKeyboardTool"));
    toolsList.removeAt(toolsList.indexOf("SlicingWheelTool"));
    m_toolManager->setViewerTools(m_sagital2DView, toolsList);
    m_toolManager->setViewerTools(m_coronal2DView, toolsList);
}

void QMPRExtension::createConnections()
{
    //// We connect the sliders and other viewers
    connect(m_axial2DView, SIGNAL(sliceChanged(int)), m_axialSlider, SLOT(setValue(int)));
    connect(m_axialSlider, SIGNAL(valueChanged(int)), m_axial2DView, SLOT(setSlice(int)));

    connect(m_axial2DView, SIGNAL(sliceChanged(int)), SLOT(axialSliceUpdated(int)));
    connect(m_axial2DView, SIGNAL(sliceChanged(int)), SLOT(updateProjectionLabel()));

    // They manage window events so they can manipulate plans
    connect(m_axial2DView, SIGNAL(eventReceived(unsigned long)), SLOT(handleAxialViewEvents(unsigned long)));
    connect(m_sagital2DView, SIGNAL(eventReceived(unsigned long)), SLOT(handleSagitalViewEvents(unsigned long)));

    connect(m_thickSlabSpinBox, SIGNAL(valueChanged(double)), SLOT(updateThickSlab(double)));
    connect(m_thickSlabSlider, SIGNAL(valueChanged(int)), SLOT(updateThickSlab(int)));

    // Layouts
    connect(m_horizontalLayoutAction, SIGNAL(triggered()), SLOT(switchHorizontalLayout()));
    connect(m_mipAction, SIGNAL(triggered(bool)), SLOT(switchToMIPLayout(bool)));

    // We do not automatically assign the input that has been selected with the patient menu, as we do additional treatments
    // on the selected volume and the final input of the viewer may differ from the initial one and it is the extension that finally decides which input
    // you want to give it to each viewer. We capture the signal of what volume has been chosen and from there we do what is needed
    m_axial2DView->setAutomaticallyLoadPatientBrowserMenuSelectedInput(false);
    connect(m_axial2DView->getPatientBrowserMenu(), SIGNAL(selectedVolume(Volume*)), SLOT(setInput(Volume*)));
    // HACK To make universal scrolling work properly. Issue #2019. We need to be aware of the volume being changed by another tool.
    connect(m_axial2DView, SIGNAL(volumeChanged(Volume*)), SLOT(setInput(Volume*)));
    // Show or not the volume information to each viewer
    connect(m_viewerInformationToolButton, SIGNAL(toggled(bool)), SLOT(showViewerInformation(bool)));

    // HACK To be able to use the screenshot tool with the button
    connect(m_axial2DView, SIGNAL(selected()), SLOT(changeSelectedViewer()));
    connect(m_sagital2DView, SIGNAL(selected()), SLOT(changeSelectedViewer()));
    connect(m_coronal2DView, SIGNAL(selected()), SLOT(changeSelectedViewer()));
    connect(m_screenShotToolButton, SIGNAL(clicked()), SLOT(screenShot()));

    //  To display export
    connect(m_screenshotsExporterToolButton, SIGNAL(clicked()), SLOT(showScreenshotsExporterDialog()));
}

void QMPRExtension::rearrangeToolsMenu(QToolButton *menuButton)
{
    QList<QAction*> actions;
    actions << menuButton->defaultAction() << menuButton->menu()->actions();

    bool found = false;
    int i = 0;
    while (!found && i < actions.count())
    {
        if (actions.at(i)->isChecked())
        {
            found = true;
        }
        ++i;
    }

    if (found)
    {
        menuButton->setDefaultAction(actions.takeAt(i - 1));
        menuButton->menu()->clear();
        menuButton->menu()->addActions(actions);
    }
}

void QMPRExtension::rearrangeROIToolsMenu()
{
    rearrangeToolsMenu(m_ROIToolButton);
}

void QMPRExtension::rearrangeAngleToolsMenu()
{
    rearrangeToolsMenu(m_angleToolButton);
}

void QMPRExtension::rearrangeZoomToolsMenu()
{
    rearrangeToolsMenu(m_zoomToolButton);
}

void QMPRExtension::rearrangeDistanceToolsMenu()
{
    rearrangeToolsMenu(m_distanceToolButton);
}

void QMPRExtension::changeSelectedViewer()
{
    if (this->sender() == m_axial2DView)
    {
        m_axial2DView->setActive(true);

        m_sagital2DView->setActive(false);
        m_coronal2DView->setActive(false);
    }
    else if (this->sender() == m_sagital2DView)
    {
        m_sagital2DView->setActive(true);

        m_axial2DView->setActive(false);
        m_coronal2DView->setActive(false);
    }
    else if (this->sender() == m_coronal2DView)
    {
        m_coronal2DView->setActive(true);

        m_axial2DView->setActive(false);
        m_sagital2DView->setActive(false);
    }
}

void QMPRExtension::screenShot()
{
    ScreenShotTool *screenShotTool = 0;
    if (m_axial2DView->isActive())
    {
        screenShotTool = dynamic_cast<ScreenShotTool*>(m_axial2DView->getToolProxy()->getTool("ScreenShotTool"));
    }
    else if (m_sagital2DView->isActive())
    {
        screenShotTool = dynamic_cast<ScreenShotTool*>(m_sagital2DView->getToolProxy()->getTool("ScreenShotTool"));
    }
    else if (m_coronal2DView->isActive())
    {
        screenShotTool = dynamic_cast<ScreenShotTool*>(m_coronal2DView->getToolProxy()->getTool("ScreenShotTool"));
    }

    if (screenShotTool)
    {
        screenShotTool->singleCapture();
    }
    else
    {
        DEBUG_LOG("No hi ha tool d'screenshot disponible");
    }
}

void QMPRExtension::showScreenshotsExporterDialog()
{
    Q2DViewer *viewer = 0;
    if (m_axial2DView->isActive())
    {
        viewer = m_axial2DView;
    }
    else if (m_sagital2DView->isActive())
    {
        viewer = m_sagital2DView;
    }
    else if (m_coronal2DView->isActive())
    {
        viewer = m_coronal2DView;
    }

    if (viewer)
    {
        QExporterTool exporter(viewer);
        exporter.exec();
    }
    else
    {
        QMessageBox::warning(this, tr("Export to DICOM"), tr("Please, select a viewer and try again."));
    }
}

void QMPRExtension::showViewerInformation(bool show)
{
    m_axial2DView->enableAnnotation(VoiLutAnnotation | PatientOrientationAnnotation | SliceAnnotation | MainInformationAnnotation, show);
    m_sagital2DView->enableAnnotation(VoiLutAnnotation, show);
    m_coronal2DView->enableAnnotation(VoiLutAnnotation, show);
}

void QMPRExtension::updateProjectionLabel()
{
    m_projectionLabel->setText(m_axial2DView->getCurrentAnatomicalPlaneLabel());
}

void QMPRExtension::switchHorizontalLayout()
{
    QWidget *leftWidget, *rightWidget;
    leftWidget = m_horizontalSplitter->widget(0);
    rightWidget = m_horizontalSplitter->widget(1);

    m_horizontalSplitter->insertWidget(0, rightWidget);
    m_horizontalSplitter->insertWidget(1, leftWidget);
}

void QMPRExtension::switchToMIPLayout(bool isMIPChecked)
{
    //We save the size before changing the widgets
    QList<int> splitterSize = m_horizontalSplitter->sizes();
    if (isMIPChecked)
    {
        if (!m_mipViewer)
        {
            m_mipViewer = new Q3DViewer;
            m_mipViewer->orientationMarkerOff();
            m_mipViewer->setBlendMode(Q3DViewer::BlendMode::MaximumIntensity);
        }
        Volume *mipInput = new Volume;
        // TODO This is necessary so that you have the information of the series, studies, patient ...
        mipInput->setImages(m_volume->getImages());
        mipInput->setData(m_coronalReslice->GetOutput());
        m_mipViewer->setInput(mipInput);
        m_mipViewer->render();
        m_mipViewer->show();
        //We have widget distribution
        m_horizontalSplitter->insertWidget(m_horizontalSplitter->indexOf(m_verticalSplitter), m_mipViewer);
        m_verticalSplitter->hide();
        m_horizontalSplitter->insertWidget(2, m_verticalSplitter);
    }
    else
    {
        m_horizontalSplitter->insertWidget(m_horizontalSplitter->indexOf(m_mipViewer), m_verticalSplitter);
        m_verticalSplitter->show();
        m_mipViewer->hide();
        m_horizontalSplitter->insertWidget(2, m_mipViewer);
    }
    // We recover the sizes
    m_horizontalSplitter->setSizes(splitterSize);
}

void QMPRExtension::handleAxialViewEvents(unsigned long eventID)
{
    switch (eventID)
    {
    case vtkCommand::LeftButtonPressEvent:
        if (detectAxialViewAxisActor())
        {
            if (m_axial2DView->getInteractor()->GetControlKey())
            {
                m_state = Pushing;
            }
            else
            {
                m_state = Rotating;
            }
        }
        break;

    case vtkCommand::LeftButtonReleaseEvent:
        if (m_state != None)
        {
            releaseAxialViewAxisActor();
        }
        break;

    case vtkCommand::MouseMoveEvent:
        if (m_state == Rotating)
        {
            rotateAxialViewAxisActor();
        }
        else if (m_state == Pushing)
        {
            pushAxialViewAxisActor();
        }

        if (m_pickedActorPlaneSource == m_coronalPlaneSource && m_coronal2DView->getDrawer()->getNumberOfDrawnPrimitives() > 0)
        {
            m_coronal2DView->getDrawer()->removeAllPrimitives();
        }

        if (m_pickedActorPlaneSource == m_sagitalPlaneSource && m_sagital2DView->getDrawer()->getNumberOfDrawnPrimitives() > 0)
        {
            m_sagital2DView->getDrawer()->removeAllPrimitives();
        }

        break;

    default:
        break;
    }
}

void QMPRExtension::handleSagitalViewEvents(unsigned long eventID)
{
    switch (eventID)
    {
    case vtkCommand::LeftButtonPressEvent:
        if (m_sagital2DView->getInteractor()->GetControlKey())
        {
            detectPushSagitalViewAxisActor();
        }
        else
        {
            detectSagitalViewAxisActor();
        }
        break;

    case vtkCommand::LeftButtonReleaseEvent:
        if (m_state != None)
        {
            releaseSagitalViewAxisActor();
        }
        break;

    case vtkCommand::MouseMoveEvent:
        if (m_state == Rotating)
        {
            rotateSagitalViewAxisActor();
        }
        else if (m_state == Pushing)
        {
            if (m_pickedActorPlaneSource == m_coronalPlaneSource)
            {
                pushSagitalViewCoronalAxisActor();
            }
            else
            {
                pushSagitalViewAxialAxisActor();
            }
        }

        if (m_pickedActorPlaneSource == m_coronalPlaneSource && m_coronal2DView->getDrawer()->getNumberOfDrawnPrimitives() > 0)
        {
            m_coronal2DView->getDrawer()->removeAllPrimitives();
        }
        break;
    default:
        break;
    }
}

bool QMPRExtension::detectAxialViewAxisActor()
{
    bool picked = false;
    double clickedWorldPoint[3];
    double dummyPoint[3];
    m_axial2DView->getEventWorldCoordinate(clickedWorldPoint);

    // We detect which actor is closest, identify it, and then leave it as selected
    double point[3] = { clickedWorldPoint[0], clickedWorldPoint[1], 0.0 };
    double *r1, *r2;
    double distanceToCoronal, distanceToSagital;

    r1 = m_coronalOverAxialIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_coronalOverAxialIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToCoronal = MathTools::getPointToFiniteLineDistance(point, r1, r2, dummyPoint);

    r1 = m_sagitalOverAxialAxisActor->GetPositionCoordinate()->GetValue();
    r2 = m_sagitalOverAxialAxisActor->GetPosition2Coordinate()->GetValue();
    distanceToSagital = MathTools::getPointToFiniteLineDistance(point, r1, r2, dummyPoint);

    // We pass distances to view coordinates to control tolerance
    double zeroInDisplay[3];
    m_axial2DView->computeWorldToDisplay(0.0, 0.0, 0.0, zeroInDisplay);
    double distanceToCoronalInDisplay[3];
    m_axial2DView->computeWorldToDisplay(distanceToCoronal, 0.0, 0.0, distanceToCoronalInDisplay);
    distanceToCoronal = MathTools::getDistance3D(zeroInDisplay, distanceToCoronalInDisplay);
    double distanceToSagitalInDisplay[3];
    m_axial2DView->computeWorldToDisplay(distanceToSagital, 0.0, 0.0, distanceToSagitalInDisplay);
    distanceToSagital = MathTools::getDistance3D(zeroInDisplay, distanceToSagitalInDisplay);

    // We give a minimum “tolerance”
    if (distanceToCoronal < PickingDistanceThreshold || distanceToSagital < PickingDistanceThreshold)
    {
        if (distanceToCoronal < distanceToSagital)
        {
            m_pickedActorPlaneSource = m_coronalPlaneSource;
            m_pickedActorReslice = m_coronalReslice;
        }
        else
        {
            m_pickedActorPlaneSource = m_sagitalPlaneSource;
            m_pickedActorReslice = m_sagitalReslice;
        }
        m_pickedActorReslice->SetInterpolationModeToNearestNeighbor();
        //We deactivate tools that may be active
        m_toolManager->disableAllToolsTemporarily();
        m_initialPickX = clickedWorldPoint[0];
        m_initialPickY = clickedWorldPoint[1];
        picked = true;
    }
    return picked;
}

void QMPRExtension::rotateAxialViewAxisActor()
{
    double clickedWorldPoint[3];
    m_axial2DView->getEventWorldCoordinate(clickedWorldPoint);

    // Vectors des del centre del picked plane a m_initialPick i clickedWorldPoint
    double vec1[3], vec2[3];

    vec1[0] = m_initialPickX - m_pickedActorPlaneSource->GetCenter()[0];
    vec1[1] = m_initialPickY - m_pickedActorPlaneSource->GetCenter()[1];
    vec1[2] = 0.0;

    vec2[0] = clickedWorldPoint[0] - m_pickedActorPlaneSource->GetCenter()[0];
    vec2[1] = clickedWorldPoint[1] - m_pickedActorPlaneSource->GetCenter()[1];
    vec2[2] = 0.0;

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];

    //Angle of rotation in degrees
    double angle = MathTools::angleInDegrees(QVector3D(vec1[0], vec1[1], vec1[2]), QVector3D(vec2[0], vec2[1], vec2[2]));

    // Direction of the axis of rotation
    double direction[3];
    MathTools::crossProduct(vec1, vec2, direction);

    // Axis of rotation in world coordinates
    double axis[3];
    m_axialPlaneSource->GetNormal(axis);

    //We calculate the scalar product to know the direction of the axis (and therefore of the turn)
    double dot = MathTools::dotProduct(direction, axis);
    axis[0] *= dot;
    axis[1] *= dot;
    axis[2] *= dot;
    MathTools::normalize(axis);

    rotateMiddle(angle, axis, m_pickedActorPlaneSource);

    updatePlanes();
    updateControls();
}

void QMPRExtension::releaseAxialViewAxisActor()
{
    if (m_pickedActorReslice)
    {
        m_pickedActorReslice->SetInterpolationModeToCubic();
        // TODO No seria millor un restoreOverrideCursor?
        m_axial2DView->unsetCursor();
        if (m_pickedActorPlaneSource == m_sagitalPlaneSource)
        {
            m_sagital2DView->render();
        }
        else
        {
            m_coronal2DView->render();
        }
        m_state = None;
        m_pickedActorReslice = 0;
        m_pickedActorPlaneSource = 0;
        // Reactivem les tools
        m_toolManager->undoDisableAllToolsTemporarily();
    }
}

void QMPRExtension::detectSagitalViewAxisActor()
{
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate(clickedWorldPoint);

    // We detect which actor is closest, identify it, and then leave it as selected
    double point[3] = { clickedWorldPoint[0], clickedWorldPoint[1], 0.0 };
    double *r1, *r2;
    double distanceToCoronal;
    double dummyPoint[3];

    r1 = m_coronalOverSagitalIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_coronalOverSagitalIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToCoronal = MathTools::getPointToFiniteLineDistance(point, r1, r2, dummyPoint);

    //We pass distances to view coordinates to control tolerance
    double zeroInDisplay[3];
    m_sagital2DView->computeWorldToDisplay(0.0, 0.0, 0.0, zeroInDisplay);
    double distanceToCoronalInDisplay[3];
    m_sagital2DView->computeWorldToDisplay(distanceToCoronal, 0.0, 0.0, distanceToCoronalInDisplay);
    distanceToCoronal = MathTools::getDistance3D(zeroInDisplay, distanceToCoronalInDisplay);

    // We give a minimum “tolerance”
    if (distanceToCoronal < PickingDistanceThreshold)
    {
        m_pickedActorReslice = m_coronalReslice;
        m_pickedActorReslice->SetInterpolationModeToNearestNeighbor();
        m_pickedActorPlaneSource = m_coronalPlaneSource;
        // We deactivate tools that may be active
        m_toolManager->disableAllToolsTemporarily();

        m_initialPickX = clickedWorldPoint[0];
        m_initialPickY = clickedWorldPoint[1];
        m_state = Rotating;
    }
}

void QMPRExtension::rotateSagitalViewAxisActor()
{
    //Sagittal coordinates
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate(clickedWorldPoint);

    //  Transformation of world coordinates to sagittal coordinates
    vtkTransform *transform = getWorldToSagitalTransform();

    //Center of the picked plane (sagittal coordinates)
    double pickedPlaneCenter[3];
    transform->TransformPoint(m_pickedActorPlaneSource->GetCenter(), pickedPlaneCenter);

    ///  Vectors from the center of the picked plane (always the coronal)
    ///  to m_initialPick and clickedWorldPoint (sagittal coordinates)
    double vec1[3], vec2[3];

    vec1[0] = m_initialPickX - pickedPlaneCenter[0];
    vec1[1] = m_initialPickY - pickedPlaneCenter[1];
    vec1[2] = 0.0;

    vec2[0] = clickedWorldPoint[0] - pickedPlaneCenter[0];
    vec2[1] = clickedWorldPoint[1] - pickedPlaneCenter[1];
    vec2[2] = 0.0;

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];

    // Angle of rotation in degrees
    double angle = MathTools::angleInDegrees(QVector3D(vec1[0], vec1[1], vec1[2]), QVector3D(vec2[0], vec2[1], vec2[2]));

    //  Direction of the axis of rotation (sagittal coordinates)
    double direction[3];
    MathTools::crossProduct(vec1, vec2, direction);
    // Transformation of sagittal coordinates to world coordinates
    transform->Inverse();
    transform->TransformVector(direction, direction);
    /// Now direction is the direction of the axis of rotation in world coordinates

    // We no longer have to make further transformations; we destroy transform
    transform->Delete();

    // Axis of rotation in world coordinates
    double axis[3];
    m_sagitalPlaneSource->GetNormal(axis);

    //We calculate the scalar product to know the direction of the axis (and therefore of the turn)
    double dot = MathTools::dotProduct(direction, axis);
    axis[0] *= dot;
    axis[1] *= dot;
    axis[2] *= dot;
    MathTools::normalize(axis);

    rotateMiddle(angle, axis, m_pickedActorPlaneSource);

    updatePlanes();
    updateControls();
}

void QMPRExtension::releaseSagitalViewAxisActor()
{
    if (m_pickedActorReslice)
    {
        m_sagital2DView->unsetCursor();
        m_pickedActorReslice->SetInterpolationModeToCubic();
        m_coronal2DView->render();
        m_state = None;
        m_pickedActorReslice = 0;
        m_pickedActorPlaneSource = 0;
        // Reactivem the tools
        m_toolManager->undoDisableAllToolsTemporarily();
    }
}

void QMPRExtension::getRotationAxis(vtkPlaneSource *plane, double axis[3])
{
    if (!plane)
    {
        return;
    }

    axis[0] = plane->GetPoint2()[0] - plane->GetOrigin()[0];
    axis[1] = plane->GetPoint2()[1] - plane->GetOrigin()[1];
    axis[2] = plane->GetPoint2()[2] - plane->GetOrigin()[2];
}

void QMPRExtension::pushSagitalViewCoronalAxisActor()
{
    m_sagital2DView->setCursor(Qt::ClosedHandCursor);

    // Sagittal coordinates
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate(clickedWorldPoint);

    // Translation of the coronal plane (sagittal coordinates)
    double translation[3];
    translation[0] = clickedWorldPoint[0] - m_initialPickX;
    translation[1] = clickedWorldPoint[1] - m_initialPickY;
    translation[2] = 0.0;

    // Transformation of sagittal coordinates to world coordinates
    vtkTransform *sagitalToWorldTransform = getWorldToSagitalTransform();
    sagitalToWorldTransform->Inverse();

    sagitalToWorldTransform->TransformVector(translation, translation);
    // Now translation is the translation of the coronal plane into world coordinates

    // We no longer have to make further transformations; we destroy sagittalToWorldTransform
    sagitalToWorldTransform->Delete();

    m_pickedActorPlaneSource->Push(MathTools::dotProduct(translation, m_pickedActorPlaneSource->GetNormal()));

    updatePlanes();
    updateControls();

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];
}

void QMPRExtension::pushAxialViewAxisActor()
{
    m_axial2DView->setCursor(Qt::ClosedHandCursor);
    double clickedWorldPoint[3];
    m_axial2DView->getEventWorldCoordinate(clickedWorldPoint);
    // Get the motion vector
    double v[3];
    v[0] = clickedWorldPoint[0] - m_initialPickX;
    v[1] = clickedWorldPoint[1] - m_initialPickY;
    v[2] = 0.0;

    m_pickedActorPlaneSource->Push(MathTools::dotProduct(v, m_pickedActorPlaneSource->GetNormal()));
    updatePlanes();
    updateControls();

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];
}

void QMPRExtension::detectPushSagitalViewAxisActor()
{
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate(clickedWorldPoint);
    // We detect which is the closest actor, identify it and then leave it as selected
    // We will only move the axial view. From the sagittal point of view we will not be able to move the slice of the coronal
    double point[3] = { clickedWorldPoint[0], clickedWorldPoint[1], 0.0 };
    double *r1, *r2;
    double distanceToAxial, distanceToCoronal;
    double dummyPoint[3];

    r1 = m_axialOverSagitalIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_axialOverSagitalIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToAxial = MathTools::getPointToFiniteLineDistance(point, r1, r2, dummyPoint);

    r1 = m_coronalOverSagitalIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_coronalOverSagitalIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToCoronal = MathTools::getPointToFiniteLineDistance(point, r1, r2, dummyPoint);

    // We pass distances to view coordinates to control tolerance
    double zeroInDisplay[3];
    m_sagital2DView->computeWorldToDisplay(0.0, 0.0, 0.0, zeroInDisplay);
    double distanceToCoronalInDisplay[3];
    m_sagital2DView->computeWorldToDisplay(distanceToCoronal, 0.0, 0.0, distanceToCoronalInDisplay);
    distanceToCoronal = MathTools::getDistance3D(zeroInDisplay, distanceToCoronalInDisplay);
    double distanceToAxialInDisplay[3];
    m_sagital2DView->computeWorldToDisplay(distanceToAxial, 0.0, 0.0, distanceToAxialInDisplay);
    distanceToAxial = MathTools::getDistance3D(zeroInDisplay, distanceToAxialInDisplay);

    // We give a minimum “tolerance”
    if (distanceToCoronal < PickingDistanceThreshold || distanceToAxial < PickingDistanceThreshold)
    {
        m_sagital2DView->setCursor(Qt::OpenHandCursor);
        // We disable the tools so that they do not interfere
        m_toolManager->disableAllToolsTemporarily();
        if (distanceToCoronal < distanceToAxial)
        {
            m_pickedActorPlaneSource = m_coronalPlaneSource;
            m_pickedActorReslice = m_coronalReslice;
        }
        else
        {
            m_pickedActorPlaneSource = m_axialPlaneSource;
            m_pickedActorReslice = m_sagitalReslice;
        }
        m_state = Pushing;
        m_initialPickX = clickedWorldPoint[0];
        m_initialPickY = clickedWorldPoint[1];
    }
}

void QMPRExtension::pushSagitalViewAxialAxisActor()
{
    m_sagital2DView->setCursor(Qt::ClosedHandCursor);

    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate(clickedWorldPoint);

    m_axial2DView->setSlice(m_axial2DView->getMaximumSlice() - static_cast<int>(clickedWorldPoint[1] / m_axialSpacing[2]));
    updatePlanes();
    updateControls();

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];
}

void QMPRExtension::setInput(Volume *input)
{
	Volume *selVolume = QViewer::selectVolume();
	if (selVolume)
	{
		input = selVolume;
	}
	else if (!input)
	{
		return;
	}
    // HACK To make universal scrolling work properly. Issue #2019.
    if (input->objectName() == "Dummy Volume")
    {
        return;
    }
    // HACK End
    
    if (input->getNumberOfPhases() > 1)
    {
        m_phasesAlertLabel->setVisible(true);
    }
    else
    {
        m_phasesAlertLabel->setVisible(false);
    }

    vtkImageChangeInformation *changeInfo = vtkImageChangeInformation::New();
    changeInfo->SetInputData(input->getVtkData());
    changeInfo->SetOutputOrigin(.0, .0, .0);
    changeInfo->Update();

    // TODO Es crea un nou volum cada cop!
    m_volume = new Volume;
    m_volume->setImages(input->getImages());
    m_volume->setData(changeInfo->GetOutput());
    m_volume->setNumberOfPhases(input->getNumberOfPhases());
    m_volume->setNumberOfSlicesPerPhase(input->getNumberOfSlicesPerPhase());
    // We need to tell you the identifier in the volume repository for
    // such that when we show the patient menu we are shown in bold the current volume
    // This is still a HACK that will cease to exist when we do not transform it
    // initial source to volume when we have the new MPR module ready
    m_volume->setIdentifier(input->getIdentifier());

    m_volume->getSpacing(m_axialSpacing);

    if (m_sagitalReslice)
    {
        m_sagitalReslice->Delete();
    }
    m_sagitalReslice = vtkImageReslice::New();
    //So that the output extent is sufficient and no data is "eaten"
    m_sagitalReslice->AutoCropOutputOn();
    m_sagitalReslice->SetInterpolationModeToCubic();
    m_sagitalReslice->SetInputData(m_volume->getVtkData());

    if (m_coronalReslice)
    {
        m_coronalReslice->Delete();
    }
    m_coronalReslice = vtkImageReslice::New();
    m_coronalReslice->AutoCropOutputOn();
    m_coronalReslice->SetInterpolationModeToCubic();
    m_coronalReslice->SetInputData(m_volume->getVtkData());

    // Faltaria refrescar l'input dels 3 mpr
    // HACK To make universal scrolling work properly. Issue #2019. We have to disconnect and reconnect the signal to avoid infinite loops
    disconnect(m_axial2DView, SIGNAL(volumeChanged(Volume*)), this, SLOT(setInput(Volume*)));
    m_axial2DView->setInput(m_volume);
    connect(m_axial2DView, SIGNAL(volumeChanged(Volume*)), SLOT(setInput(Volume*)));
    int extent[6];
    m_volume->getExtent(extent);
    m_axialSlider->setMaximum(extent[5]);

    double maxThickSlab = sqrt((m_axialSpacing[0] * extent[1]) * (m_axialSpacing[0] * extent[1]) + (m_axialSpacing[1] * extent[3]) *
            (m_axialSpacing[1] * extent[3]) + (m_axialSpacing[2] * extent[5]) * (m_axialSpacing[2] * extent[5]));
    m_thickSlabSlider->setMaximum((int) maxThickSlab);
    m_thickSlabSpinBox->setMaximum(maxThickSlab);

    // Tuning planeSource
    initOrientation();

    Volume *sagitalResliced = new Volume;
    // TODO This is necessary so that you have the information of the series, studies, patient ...
    sagitalResliced->setImages(m_volume->getImages());
    sagitalResliced->setData(m_sagitalReslice->GetOutput());
    sagitalResliced->setNumberOfPhases(1);
    sagitalResliced->setNumberOfSlicesPerPhase(1);

    m_sagital2DView->setInput(sagitalResliced);

    Volume *coronalResliced = new Volume;
    // TODO This is necessary so that you have the information of the series, studies, patient ...
    coronalResliced->setImages(m_volume->getImages());
    coronalResliced->setData(m_coronalReslice->GetOutput());
    coronalResliced->setNumberOfPhases(1);
    coronalResliced->setNumberOfSlicesPerPhase(1);

    m_coronal2DView->setInput(coronalResliced);

    m_sagital2DView->render();
    m_coronal2DView->render();
}

void QMPRExtension::initOrientation()
{
    // IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Now we give each plane some dimensions, extent, spacing, etc. according as if these planes should be orthogonal, but according to the cutting plane
    // it will be one way or another
    // The axial view will keep the original spaced and extended
    // The sagittal view, as it can only be rotated on the Y axis, will keep the extent of its X the same as the original Y extent. The others
    // must be adapted to the distances corresponding to the diagonals zmax-xmax
    // In the coronal view, as it can have any orientation you will have to adapt its extensions to the maximums

    int extent[6];
    m_volume->getExtent(extent);
    int extentLength[3] = { extent[1] - extent[0] + 1, extent[3] - extent[2] + 1, extent[5] - extent[4] + 1 };
    double origin[3];
    m_volume->getOrigin(origin);
    double spacing[3];
    m_volume->getSpacing(spacing);

    //Prevent obscuring voxels by offsetting the plane geometry
    double xbounds[] = { origin[0] + spacing[0] * (extent[0] - 0.5),
                         origin[0] + spacing[0] * (extent[1] + 0.5) };
    double ybounds[] = { origin[1] + spacing[1] * (extent[2] - 0.5),
                         origin[1] + spacing[1] * (extent[3] + 0.5) };
    double zbounds[] = { origin[2] + spacing[2] * (extent[4] - 0.5),
                         origin[2] + spacing[2] * (extent[5] + 0.5) };

    if (spacing[0] < 0.0)
    {
        double t = xbounds[0];
        xbounds[0] = xbounds[1];
        xbounds[1] = t;
    }
    if (spacing[1] < 0.0)
    {
        double t = ybounds[0];
        ybounds[0] = ybounds[1];
        ybounds[1] = t;
    }
    if (spacing[2] < 0.0)
    {
        double t = zbounds[0];
        zbounds[0] = zbounds[1];
        zbounds[1] = t;
    }

    double volumeSize[3] = { xbounds[1] - xbounds[0], ybounds[1] - ybounds[0], zbounds[1] - zbounds[0] };

    // XY, z-normal : vista axial, en principi d'aquesta vista nomès canviarem la llesca
    m_axialPlaneSource->SetOrigin(xbounds[0], ybounds[0], zbounds[0]);
    m_axialPlaneSource->SetPoint1(xbounds[1], ybounds[0], zbounds[0]);
    m_axialPlaneSource->SetPoint2(xbounds[0], ybounds[1], zbounds[0]);
    m_axialZeroSliceCoordinate = zbounds[0];

    // YZ, x-normal: sagittal sight
    // We are adjusting the size of the plane to the dimensions of this orientation
    // The size of the initial Y, which will be a combination of X and Y during execution, must be the diagonal of the XY plane. We will enlarge half on each side
    // about the size of Y.
    // Warning: we are assuming that xbounds [0] = 0. The correct form would be (xbounds [1] - xbounds [0] (+1?)). The same for y.
    double xyDiagonal = sqrt(volumeSize[0] * volumeSize[0] + volumeSize[1] * volumeSize[1]);
    double halfDeltaY = (xyDiagonal - volumeSize[1]) * 0.5;
    m_sagitalPlaneSource->SetOrigin(xbounds[0], ybounds[0] - halfDeltaY, zbounds[1]);
    m_sagitalPlaneSource->SetPoint1(xbounds[0], ybounds[1] + halfDeltaY, zbounds[1]);
    m_sagitalPlaneSource->SetPoint2(xbounds[0], ybounds[0] - halfDeltaY, zbounds[0]);
    m_sagitalPlaneSource->Push(-0.5 * volumeSize[0]);
    //We calculate the translation required to draw the intersections of the planes in the sagittal view
    m_sagitalTranslation[0] = m_sagitalPlaneSource->GetCenter()[1] + halfDeltaY;
    m_sagitalTranslation[1] = m_sagitalPlaneSource->GetCenter()[2];
    m_sagitalTranslation[2] = 0.0;
    //We calculate the extent of the sagittal
    double sagitalExtentLengthX = sqrt(static_cast<double>(extentLength[0] * extentLength[0] + extentLength[1] * extentLength[1]));
    ///sagitalExtentLengthX * = 2.0; //
    ///  maybe we should double the extent to ensure that no detail is lost (Nyquist)
    m_sagitalExtentLength[0] = MathTools::roundUpToPowerOf2(MathTools::roundToNearestInteger(sagitalExtentLengthX));
    m_sagitalExtentLength[1] = extentLength[2];

    /// ZX, y-normal: coronal view
    /// same as above
    /// The size of the initial X and Z, which will be a combination of
    /// X, Y, and Z during execution, must be the diagonal of the volume. We will expand half a
    /// each band about the size of the X and Z axes.
    /// Warning: we are assuming that xbounds [0] = 0.
    /// The correct form would be (xbounds [1] - xbounds [0] (+1?)). The same for y and z.
    double diagonal = sqrt(volumeSize[0] * volumeSize[0] + volumeSize[1] * volumeSize[1] + volumeSize[2] * volumeSize[2]);
    double halfDeltaX = (diagonal - volumeSize[0]) * 0.5;
    double halfDeltaZ = (diagonal - volumeSize[2]) * 0.5;
    m_coronalPlaneSource->SetOrigin(xbounds[0] - halfDeltaX, ybounds[0], zbounds[1] + halfDeltaZ);
    m_coronalPlaneSource->SetPoint1(xbounds[1] + halfDeltaX, ybounds[0], zbounds[1] + halfDeltaZ);
    m_coronalPlaneSource->SetPoint2(xbounds[0] - halfDeltaX, ybounds[0], zbounds[0] - halfDeltaZ);
    m_coronalPlaneSource->Push(0.5 * volumeSize[1]);
    // We calculate the extent of the coronal
    double coronalExtentLength = sqrt(static_cast<double>(extentLength[0] * extentLength[0] + extentLength[1] * extentLength[1] + extentLength[2] *
            extentLength[2]));
    ///coronalExtentLength * = 2.0; //
    ///  maybe we should double the extent to ensure that no detail is lost (Nyquist)
    m_coronalExtentLength[0] = MathTools::roundUpToPowerOf2(MathTools::roundToNearestInteger(coronalExtentLength));
    m_coronalExtentLength[1] = m_coronalExtentLength[0];

    // We put the sizes of the drawer points
    const double RadiusFactor = 0.01;
    double o[3], p1[3], p2[3];
    double width, height;
    double radius;
    // Axials
    m_axialPlaneSource->GetOrigin(o);
    m_axialPlaneSource->GetPoint1(p1);
    m_axialPlaneSource->GetPoint2(p2);
    width = MathTools::getDistance3D(o, p1);
    height = MathTools::getDistance3D(o, p2);
    radius = RadiusFactor * qMax(width, height);
    m_axialViewSagitalCenterDrawerPoint->setRadius(radius);
    m_axialViewCoronalCenterDrawerPoint->setRadius(radius);
    // Sagitals
    m_sagitalPlaneSource->GetOrigin(o);
    m_sagitalPlaneSource->GetPoint1(p1);
    m_sagitalPlaneSource->GetPoint2(p2);
    width = MathTools::getDistance3D(o, p1);
    height = MathTools::getDistance3D(o, p2);
    radius = RadiusFactor * qMax(width, height);
    m_sagitalViewAxialCenterDrawerPoint->setRadius(radius);
    m_sagitalViewCoronalCenterDrawerPoint->setRadius(radius);

    updatePlanes();
    updateControls();
}

void QMPRExtension::createActors()
{
    QColor axialColor = QColor::fromRgbF(1.0, 1.0, 0.0);
    QColor sagitalColor = QColor::fromRgbF(1.0, 0.6, 0.0);
    QColor coronalColor = QColor::fromRgbF(0.0, 1.0, 1.0);

    // We create the axis actors
    m_sagitalOverAxialAxisActor = vtkAxisActor2D::New();
    m_coronalOverAxialIntersectionAxis = vtkAxisActor2D::New();
    m_coronalOverSagitalIntersectionAxis = vtkAxisActor2D::New();
    m_axialOverSagitalIntersectionAxis = vtkAxisActor2D::New();
    m_thickSlabOverAxialActor = vtkAxisActor2D::New();
    m_thickSlabOverSagitalActor = vtkAxisActor2D::New();

    m_sagitalOverAxialAxisActor->AxisVisibilityOn();
    m_sagitalOverAxialAxisActor->TickVisibilityOff();
    m_sagitalOverAxialAxisActor->LabelVisibilityOff();
    m_sagitalOverAxialAxisActor->TitleVisibilityOff();
    m_sagitalOverAxialAxisActor->GetProperty()->SetColor(sagitalColor.redF(), sagitalColor.greenF(), sagitalColor.blueF());

    m_coronalOverAxialIntersectionAxis->TickVisibilityOff();
    m_coronalOverAxialIntersectionAxis->LabelVisibilityOff();
    m_coronalOverAxialIntersectionAxis->TitleVisibilityOff();
    m_coronalOverAxialIntersectionAxis->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());

    m_coronalOverSagitalIntersectionAxis->AxisVisibilityOn();
    m_coronalOverSagitalIntersectionAxis->TickVisibilityOff();
    m_coronalOverSagitalIntersectionAxis->LabelVisibilityOff();
    m_coronalOverSagitalIntersectionAxis->TitleVisibilityOff();
    m_coronalOverSagitalIntersectionAxis->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());

    m_axialOverSagitalIntersectionAxis->AxisVisibilityOn();
    m_axialOverSagitalIntersectionAxis->TickVisibilityOff();
    m_axialOverSagitalIntersectionAxis->LabelVisibilityOff();
    m_axialOverSagitalIntersectionAxis->TitleVisibilityOff();
    m_axialOverSagitalIntersectionAxis->GetProperty()->SetColor(axialColor.redF(), axialColor.greenF(), axialColor.blueF());

    ///For now the thickslab lines will be invisible as we cannot do MIPs
    /// and being superimposed on the lines of the planes have a bad effect
    m_thickSlabOverAxialActor->AxisVisibilityOff();
    m_thickSlabOverAxialActor->TickVisibilityOff();
    m_thickSlabOverAxialActor->LabelVisibilityOff();
    m_thickSlabOverAxialActor->TitleVisibilityOff();
    m_thickSlabOverAxialActor->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());
    m_thickSlabOverAxialActor->GetProperty()->SetLineStipplePattern(65280);

    m_thickSlabOverSagitalActor->AxisVisibilityOff();
    m_thickSlabOverSagitalActor->TickVisibilityOff();
    m_thickSlabOverSagitalActor->LabelVisibilityOff();
    m_thickSlabOverSagitalActor->TitleVisibilityOff();
    m_thickSlabOverSagitalActor->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());
    m_thickSlabOverSagitalActor->GetProperty()->SetLineStipplePattern(65280);

    m_axial2DView->getRenderer()->AddViewProp(m_sagitalOverAxialAxisActor);
    m_axial2DView->getRenderer()->AddViewProp(m_coronalOverAxialIntersectionAxis);
    m_axial2DView->getRenderer()->AddViewProp(m_thickSlabOverAxialActor);
    m_sagital2DView->getRenderer()->AddViewProp(m_coronalOverSagitalIntersectionAxis);
    m_sagital2DView->getRenderer()->AddViewProp(m_axialOverSagitalIntersectionAxis);
    m_sagital2DView->getRenderer()->AddViewProp(m_thickSlabOverSagitalActor);

    // We create the drawer points

    m_axialViewSagitalCenterDrawerPoint = new DrawerPoint();
    m_axialViewSagitalCenterDrawerPoint->increaseReferenceCount();
    m_axialViewSagitalCenterDrawerPoint->setColor(sagitalColor);

    m_axialViewCoronalCenterDrawerPoint = new DrawerPoint();
    m_axialViewCoronalCenterDrawerPoint->increaseReferenceCount();
    m_axialViewCoronalCenterDrawerPoint->setColor(coronalColor);

    m_sagitalViewAxialCenterDrawerPoint = new DrawerPoint();
    m_sagitalViewAxialCenterDrawerPoint->increaseReferenceCount();
    m_sagitalViewAxialCenterDrawerPoint->setColor(axialColor);

    m_sagitalViewCoronalCenterDrawerPoint = new DrawerPoint();
    m_sagitalViewCoronalCenterDrawerPoint->increaseReferenceCount();
    m_sagitalViewCoronalCenterDrawerPoint->setColor(coronalColor);
}

void QMPRExtension::axialSliceUpdated(int slice)
{
    // Relative push we have to do = relocate to the start and place the slice
    m_axialPlaneSource->Push(m_axialZeroSliceCoordinate - m_axialPlaneSource->GetOrigin()[2] + slice * m_axialSpacing[2]);
    m_axialPlaneSource->Update();
    updateControls();
}

void QMPRExtension::updateControls()
{
    // Let's move on to the world coordinate system
    m_sagitalOverAxialAxisActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_sagitalOverAxialAxisActor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_coronalOverSagitalIntersectionAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_coronalOverSagitalIntersectionAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_coronalOverAxialIntersectionAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_coronalOverAxialIntersectionAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_axialOverSagitalIntersectionAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_axialOverSagitalIntersectionAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_thickSlabOverAxialActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_thickSlabOverAxialActor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_thickSlabOverSagitalActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_thickSlabOverSagitalActor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_thickSlabPlaneSource->SetOrigin(m_coronalPlaneSource->GetOrigin());
    m_thickSlabPlaneSource->SetPoint1(m_coronalPlaneSource->GetPoint1());
    m_thickSlabPlaneSource->SetPoint2(m_coronalPlaneSource->GetPoint2());
    m_thickSlabPlaneSource->Push(m_thickSlab);

    //We obtain the transformation to pass from mona coordinates to sagittal coordinates
    vtkTransform *worldToSagitalTransform = getWorldToSagitalTransform();

    //We calculate the intersections

    double r[3], t[3], position1[3], position2[3];
    const double Length = 2000.0;

    //Sagittal projection on axial and vice versa
    MathTools::planeIntersection(m_axialPlaneSource->GetOrigin(), m_axialPlaneSource->GetNormal(), m_sagitalPlaneSource->GetOrigin(),
                                 m_sagitalPlaneSource->GetNormal(), r, t);
    // We normalize t so that we always have the same length (1)
    MathTools::normalize(t);

    position1[0] = r[0] - t[0] * Length;
    position1[1] = r[1] - t[1] * Length;
    position1[2] = r[2] - t[2] * Length;

    position2[0] = r[0] + t[0] * Length;
    position2[1] = r[1] + t[1] * Length;
    position2[2] = r[2] + t[2] * Length;

    m_sagitalOverAxialAxisActor->SetPosition(position1[0], position1[1]);
    m_sagitalOverAxialAxisActor->SetPosition2(position2[0], position2[1]);

    worldToSagitalTransform->TransformPoint(position1, position1);
    worldToSagitalTransform->TransformPoint(position2, position2);
    m_axialOverSagitalIntersectionAxis->SetPosition(position1[0], position1[1]);
    m_axialOverSagitalIntersectionAxis->SetPosition2(position2[0], position2[1]);

    // Coronal projection on sagittal

    MathTools::planeIntersection(m_coronalPlaneSource->GetOrigin(), m_coronalPlaneSource->GetNormal(), m_sagitalPlaneSource->GetOrigin(),
                                 m_sagitalPlaneSource->GetNormal(), r, t);
    //We normalize t so that we always have the same length (1)
    MathTools::normalize(t);

    position1[0] = r[0] - t[0] * Length;
    position1[1] = r[1] - t[1] * Length;
    position1[2] = r[2] - t[2] * Length;

    position2[0] = r[0] + t[0] * Length;
    position2[1] = r[1] + t[1] * Length;
    position2[2] = r[2] + t[2] * Length;

    worldToSagitalTransform->TransformPoint(position1, position1);
    worldToSagitalTransform->TransformPoint(position2, position2);
    m_coronalOverSagitalIntersectionAxis->SetPosition(position1[0], position1[1]);
    m_coronalOverSagitalIntersectionAxis->SetPosition2(position2[0], position2[1]);

    // Thick slab projection on sagittal
    MathTools::planeIntersection(m_thickSlabPlaneSource->GetOrigin(), m_thickSlabPlaneSource->GetNormal(), m_sagitalPlaneSource->GetOrigin(),
                                 m_sagitalPlaneSource->GetNormal(), r, t);
    //We normalize t so that we always have the same length (1)
    MathTools::normalize(t);

    position1[0] = r[0] - t[0] * Length;
    position1[1] = r[1] - t[1] * Length;
    position1[2] = r[2] - t[2] * Length;

    position2[0] = r[0] + t[0] * Length;
    position2[1] = r[1] + t[1] * Length;
    position2[2] = r[2] + t[2] * Length;

    worldToSagitalTransform->TransformPoint(position1, position1);
    worldToSagitalTransform->TransformPoint(position2, position2);
    m_thickSlabOverSagitalActor->SetPosition(position1[0], position1[1]);
    m_thickSlabOverSagitalActor->SetPosition2(position2[0], position2[1]);

    // Coronal projection on axial
    MathTools::planeIntersection(m_coronalPlaneSource->GetOrigin(), m_coronalPlaneSource->GetNormal(), m_axialPlaneSource->GetOrigin(),
                                 m_axialPlaneSource->GetNormal(), r, t);
    // We normalize t so that we always have the same length (1)
    MathTools::normalize(t);

    position1[0] = r[0] - t[0] * Length;
    position1[1] = r[1] - t[1] * Length;
    position1[2] = r[2] - t[2] * Length;

    position2[0] = r[0] + t[0] * Length;
    position2[1] = r[1] + t[1] * Length;
    position2[2] = r[2] + t[2] * Length;

    m_coronalOverAxialIntersectionAxis->SetPosition(position1[0], position1[1]);
    m_coronalOverAxialIntersectionAxis->SetPosition2(position2[0], position2[1]);

    //Thick slab projection on axial
    MathTools::planeIntersection(m_thickSlabPlaneSource->GetOrigin(), m_thickSlabPlaneSource->GetNormal(), m_axialPlaneSource->GetOrigin(),
                                 m_axialPlaneSource->GetNormal(), r, t);
    // We normalize t so that we always have the same length (1)
    MathTools::normalize(t);

    position1[0] = r[0] - t[0] * Length;
    position1[1] = r[1] - t[1] * Length;
    position1[2] = r[2] - t[2] * Length;

    position2[0] = r[0] + t[0] * Length;
    position2[1] = r[1] + t[1] * Length;
    position2[2] = r[2] + t[2] * Length;

    m_thickSlabOverAxialActor->SetPosition(position1[0], position1[1]);
    m_thickSlabOverAxialActor->SetPosition2(position2[0], position2[1]);

    //We place the drawer points

    double center[3];

    m_sagitalPlaneSource->GetCenter(center);
    center[2] = 0.0;
    m_axialViewSagitalCenterDrawerPoint->setPosition(center);
    m_axialViewSagitalCenterDrawerPoint->update();
    m_axial2DView->getDrawer()->draw(m_axialViewSagitalCenterDrawerPoint);

    m_coronalPlaneSource->GetCenter(center);
    center[2] = 0.0;
    m_axialViewCoronalCenterDrawerPoint->setPosition(center);
    m_axialViewCoronalCenterDrawerPoint->update();
    m_axial2DView->getDrawer()->draw(m_axialViewCoronalCenterDrawerPoint);

    worldToSagitalTransform->TransformPoint(m_axialPlaneSource->GetCenter(), center);
    center[2] = 0.0;
    m_sagitalViewAxialCenterDrawerPoint->setPosition(center);
    m_sagitalViewAxialCenterDrawerPoint->update();
    m_sagital2DView->getDrawer()->draw(m_sagitalViewAxialCenterDrawerPoint);

    worldToSagitalTransform->TransformPoint(m_coronalPlaneSource->GetCenter(), center);
    center[2] = 0.0;
    m_sagitalViewCoronalCenterDrawerPoint->setPosition(center);
    m_sagitalViewCoronalCenterDrawerPoint->update();
    m_sagital2DView->getDrawer()->draw(m_sagitalViewCoronalCenterDrawerPoint);

    worldToSagitalTransform->Delete();

    // Let's repaint the scene
    m_axial2DView->render();
    m_sagital2DView->render();
    m_coronal2DView->render();
}

void QMPRExtension::updatePlanes()
{
    updatePlane(m_sagitalPlaneSource, m_sagitalReslice, m_sagitalExtentLength);
    updatePlane(m_coronalPlaneSource, m_coronalReslice, m_coronalExtentLength);
}

void QMPRExtension::updatePlane(vtkPlaneSource *planeSource, vtkImageReslice *reslice, int extentLength[2])
{
    if (!reslice || !(vtkImageData::SafeDownCast(reslice->GetInput())))
    {
        return;
    }

    //Calculate appropriate pixel spacing for the reslicing
    double spacing[3];
    m_volume->getSpacing(spacing);

    int i;

    //     if (this->RestrictPlaneToVolume)
    //     {
    double origin[3];
    m_volume->getOrigin(origin);

    int extent[6];
    m_volume->getExtent(extent);

    // The order of the data is xmin, xmax, ymin, ymax, zmin and zmax
    double bounds[] = { origin[0] + spacing[0] * extent[0],
                        origin[0] + spacing[0] * extent[1],
                        origin[1] + spacing[1] * extent[2],
                        origin[1] + spacing[1] * extent[3],
                        origin[2] + spacing[2] * extent[4],
                        origin[2] + spacing[2] * extent[5] };

    // Reverse bounds if necessary
    for (i = 0; i <= 4; i += 2)
    {
        if (bounds[i] > bounds[i + 1])
        {
            double t = bounds[i + 1];
            bounds[i + 1] = bounds[i];
            bounds[i] = t;
        }
    }

    double abs_normal[3];
    planeSource->GetNormal(abs_normal);

    double planeCenter[3];
    planeSource->GetCenter(planeCenter);

    double nmax = 0.0;
    int k = 0;
    for (i = 0; i < 3; i++)
    {
        abs_normal[i] = fabs(abs_normal[i]);
        if (abs_normal[i]>nmax)
        {
            nmax = abs_normal[i];
            k = i;
        }
    }
    // Force the plane to lie within the true image bounds along its normal
    if (planeCenter[k] > bounds[2 * k + 1])
    {
        planeCenter[k] = bounds[2 * k + 1];
    }
    else if (planeCenter[k] < bounds[2 * k])
    {
        planeCenter[k] = bounds[2 * k];
    }
    planeSource->SetCenter(planeCenter);
    //     }

    double planeAxis1[3];
    double planeAxis2[3];
    // We get the vectors
    planeAxis1[0] = planeSource->GetPoint1()[0] - planeSource->GetOrigin()[0];
    planeAxis1[1] = planeSource->GetPoint1()[1] - planeSource->GetOrigin()[1];
    planeAxis1[2] = planeSource->GetPoint1()[2] - planeSource->GetOrigin()[2];

    planeAxis2[0] = planeSource->GetPoint2()[0] - planeSource->GetOrigin()[0];
    planeAxis2[1] = planeSource->GetPoint2()[1] - planeSource->GetOrigin()[1];
    planeAxis2[2] = planeSource->GetPoint2()[2] - planeSource->GetOrigin()[2];

    //The x,y dimensions of the plane
    double planeSizeX = MathTools::normalize(planeAxis1);
    double planeSizeY = MathTools::normalize(planeAxis2);

    double normal[3];
    planeSource->GetNormal(normal);

    //Generate the slicing matrix
    //
    // It could be a class member, as it was originally, or pass as a parameter
    vtkMatrix4x4 *resliceAxes = vtkMatrix4x4::New();
    resliceAxes->Identity();
    for (i = 0; i < 3; i++)
    {
        resliceAxes->SetElement(0, i, planeAxis1[i]);
        resliceAxes->SetElement(1, i, planeAxis2[i]);
        resliceAxes->SetElement(2, i, normal[i]);
    }

    double planeOrigin[4];
    planeSource->GetOrigin(planeOrigin);
    planeOrigin[3] = 1.0;
    double originXYZW[4];
    resliceAxes->MultiplyPoint(planeOrigin, originXYZW);

    resliceAxes->Transpose();
    double neworiginXYZW[4];
    double point[] = { originXYZW[0], originXYZW[1], originXYZW[2], originXYZW[3] };
    resliceAxes->MultiplyPoint(point, neworiginXYZW);

    resliceAxes->SetElement(0, 3, neworiginXYZW[0]);
    resliceAxes->SetElement(1, 3, neworiginXYZW[1]);
    resliceAxes->SetElement(2, 3, neworiginXYZW[2]);

    reslice->SetResliceAxes(resliceAxes);

    resliceAxes->Delete();

    reslice->SetOutputSpacing(planeSizeX / extentLength[0], planeSizeY / extentLength[1], 1.0);
    reslice->SetOutputOrigin(0.0, 0.0, 0.0);
    // TODO We pass it thickSlab which is double but this only accepts int's! Find out if this is the right way. Maybe if we want to use doubles
    // we should combine it with outputSpacing
    // We get a single slice
    reslice->SetOutputExtent(0, extentLength[0] - 1, 0, extentLength[1] - 1, 0, static_cast<int>(m_thickSlab));
    reslice->Update();
}

void QMPRExtension::getSagitalXVector(double x[3])
{
    double *p1 = m_sagitalPlaneSource->GetPoint1();
    double *o = m_sagitalPlaneSource->GetOrigin();
    x[0] = p1[0] - o[0];
    x[1] = p1[1] - o[1];
    x[2] = p1[2] - o[2];
}

void QMPRExtension::getSagitalYVector(double y[3])
{
    double *p1 = m_sagitalPlaneSource->GetPoint2();
    double *o = m_sagitalPlaneSource->GetOrigin();
    y[0] = p1[0] - o[0];
    y[1] = p1[1] - o[1];
    y[2] = p1[2] - o[2];
}

void QMPRExtension::getCoronalXVector(double x[3])
{
    double *p1 = m_coronalPlaneSource->GetPoint1();
    double *o = m_coronalPlaneSource->GetOrigin();
    x[0] = p1[0] - o[0];
    x[1] = p1[1] - o[1];
    x[2] = p1[2] - o[2];
}

void QMPRExtension::getAxialXVector(double x[3])
{
    double *p1 = m_axialPlaneSource->GetPoint1();
    double *o = m_axialPlaneSource->GetOrigin();
    x[0] = p1[0] - o[0];
    x[1] = p1[1] - o[1];
    x[2] = p1[2] - o[2];
}

void QMPRExtension::getAxialYVector(double y[3])
{
    double *p2 = m_axialPlaneSource->GetPoint2();
    double *o = m_axialPlaneSource->GetOrigin();
    y[0] = p2[0] - o[0];
    y[1] = p2[1] - o[1];
    y[2] = p2[2] - o[2];
}
void QMPRExtension::getCoronalYVector(double y[3])
{
    double *p1 = m_coronalPlaneSource->GetPoint2();
    double *o = m_coronalPlaneSource->GetOrigin();
    y[0] = p1[0] - o[0];
    y[1] = p1[1] - o[1];
    y[2] = p1[2] - o[2];
}

bool QMPRExtension::isParallel(double axis[3])
{
    QVector3D xyzAxis(1, 0, 0);
    QVector3D axis3D(axis[0], axis[1], axis[2]);
    // TODO We should have a MathTools :: areParallel method (vector1, vector2)
    if (MathTools::angleInDegrees(xyzAxis, axis3D) == 0.0)
    {
        return true;
    }
    xyzAxis.setX(-1);
    xyzAxis.setY(0);
    xyzAxis.setZ(0);
    if (MathTools::angleInDegrees(xyzAxis, axis3D) == 0.0)
    {
        return true;
    }

    xyzAxis.setX(0);
    xyzAxis.setY(0);
    xyzAxis.setZ(1);
    if (MathTools::angleInDegrees(xyzAxis, axis3D) == 0.0)
    {
        return true;
    }
    xyzAxis.setX(0);
    xyzAxis.setY(0);
    xyzAxis.setZ(-1);
    if (MathTools::angleInDegrees(xyzAxis, axis3D) == 0.0)
    {
        return true;
    }

    xyzAxis.setX(0);
    xyzAxis.setY(1);
    xyzAxis.setZ(0);
    if (MathTools::angleInDegrees(xyzAxis, axis3D) == 0.0)
    {
        return true;
    }
    xyzAxis.setX(0);
    xyzAxis.setY(-1);
    xyzAxis.setZ(0);
    if (MathTools::angleInDegrees(xyzAxis, axis3D) == 0.0)
    {
        return true;
    }

    return false;
}

void QMPRExtension::rotateMiddle(double degrees, double rotationAxis[3], vtkPlaneSource *plane)
{
    //     MathTools::normalize(rotationAxis);
    m_transform->Identity();
    m_transform->Translate(plane->GetCenter()[0], plane->GetCenter()[1], plane->GetCenter()[2]);
    m_transform->RotateWXYZ(degrees, rotationAxis);
    m_transform->Translate(-plane->GetCenter()[0], -plane->GetCenter()[1], -plane->GetCenter()[2]);
    //Now that we have the transformation, we apply it to the points of the plane (origin, point1, point2)
    double newPoint[3];
    m_transform->TransformPoint(plane->GetPoint1(), newPoint);
    plane->SetPoint1(newPoint);
    m_transform->TransformPoint(plane->GetPoint2(), newPoint);
    plane->SetPoint2(newPoint);
    m_transform->TransformPoint(plane->GetOrigin(), newPoint);
    plane->SetOrigin(newPoint);
    plane->Update();
}

void QMPRExtension::updateThickSlab(double value)
{
    m_thickSlab = value;
    m_thickSlabSlider->setValue((int) value);
    updatePlane(m_coronalPlaneSource, m_coronalReslice, m_coronalExtentLength);
    updateControls();
}

void QMPRExtension::updateThickSlab(int value)
{
    m_thickSlab = (double) value;
    m_thickSlabSpinBox->setValue(m_thickSlab);
    updatePlane(m_coronalPlaneSource, m_coronalReslice, m_coronalExtentLength);
    updateControls();
}

void QMPRExtension::readSettings()
{
    Settings settings;

    if (settings.getValue(MPRSettings::HorizontalSplitterGeometry).toByteArray().isEmpty())
    {
        QList<int> list;
        list << this->size().width() / 2 << this->size().width() / 2;
        m_horizontalSplitter->setSizes(list);
    }
    else
    {
        settings.restoreGeometry(MPRSettings::HorizontalSplitterGeometry, m_horizontalSplitter);
    }

    if (settings.getValue(MPRSettings::VerticalSplitterGeometry).toByteArray().isEmpty())
    {
        QList<int> list;
        list << this->size().height() / 2 << this->size().height() / 2;
        m_verticalSplitter->setSizes(list);
    }
    else
    {
        settings.restoreGeometry(MPRSettings::VerticalSplitterGeometry, m_verticalSplitter);
    }
}

void QMPRExtension::writeSettings()
{
    Settings settings;

    settings.saveGeometry(MPRSettings::HorizontalSplitterGeometry, m_horizontalSplitter);
    settings.saveGeometry(MPRSettings::VerticalSplitterGeometry, m_verticalSplitter);
}

vtkTransform* QMPRExtension::getWorldToSagitalTransform() const
{
    double sagitalPlaneAxis1[3];
    double sagitalPlaneAxis2[3];
    double sagitalPlaneNormal[3];
    double sagitalPlaneCenter[3];
    sagitalPlaneAxis1[0] = m_sagitalPlaneSource->GetPoint1()[0] - m_sagitalPlaneSource->GetOrigin()[0];
    sagitalPlaneAxis1[1] = m_sagitalPlaneSource->GetPoint1()[1] - m_sagitalPlaneSource->GetOrigin()[1];
    sagitalPlaneAxis1[2] = m_sagitalPlaneSource->GetPoint1()[2] - m_sagitalPlaneSource->GetOrigin()[2];
    MathTools::normalize(sagitalPlaneAxis1);
    sagitalPlaneAxis2[0] = m_sagitalPlaneSource->GetPoint2()[0] - m_sagitalPlaneSource->GetOrigin()[0];
    sagitalPlaneAxis2[1] = m_sagitalPlaneSource->GetPoint2()[1] - m_sagitalPlaneSource->GetOrigin()[1];
    sagitalPlaneAxis2[2] = m_sagitalPlaneSource->GetPoint2()[2] - m_sagitalPlaneSource->GetOrigin()[2];
    MathTools::normalize(sagitalPlaneAxis2);
    m_sagitalPlaneSource->GetNormal(sagitalPlaneNormal);
    m_sagitalPlaneSource->GetCenter(sagitalPlaneCenter);

    vtkMatrix4x4 *sagitalRotationMatrix = vtkMatrix4x4::New();

    for (int i = 0; i < 3; i++)
    {
        sagitalRotationMatrix->SetElement(0, i, sagitalPlaneAxis1[i]);
        sagitalRotationMatrix->SetElement(1, i, sagitalPlaneAxis2[i]);
        sagitalRotationMatrix->SetElement(2, i, sagitalPlaneNormal[i]);
    }

    vtkTransform *sagitalTransform = vtkTransform::New();
    sagitalTransform->Translate(m_sagitalTranslation);
    sagitalTransform->Concatenate(sagitalRotationMatrix);
    sagitalTransform->Translate(-sagitalPlaneCenter[0], -sagitalPlaneCenter[1], -sagitalPlaneCenter[2]);

    sagitalRotationMatrix->Delete();

    return sagitalTransform;
}

};  // End namespace udg
