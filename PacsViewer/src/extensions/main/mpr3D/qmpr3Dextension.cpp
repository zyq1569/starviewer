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

#include "qmpr3Dextension.h"

#include "drawer.h"
#include "drawerpoint.h"
#include "logging.h"
#include "image.h"
#include "series.h"
#include "voiluthelper.h"
//By intersection calculations
#include "mathtools.h"
#include "mpr3Dsettings.h"
#include "patientbrowsermenu.h"
#include "q3dviewer.h"
#include "qexportertool.h"
#include "screenshottool.h"
#include "toolmanager.h"
#include "toolproxy.h"
#include "volume.h"
#include "voilutpresetstooldata.h"
#include "imageplane.h"

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
#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkCellPicker.h>
#include <vtkProperty.h>
//
#include <vtkResliceImageViewer.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkImagePlaneWidget.h>
#include "vtkInteractorStyleImage.h"
#include <vtkCornerAnnotation.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include <vtkCornerAnnotation.h>
#include <vtkVolume.h>
#include <vtkSmartVolumeMapper.h>

//--
#include <vtkLODProp3D.h>
#include <vtkOutlineFilter.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkVolumeProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageFlip.h>
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkImageSlabReslice.h"


using namespace std;
//
// Qt
#include <QMessageBox>
#include <QMenu>
#include <QVector3D>

class vtkResliceImageViewerScrollCallback : public vtkCommand
{
public:
	static vtkResliceImageViewerScrollCallback* New()
	{
		return new vtkResliceImageViewerScrollCallback;
	}

	void Execute(vtkObject*, unsigned long ev, void*) override
	{
		if (!this->Viewer->GetSliceScrollOnMouseWheel())
		{
			return;
		}

		// Do not process if any modifiers are ON
		if (this->Viewer->GetInteractor()->GetShiftKey() ||	this->Viewer->GetInteractor()->GetControlKey() || this->Viewer->GetInteractor()->GetAltKey())
		{
			return;
		}

		// forwards or backwards
		//int sign = (ev == vtkCommand::MouseWheelForwardEvent) ? 1 : -1;
		int sign = (ev == vtkCommand::MouseWheelForwardEvent) ? SliceInc : -SliceInc;

		this->Viewer->IncrementSlice(sign);

		//this->Viewer->SetSlice(this->Viewer->GetSlice() + static_cast<int>(std::round(sign * 1.0)));
		// Abort further event processing for the scroll.
		this->SetAbortFlag(1);
	}

	vtkResliceImageViewerScrollCallback(): Viewer(nullptr), SliceInc(1)
	{

	}
	void ChangeSliceScrollOnMouseWheel(bool ChangeSliceScrollOnMouseWheel = true)
	{
		SliceInc = (ChangeSliceScrollOnMouseWheel) ? -1 : 1;
	}
	vtkResliceImageViewer* Viewer;
	int SliceInc;
};
class vtkMPRResliceImageViewer :public vtkResliceImageViewer
{
public:
	static vtkMPRResliceImageViewer* New();
	vtkTypeMacro(vtkMPRResliceImageViewer, vtkResliceImageViewer);

	void  setScrollCallback(vtkResliceImageViewerScrollCallback* call , vtkResliceImageViewer* viewer)
	{
		this->ScrollCallback = vtkResliceImageViewerScrollCallback::New();
		this->ScrollCallback->Viewer = viewer;
		this->ScrollCallback->ChangeSliceScrollOnMouseWheel(true);
	}
	void init(bool ChangeSliceScrollOnMouseWheel = false)
	{
		this->ScrollCallback = vtkResliceImageViewerScrollCallback::New();
		this->ScrollCallback->Viewer = this;
		this->ScrollCallback->ChangeSliceScrollOnMouseWheel(ChangeSliceScrollOnMouseWheel);
	}
protected:
	vtkMPRResliceImageViewer()
	{
		init();
	}
	~vtkMPRResliceImageViewer() override
	{

	}
};
vtkStandardNewMacro(vtkMPRResliceImageViewer);

namespace udg {

const double PI = -3.141592653589793238462643383279502884197169399375105820974944;
double       g_SliceThickness[3] = { 0 };
QString      g_ser = 0;
QString      g_time = "";
QString      g_institutionName = "";
QString      g_dicomKVP = "";
QString      g_dicomXRayTubeCurrent = "";
const double QMPR3DExtension::PickingDistanceThreshold = 7.0;

double getSliceThickness(Volume * volume, int i)
{
	double thickness = 0.0;
	if (volume)
	{
		switch (i)
		{
		case 0://OrthogonalPlane::XYPlane:
		{
			Image *image = volume->getImage(0, 0);// this->getCurrentSlice(), this->getCurrentPhase());

			if (image)
			{
				thickness = image->getSliceThickness();
			}
		}
		break;

		case 1://OrthogonalPlane::YZPlane:
			thickness = volume->getSpacing()[0];
			break;

		case 2://OrthogonalPlane::XZPlane:
			thickness = volume->getSpacing()[1];
			break;
		}
	}
	return thickness;

}
void setCornerAnnotations(vtkCornerAnnotation* vtkCornerAnnotation, int Slice, int Window, int Level);
void setCornerAnnotations(vtkCornerAnnotation* vtkCornerAnnotation, int Slice, int Window, int Level)
{
	QString sliceInfo = QObject::tr("ims: %1\n").arg(Slice); 
	sliceInfo += QObject::tr("ser: %1 \n").arg(g_ser);
	sliceInfo += QObject::tr("WW: %1 WL: %2 \n").arg(Window).arg(Level);
	sliceInfo += QObject::tr("KV: %1 mA: %2 \n").arg(g_dicomKVP).arg(g_dicomXRayTubeCurrent);
	vtkCornerAnnotation->SetText(2, sliceInfo.toLatin1().constData());
}

class vtkResliceCursorCallback : public vtkCommand, public QObject
{
public:
	static vtkResliceCursorCallback *New()
	{
		return new vtkResliceCursorCallback;
	}
	void Execute(vtkObject *caller, unsigned long ev, void *callData) override
	{
		if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||	ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
		{
			//setCornerAnnotations
			vtkResliceCursorWidget *rcw = dynamic_cast<vtkResliceCursorWidget *>(caller);
			if (rcw)
			{
				vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(rcw->GetRepresentation());
				double m_CurrentWL[2];
				rep->GetWindowLevel(m_CurrentWL);
				for (int i = 0; i < 3; i++)
				{
					//int now = m_resliceImageViewer[i]->GetSlice() + 1;
					int max = m_resliceImageViewer[i]->GetSliceMax() + 1;
					setCornerAnnotations(m_cornerAnnotations[i], max, MathTools::roundToNearestInteger(m_CurrentWL[0]), MathTools::roundToNearestInteger(m_CurrentWL[1]));
				}
			}
			// Render everything
			for (int i = 0; i < 3; i++)
			{
				this->RCW[i]->Render();
			}
			this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
			return;
		}


		vtkImagePlaneWidget* ipw =	dynamic_cast<vtkImagePlaneWidget*>(caller);
		if (ipw)
		{
			double* wl = static_cast<double*>(callData);
		
			if (ipw == this->IPW[0])
			{
				this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
				this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
			}
			else if (ipw == this->IPW[1])
			{
				this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
				this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
			}
			else if (ipw == this->IPW[2])
			{
				this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
				this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
			}
		}

		vtkResliceCursorWidget *rcw = dynamic_cast<	vtkResliceCursorWidget *>(caller);
		if (rcw)
		{
			vtkResliceCursorLineRepresentation *rep = dynamic_cast<	vtkResliceCursorLineRepresentation *>(rcw->GetRepresentation());
			// Although the return value is not used, we keep the get calls
		    // in case they had side-effects
			rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
			for (int i = 0; i < 3; i++)
			{
				vtkPlaneSource *ps = static_cast<vtkPlaneSource *>(	this->IPW[i]->GetPolyDataAlgorithm());
				ps->SetOrigin(this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetOrigin());
				ps->SetPoint1(this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint1());
				ps->SetPoint2(this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint2());
		
				// If the reslice plane has modified, update it on the 3D widget
				this->IPW[i]->UpdatePlacement();
			}
		}

		// Render everything
		for (int i = 0; i < 3; i++)
		{
			this->RCW[i]->Render();
		}
		this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
	}
	vtkResliceCursorCallback() {}
public:
	vtkImagePlaneWidget*             IPW[3];
	vtkResliceCursorWidget*          RCW[3];
	vtkMPRResliceImageViewer*        m_resliceImageViewer[3];
	vtkCornerAnnotation*             m_cornerAnnotations[3];
};

////有人提问,VTK没有看到好的方法. https://discourse.vtk.org/t/the-picture-of-coronal-planes-left-and-right-is-reversion-in-vtk8-2-0-dicom-mpr/1754/2
QMPR3DExtension::QMPR3DExtension(QWidget *parent): QWidget(parent), m_axialZeroSliceCoordinate(.0)
{
    setupUi(this);
    MPR3DSettings().init();

    init();
    createActions();
    createConnections();
    createActors();
    readSettings();
    // Window level adjustments for the combo box

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

void QMPR3DExtension::changeSetWindowLevel()
{
	VoiLut voiLut = m_VoiLutPresetsToolData->getCurrentPreset();
	m_CurrentWL[0] = voiLut.getWindowLevel().getWidth();
	m_CurrentWL[1] = voiLut.getWindowLevel().getCenter();
	for (int i = 0; i < 3; i++)
	{
		//int now = m_resliceImageViewer[i]->GetSlice() + 1;
		int max = m_resliceImageViewer[i]->GetSliceMax() + 1;				
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation());
		rep->SetWindowLevel(m_CurrentWL[0], m_CurrentWL[1]);
		setCornerAnnotations(m_cornerAnnotations[i], max, MathTools::roundToNearestInteger(m_CurrentWL[0]), MathTools::roundToNearestInteger(m_CurrentWL[1]));
	}
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->Render();
	}
}
QMPR3DExtension::~QMPR3DExtension()
{
    writeSettings();
	//-------------
	for (int i = 0; i < 3; i++)
	{
		if (m_resliceImageViewer[i])
		{
			m_resliceImageViewer[i]->Delete();
			m_renderWindow[i]->Delete();
			m_cornerAnnotations[i]->Delete();
			m_planeWidget[i]->Delete();
		}
	}
	delete m_VoiLutPresetsToolData;
	
	//-------------
	/*

    //Doing this or not seems to free up the same memory thanks to smart pointers
    if (m_sagitalReslice)
    {
        m_sagitalReslice->Delete();
    }
    if (m_coronalReslice)
    {
        m_coronalReslice->Delete();
    }

	if (m_transform)
       m_transform->Delete();

	if (m_sagitalOverAxialAxisActor)
		m_sagitalOverAxialAxisActor->Delete();

	if (m_axialOverSagitalIntersectionAxis)
		m_axialOverSagitalIntersectionAxis->Delete();

	if (m_coronalOverAxialIntersectionAxis)
		m_coronalOverAxialIntersectionAxis->Delete();

	if (m_coronalOverSagitalIntersectionAxis)
		m_coronalOverSagitalIntersectionAxis->Delete();

	if (m_thickSlabOverAxialActor)
		m_thickSlabOverAxialActor->Delete();

	if (m_thickSlabOverSagitalActor)
		m_thickSlabOverSagitalActor->Delete();

	if (m_axialViewSagitalCenterDrawerPoint)
	{
		m_axialViewSagitalCenterDrawerPoint->decreaseReferenceCount();
		delete m_axialViewSagitalCenterDrawerPoint;
	}


	if (m_axialViewCoronalCenterDrawerPoint)
	{
		m_axialViewCoronalCenterDrawerPoint->decreaseReferenceCount();
		delete m_axialViewCoronalCenterDrawerPoint;
	}


	if (m_sagitalViewAxialCenterDrawerPoint)
	{
		m_sagitalViewAxialCenterDrawerPoint->decreaseReferenceCount();
		delete m_sagitalViewAxialCenterDrawerPoint;
	}


	if (m_sagitalViewCoronalCenterDrawerPoint)
	{
		m_sagitalViewCoronalCenterDrawerPoint->decreaseReferenceCount();
		delete m_sagitalViewCoronalCenterDrawerPoint;
	}

	if (m_sagitalViewCoronalCenterDrawerPoint)
		m_axialPlaneSource->Delete();

	if (m_sagitalViewCoronalCenterDrawerPoint)
		m_sagitalPlaneSource->Delete();

	if (m_sagitalViewCoronalCenterDrawerPoint)
		m_coronalPlaneSource->Delete();

	if (m_sagitalViewCoronalCenterDrawerPoint)
		m_thickSlabPlaneSource->Delete();

    if (m_pickedActorReslice)
    {
        m_pickedActorReslice->Delete();
    }

    if (m_mipViewer)
    {
        delete m_mipViewer;
    }

	if (m_mipViewer)
	{
		delete m_coronal2DView;
	}


	if (m_VoiLutPresetsToolData)
	{
		delete m_VoiLutPresetsToolData;
	}
	
	*/
}

void QMPR3DExtension::on_m_reset_clicked()
{
	ResetViews();
}
void QMPR3DExtension::init()
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
    //m_sagital2DView->removeAnnotation(PatientOrientationAnnotation | MainInformationAnnotation | SliceAnnotation);
    //m_coronal2DView->removeAnnotation(PatientOrientationAnnotation | MainInformationAnnotation | SliceAnnotation);
    showViewerInformation(m_viewerInformationToolButton->isChecked());

    //m_sagital2DView->disableContextMenu();
    //m_coronal2DView->disableContextMenu();

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

	//add
	m_thickModeBox->setEnabled(false);
    m_reset->setIcon(QIcon(":/images/icons/reset-view.svg"));
	m_reset->setToolTip("reset");

	connect(m_thickMode, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));
	connect(m_thickModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(resliceModeComBox(int)));
	m_slicingToolButton->hide();
	m_zoomToolButton->hide();
	m_ROIToolButton->hide();
	m_distanceToolButton->hide();
	m_angleToolButton->hide();
	m_eraserToolButton->hide();
	m_screenShotToolButton->hide();
	m_screenshotsExporterToolButton->hide();
	//m_viewerInformationToolButton
	m_viewerInformationToolButton->hide();
	m_voxelInformationToolButton->hide();
	m_thickSlabLabel->hide();
	m_thickSlabSlider->hide();
	m_thickSlabSpinBox->hide();
	m_mipToolButton->hide();
	//m_horizontalLayoutToolButton->hide();
	m_screenshotsExporterToolButton->hide();
	m_voiLutComboBox2->hide();
	labelLUT2->hide();

	//-----
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i] = nullptr;
		m_renderWindow[i]       = nullptr;
		m_cornerAnnotations[i]  = nullptr;
		m_planeWidget[i]        = nullptr;
	}
	m_VoiLutPresetsToolData = nullptr;

}

void QMPR3DExtension::resliceMode(int mode)
{
    m_thickModeBox->setEnabled(mode);
}
void QMPR3DExtension::resliceModeComBox(int mode)
{
	//m_thickModeBox->setEnabled(mode);
	if (mode > 0)
	{
		//SetBlendMode(VTK_IMAGE_SLAB_MAX);	
		//SetBlendMode(VTK_IMAGE_SLAB_MIN);	
		//SetBlendMode(VTK_IMAGE_SLAB_MEAN);
		for (int i = 0; i < 3; i++)
		{
			m_resliceImageViewer[i]->SetResliceMode(1);
			m_resliceImageViewer[i]->SetThickMode(1);
			
			vtkImageSlabReslice* thickSlabReslice = vtkImageSlabReslice::SafeDownCast(vtkResliceCursorThickLineRepresentation::SafeDownCast(
				                                    m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
			thickSlabReslice->SetBlendMode(mode - 1);

			//
			vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation());
			rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
			rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
			rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
			//https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8879
			rep->GetResliceCursorActor()->GetCenterlineProperty(0)->RenderLinesAsTubesOn();
			rep->GetResliceCursorActor()->GetCenterlineProperty(1)->RenderLinesAsTubesOn();
			rep->GetResliceCursorActor()->GetCenterlineProperty(2)->RenderLinesAsTubesOn();
			rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetLineWidth(2);
			rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetLineWidth(2);
			rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetLineWidth(2);
			//
			//
			m_resliceImageViewer[i]->Render();
		}
	}
}

void QMPR3DExtension::createActions()
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

void QMPR3DExtension::initializeZoomTools()
{
   //Q_ASSERT(m_toolManager);
   //
   //m_zoomToolButton->setDefaultAction(m_toolManager->registerTool("ZoomTool"));
   //// We add a menu to the zoom button to incorporate the focused zoom tool
   //m_zoomToolButton->setPopupMode(QToolButton::MenuButtonPopup);
   //QMenu *zoomToolMenu = new QMenu(this);
   //m_zoomToolButton->setMenu(zoomToolMenu);
   //zoomToolMenu->addAction(m_toolManager->registerTool("MagnifyingGlassTool"));
   //
   //connect(m_toolManager->getRegisteredToolAction("ZoomTool"), SIGNAL(triggered()), SLOT(rearrangeZoomToolsMenu()));
   //connect(m_toolManager->getRegisteredToolAction("MagnifyingGlassTool"), SIGNAL(triggered()), SLOT(rearrangeZoomToolsMenu()));
}

void QMPR3DExtension::initializeROITools()
{
    //Q_ASSERT(m_toolManager);
    //
    //m_ROIToolButton->setDefaultAction(m_toolManager->registerTool("EllipticalROITool"));
    //// We add a menu to the PolylineROI button to incorporate the elliptical ROI tool
    //m_ROIToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    //QMenu *roiToolMenu = new QMenu(this);
    //m_ROIToolButton->setMenu(roiToolMenu);
    //roiToolMenu->addAction(m_toolManager->registerTool("MagicROITool"));
    //roiToolMenu->addAction(m_toolManager->registerTool("PolylineROITool"));
    //roiToolMenu->addAction(m_toolManager->registerTool("CircleTool"));
    //
    //connect(m_toolManager->getRegisteredToolAction("EllipticalROITool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
    //connect(m_toolManager->getRegisteredToolAction("MagicROITool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
    //connect(m_toolManager->getRegisteredToolAction("PolylineROITool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
    //connect(m_toolManager->getRegisteredToolAction("CircleTool"), SIGNAL(triggered()), SLOT(rearrangeROIToolsMenu()));
}

void QMPR3DExtension::initializeDistanceTools()
{
   //Q_ASSERT(m_toolManager);
   //
   //m_distanceToolButton->setDefaultAction(m_toolManager->registerTool("DistanceTool"));
   ////We add a menu to the distance button to incorporate the perpendicular distance tool
   //m_distanceToolButton->setPopupMode(QToolButton::MenuButtonPopup);
   //QMenu *distanceToolMenu = new QMenu(this);
   //m_distanceToolButton->setMenu(distanceToolMenu);
   //distanceToolMenu->addAction(m_toolManager->registerTool("PerpendicularDistanceTool"));
   //connect(m_toolManager->getRegisteredToolAction("DistanceTool"), SIGNAL(triggered()), SLOT(rearrangeDistanceToolsMenu()));
   //connect(m_toolManager->getRegisteredToolAction("PerpendicularDistanceTool"), SIGNAL(triggered()), SLOT(rearrangeDistanceToolsMenu()));
}

void QMPR3DExtension::initializeAngleTools()
{
   // Q_ASSERT(m_toolManager);
   //
   // m_angleToolButton->setDefaultAction(m_toolManager->registerTool("AngleTool"));
   // // We add a menu to the angle button to incorporate the open angle tool
   // m_angleToolButton->setPopupMode(QToolButton::MenuButtonPopup);
   // QMenu *angleToolMenu = new QMenu(this);
   // m_angleToolButton->setMenu(angleToolMenu);
   // angleToolMenu->addAction(m_toolManager->registerTool("NonClosedAngleTool"));
   // connect(m_toolManager->getRegisteredToolAction("AngleTool"), SIGNAL(triggered()), SLOT(rearrangeAngleToolsMenu()));
   // connect(m_toolManager->getRegisteredToolAction("NonClosedAngleTool"), SIGNAL(triggered()), SLOT(rearrangeAngleToolsMenu()));
}

void QMPR3DExtension::initializeTools()
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
    //m_toolManager->setupRegisteredTools(m_axial2DView);
    // On the other viewers we want all the registered tools but slicing (mouse and keyboard modes)
    QStringList toolsList = m_toolManager->getRegisteredToolsList();
    toolsList.removeAt(toolsList.indexOf("SlicingMouseTool"));
    toolsList.removeAt(toolsList.indexOf("SlicingKeyboardTool"));
    toolsList.removeAt(toolsList.indexOf("SlicingWheelTool"));
    //m_toolManager->setViewerTools(m_sagital2DView, toolsList);
    //m_toolManager->setViewerTools(m_coronal2DView, toolsList);
}

void QMPR3DExtension::createConnections()
{
    //// We connect the sliders and other viewers
    //connect(m_axial2DView, SIGNAL(sliceChanged(int)), m_axialSlider, SLOT(setValue(int)));
    //connect(m_axialSlider, SIGNAL(valueChanged(int)), m_axial2DView, SLOT(setSlice(int)));

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
    //m_axial2DView->setAutomaticallyLoadPatientBrowserMenuSelectedInput(false);
    //connect(m_axial2DView->getPatientBrowserMenu(), SIGNAL(selectedVolume(Volume*)), SLOT(setInput(Volume*)));
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

void QMPR3DExtension::rearrangeToolsMenu(QToolButton *menuButton)
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

void QMPR3DExtension::rearrangeROIToolsMenu()
{
    rearrangeToolsMenu(m_ROIToolButton);
}

void QMPR3DExtension::rearrangeAngleToolsMenu()
{
    rearrangeToolsMenu(m_angleToolButton);
}

void QMPR3DExtension::rearrangeZoomToolsMenu()
{
    rearrangeToolsMenu(m_zoomToolButton);
}

void QMPR3DExtension::rearrangeDistanceToolsMenu()
{
    rearrangeToolsMenu(m_distanceToolButton);
}

void QMPR3DExtension::changeSelectedViewer()
{

}

void QMPR3DExtension::ResetViews()
{
	m_thickModeBox->setEnabled(0);
	m_thickModeBox->setCurrentText("None");
	m_thickMode->setChecked(0);
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->SetResliceMode(1);
		//m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
		vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation())->SetWindowLevel(m_DeaultWL[0], m_DeaultWL[1]);
	}
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->Reset();
		m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
		m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
	}
	vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, PI);
	for (int i = 0; i < 3; i++)
	{
		//
		vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation());
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
		//https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8879
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetLineWidth(2);
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetLineWidth(2);
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetLineWidth(2);
		//
		m_resliceImageViewer[i]->Render();
	}

}
void QMPR3DExtension::screenShot()
{

}

void QMPR3DExtension::showScreenshotsExporterDialog()
{

}

void QMPR3DExtension::showViewerInformation(bool show)
{

}

void QMPR3DExtension::updateProjectionLabel()
{
    //m_projectionLabel->setText(m_axial2DView->getCurrentAnatomicalPlaneLabel());
}

void QMPR3DExtension::switchHorizontalLayout()
{
    QWidget *leftWidget, *rightWidget;
    leftWidget = m_horizontalSplitter->widget(0);
    rightWidget = m_horizontalSplitter->widget(1);

    m_horizontalSplitter->insertWidget(0, rightWidget);
    m_horizontalSplitter->insertWidget(1, leftWidget);
}

void QMPR3DExtension::switchToMIPLayout(bool isMIPChecked)
{

}

void QMPR3DExtension::handleAxialViewEvents(unsigned long eventID)
{


}

void QMPR3DExtension::handleSagitalViewEvents(unsigned long eventID)
{


}

bool QMPR3DExtension::detectAxialViewAxisActor()
{

	return true;
}

void QMPR3DExtension::rotateAxialViewAxisActor()
{
   
}

void QMPR3DExtension::releaseAxialViewAxisActor()
{

}

void QMPR3DExtension::detectSagitalViewAxisActor()
{

}

void QMPR3DExtension::rotateSagitalViewAxisActor()
{
   
}

void QMPR3DExtension::releaseSagitalViewAxisActor()
{

}

void QMPR3DExtension::getRotationAxis(vtkPlaneSource *plane, double axis[3])
{

}

void QMPR3DExtension::pushSagitalViewCoronalAxisActor()
{

}

void QMPR3DExtension::pushAxialViewAxisActor()
{

}

void QMPR3DExtension::detectPushSagitalViewAxisActor()
{
 
}

void QMPR3DExtension::pushSagitalViewAxialAxisActor()
{

}

void QMPR3DExtension::setInput(Volume *input)
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
	///--------------------------------------
	//---------------------------20241105-------------------------------------------------------------------------------
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i] = vtkMPRResliceImageViewer::New();
		m_resliceImageViewer[i]->init(i == 1);
		m_renderWindow[i] = vtkGenericOpenGLRenderWindow::New();
		m_resliceImageViewer[i]->SetRenderWindow(m_renderWindow[i]);
		m_cornerAnnotations[i] = vtkCornerAnnotation::New();
		m_resliceImageViewer[i]->GetRenderer()->AddViewProp(m_cornerAnnotations[i]);

	}

#ifdef VTK94
	m_sagital2DView->setRenderWindow(m_resliceImageViewer[0]->GetRenderWindow());
	m_resliceImageViewer[0]->SetupInteractor(m_sagital2DView->renderWindow()->GetInteractor());

	m_coronal2DView->setRenderWindow(m_resliceImageViewer[1]->GetRenderWindow());
	m_resliceImageViewer[1]->SetupInteractor(m_coronal2DView->renderWindow()->GetInteractor());

	m_axial2DView->setRenderWindow(m_resliceImageViewer[2]->GetRenderWindow());
	m_resliceImageViewer[2]->SetupInteractor(m_axial2DView->renderWindow()->GetInteractor());
#else
	m_sagital2DView->SetRenderWindow(m_resliceImageViewer[0]->GetRenderWindow());
	m_resliceImageViewer[0]->SetupInteractor(m_sagital2DView->GetRenderWindow()->GetInteractor());

	m_coronal2DView->SetRenderWindow(m_resliceImageViewer[1]->GetRenderWindow());
	m_resliceImageViewer[1]->SetupInteractor(m_coronal2DView->GetRenderWindow()->GetInteractor());

	m_axial2DView->SetRenderWindow(m_resliceImageViewer[2]->GetRenderWindow());
	m_resliceImageViewer[2]->SetupInteractor(m_axial2DView->GetRenderWindow()->GetInteractor());
#endif // VTK94

	m_lastInput = selVolume;
	Volume * vl = selVolume;
	vtkSmartPointer <vtkImageData> imageData = NULL;
	if (vl)
	{
		vl->getVtkData()->Modified();
		imageData = vl->getVtkData();
		vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
		ImageFlip->SetInputData(imageData);
		ImageFlip->SetFilteredAxes(0);
		ImageFlip->Update();
		imageData = ImageFlip->GetOutput();
		m_volume = vl;

		m_VoiLutPresetsToolData = new VoiLutPresetsToolData(this);
		VoiLutHelper().initializeVoiLutData(m_VoiLutPresetsToolData, vl);
		m_voiLutComboBox->setPresetsData(m_VoiLutPresetsToolData);
		m_voiLutComboBox->selectPreset(m_VoiLutPresetsToolData->getCurrentPresetName());
		m_voiLutComboBox->setToolTip(tr("Choose a VOI LUT preset"));
		VoiLut voiLut = m_VoiLutPresetsToolData->getCurrentPreset();
		m_CurrentWL[0] = voiLut.getWindowLevel().getWidth();
		m_CurrentWL[1] = voiLut.getWindowLevel().getCenter();
		m_DeaultWL[0] = m_CurrentWL[0];
		m_DeaultWL[1] = m_CurrentWL[1];
		//changeSetWindowLevel
		connect(m_voiLutComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSetWindowLevel()));

		///-----------------------------------------------------
		g_SliceThickness[0] = getSliceThickness(vl, 0);
		g_SliceThickness[1] = getSliceThickness(vl, 1);
		g_SliceThickness[2] = getSliceThickness(vl, 2);
		g_ser = vl->getSeries()->getSeriesNumber();
		g_time = vl->getSeries()->getDateAsString() + vl->getSeries()->getTimeAsString();
		g_institutionName = vl->getSeries()->getInstitutionName();
		g_dicomKVP = vl->getImage(0)->getDICOMKVP();
		g_dicomXRayTubeCurrent = vl->getImage(0)->getXRayTubeCurrent();
	}
	else
	{
		return;
	}
	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation());
		m_resliceImageViewer[i]->SetResliceCursor(m_resliceImageViewer[0]->GetResliceCursor());
		rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
		//-----------------------------------------------------------------------------------------------------
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
		//https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8879
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetLineWidth(2);
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetLineWidth(2);
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetLineWidth(2);
		//-------------------------------------------------------------------------------------------------------
		m_resliceImageViewer[i]->SetInputData(imageData);
		m_resliceImageViewer[i]->SetSliceOrientation(i);
		m_resliceImageViewer[i]->SetResliceModeToAxisAligned();
		rep->SetWindowLevel(m_CurrentWL[0], m_CurrentWL[1]);
	}

	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);
	vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();
	//vtkSmartPointer< vtkRenderer > ren = vtkSmartPointer< vtkRenderer >::New();
	m_MPR3DvtkRenderer = vtkSmartPointer< vtkRenderer >::New();
	//---
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
	m_mpr2DView->setRenderWindow(renderWindow);
	m_mpr2DView->renderWindow()->AddRenderer(m_MPR3DvtkRenderer);
	vtkRenderWindowInteractor* iren = m_mpr2DView->interactor();
	//---
	int imageDims[3];
	double color[3] = { 0, 0, 0 };
	double planeColor[3] = { 0, 0, 0 };

	color[0] /= 4.0;
	color[1] /= 4.0;
	color[2] /= 4.0;
	for (int i = 0; i < 3; i++)
	{
		m_planeWidget[i] = vtkImagePlaneWidget::New();
		m_planeWidget[i]->SetInteractor(iren);
		m_planeWidget[i]->SetPicker(picker);
		m_planeWidget[i]->RestrictPlaneToVolumeOn();

		planeColor[i] = 1;
		planeColor[0] /= 4.0;
		planeColor[1] /= 4.0;
		planeColor[2] /= 4.0;
		m_planeWidget[i]->GetPlaneProperty()->SetColor(planeColor/*color*/);
		m_resliceImageViewer[i]->GetRenderer()->SetBackground(color);

		m_planeWidget[i]->SetTexturePlaneProperty(ipwProp);
		m_planeWidget[i]->TextureInterpolateOff();
		m_planeWidget[i]->SetResliceInterpolateToLinear();
		m_planeWidget[i]->SetInputData(imageData);

		m_planeWidget[i]->SetPlaneOrientation(i);
		m_planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
		m_planeWidget[i]->DisplayTextOn();
		m_planeWidget[i]->SetDefaultRenderer(m_MPR3DvtkRenderer);
		m_planeWidget[i]->SetWindowLevel(m_CurrentWL[0], m_CurrentWL[1]);
		m_planeWidget[i]->On();
		m_planeWidget[i]->InteractionOn();
	}

	vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();
	for (int i = 0; i < 3; i++)
	{
		cbk->IPW[i] = m_planeWidget[i];
		cbk->RCW[i] = m_resliceImageViewer[i]->GetResliceCursorWidget();
		cbk->m_resliceImageViewer[i] = m_resliceImageViewer[i];
		cbk->m_cornerAnnotations[i]  = m_cornerAnnotations[i];

		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
		m_resliceImageViewer[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);
		m_resliceImageViewer[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, cbk);

		// Make them all share the same color map.
		m_resliceImageViewer[i]->SetLookupTable(m_resliceImageViewer[0]->GetLookupTable());
		//m_planeWidget[i]->GetColorMap()->SetLookupTable(m_resliceImageViewer[0]->GetLookupTable());
		m_planeWidget[i]->SetColorMap(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
	}
	vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, PI);

	//add  cornerAnnotations
	for (int i = 0; i < 3; i++)
	{
		//int now = m_resliceImageViewer[i]->GetSlice() + 1;
		int max = m_resliceImageViewer[i]->GetSliceMax() + 1;
		setCornerAnnotations(m_cornerAnnotations[i], max, MathTools::roundToNearestInteger(m_CurrentWL[0]), MathTools::roundToNearestInteger(m_CurrentWL[1]));
	}
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->SetResliceMode(1);
		m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
		m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
		m_resliceImageViewer[i]->Render();
	}
	
	m_axial2DView->show();
	m_sagital2DView->show();

	QList<int> hlist;
	//hlist << -1 << m_coronal2DView->height()
	hlist << m_coronal2DView->height() << 0;
	m_verticalSplitter1->setSizes(hlist);
	m_coronal2DView->show();
	//m_mpr2DView->setWindowFlags(Qt::FramelessWindowHint);
	//m_mpr2DView->show();
	//QSize size = m_verticalSplitter1->size();
	

	//---------------------------------------
}

void QMPR3DExtension::initOrientation()
{
  
}

void QMPR3DExtension::createActors()
{
	//m_sagitalOverAxialAxisActor          = nullptr;
	//m_coronalOverAxialIntersectionAxis   = nullptr;
	//m_coronalOverSagitalIntersectionAxis = nullptr;
	//m_axialOverSagitalIntersectionAxis   = nullptr;
	//m_thickSlabOverAxialActor            = nullptr;
	//m_thickSlabOverSagitalActor          = nullptr;
	//
	//return;
	//
    //QColor axialColor = QColor::fromRgbF(1.0, 1.0, 0.0);
    //QColor sagitalColor = QColor::fromRgbF(1.0, 0.6, 0.0);
    //QColor coronalColor = QColor::fromRgbF(0.0, 1.0, 1.0);
	//
    //// We create the axis actors
    //m_sagitalOverAxialAxisActor = vtkAxisActor2D::New();
    //m_coronalOverAxialIntersectionAxis = vtkAxisActor2D::New();
    //m_coronalOverSagitalIntersectionAxis = vtkAxisActor2D::New();
    //m_axialOverSagitalIntersectionAxis = vtkAxisActor2D::New();
    //m_thickSlabOverAxialActor = vtkAxisActor2D::New();
    //m_thickSlabOverSagitalActor = vtkAxisActor2D::New();
	//
    //m_sagitalOverAxialAxisActor->AxisVisibilityOn();
    //m_sagitalOverAxialAxisActor->TickVisibilityOff();
    //m_sagitalOverAxialAxisActor->LabelVisibilityOff();
    //m_sagitalOverAxialAxisActor->TitleVisibilityOff();
    //m_sagitalOverAxialAxisActor->GetProperty()->SetColor(sagitalColor.redF(), sagitalColor.greenF(), sagitalColor.blueF());
	//
    //m_coronalOverAxialIntersectionAxis->TickVisibilityOff();
    //m_coronalOverAxialIntersectionAxis->LabelVisibilityOff();
    //m_coronalOverAxialIntersectionAxis->TitleVisibilityOff();
    //m_coronalOverAxialIntersectionAxis->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());
	//
    //m_coronalOverSagitalIntersectionAxis->AxisVisibilityOn();
    //m_coronalOverSagitalIntersectionAxis->TickVisibilityOff();
    //m_coronalOverSagitalIntersectionAxis->LabelVisibilityOff();
    //m_coronalOverSagitalIntersectionAxis->TitleVisibilityOff();
    //m_coronalOverSagitalIntersectionAxis->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());
	//
    //m_axialOverSagitalIntersectionAxis->AxisVisibilityOn();
    //m_axialOverSagitalIntersectionAxis->TickVisibilityOff();
    //m_axialOverSagitalIntersectionAxis->LabelVisibilityOff();
    //m_axialOverSagitalIntersectionAxis->TitleVisibilityOff();
    //m_axialOverSagitalIntersectionAxis->GetProperty()->SetColor(axialColor.redF(), axialColor.greenF(), axialColor.blueF());
	//
    /////For now the thickslab lines will be invisible as we cannot do MIPs
    ///// and being superimposed on the lines of the planes have a bad effect
    //m_thickSlabOverAxialActor->AxisVisibilityOff();
    //m_thickSlabOverAxialActor->TickVisibilityOff();
    //m_thickSlabOverAxialActor->LabelVisibilityOff();
    //m_thickSlabOverAxialActor->TitleVisibilityOff();
    //m_thickSlabOverAxialActor->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());
    //m_thickSlabOverAxialActor->GetProperty()->SetLineStipplePattern(65280);
	//
    //m_thickSlabOverSagitalActor->AxisVisibilityOff();
    //m_thickSlabOverSagitalActor->TickVisibilityOff();
    //m_thickSlabOverSagitalActor->LabelVisibilityOff();
    //m_thickSlabOverSagitalActor->TitleVisibilityOff();
    //m_thickSlabOverSagitalActor->GetProperty()->SetColor(coronalColor.redF(), coronalColor.greenF(), coronalColor.blueF());
    //m_thickSlabOverSagitalActor->GetProperty()->SetLineStipplePattern(65280);
	//
    //// We create the drawer points
	//
    //m_axialViewSagitalCenterDrawerPoint = new DrawerPoint();
    //m_axialViewSagitalCenterDrawerPoint->increaseReferenceCount();
    //m_axialViewSagitalCenterDrawerPoint->setColor(sagitalColor);
	//
    //m_axialViewCoronalCenterDrawerPoint = new DrawerPoint();
    //m_axialViewCoronalCenterDrawerPoint->increaseReferenceCount();
    //m_axialViewCoronalCenterDrawerPoint->setColor(coronalColor);
	//
    //m_sagitalViewAxialCenterDrawerPoint = new DrawerPoint();
    //m_sagitalViewAxialCenterDrawerPoint->increaseReferenceCount();
    //m_sagitalViewAxialCenterDrawerPoint->setColor(axialColor);
	//
    //m_sagitalViewCoronalCenterDrawerPoint = new DrawerPoint();
    //m_sagitalViewCoronalCenterDrawerPoint->increaseReferenceCount();
    //m_sagitalViewCoronalCenterDrawerPoint->setColor(coronalColor);
}

void QMPR3DExtension::axialSliceUpdated(int slice)
{

}

void QMPR3DExtension::updateControls()
{
 

}

void QMPR3DExtension::updatePlanes()
{

}

void QMPR3DExtension::updatePlane(vtkPlaneSource *planeSource, vtkImageReslice *reslice, int extentLength[2])
{
 
}

void QMPR3DExtension::getSagitalXVector(double x[3])
{
   
}

void QMPR3DExtension::getSagitalYVector(double y[3])
{

}

void QMPR3DExtension::getCoronalXVector(double x[3])
{

}

void QMPR3DExtension::getAxialXVector(double x[3])
{

}

void QMPR3DExtension::getAxialYVector(double y[3])
{

}
void QMPR3DExtension::getCoronalYVector(double y[3])
{

}

bool QMPR3DExtension::isParallel(double axis[3])
{
  
    return false;
}

void QMPR3DExtension::rotateMiddle(double degrees, double rotationAxis[3], vtkPlaneSource *plane)
{

}

void QMPR3DExtension::updateThickSlab(double value)
{

}

void QMPR3DExtension::updateThickSlab(int value)
{

}

void QMPR3DExtension::readSettings()
{
    Settings settings;

    if (settings.getValue(MPR3DSettings::HorizontalSplitterGeometry).toByteArray().isEmpty())
    {
        QList<int> list;
        list << this->size().width() / 2 << this->size().width() / 2;
        m_horizontalSplitter->setSizes(list);
    }
    else
    {
        settings.restoreGeometry(MPR3DSettings::HorizontalSplitterGeometry, m_horizontalSplitter);
    }

    if (settings.getValue(MPR3DSettings::VerticalSplitterGeometry).toByteArray().isEmpty())
    {
        QList<int> list;
        list << this->size().height() / 2 << this->size().height() / 2;
        m_verticalSplitter->setSizes(list);
    }
    else
    {
        settings.restoreGeometry(MPR3DSettings::VerticalSplitterGeometry, m_verticalSplitter);
    }
}

void QMPR3DExtension::writeSettings()
{
    Settings settings;

    settings.saveGeometry(MPR3DSettings::HorizontalSplitterGeometry, m_horizontalSplitter);
    settings.saveGeometry(MPR3DSettings::VerticalSplitterGeometry, m_verticalSplitter);
}

//vtkTransform* QMPR3DExtension::getWorldToSagitalTransform() const
//{
// 
//}

void QMPR3DExtension::updateInput(Volume *input)
{
	if (m_lastInput != input)
	{
		///----
		VoiLutHelper().initializeVoiLutData(m_VoiLutPresetsToolData, input);
		m_voiLutComboBox->setPresetsData(m_VoiLutPresetsToolData);
		m_voiLutComboBox->selectPreset(m_VoiLutPresetsToolData->getCurrentPresetName());
		m_voiLutComboBox->setToolTip(tr("Choose a VOI LUT preset"));
		VoiLut voiLut = m_VoiLutPresetsToolData->getCurrentPreset();
		m_CurrentWL[0] = voiLut.getWindowLevel().getWidth();
		m_CurrentWL[1] = voiLut.getWindowLevel().getCenter();
		m_DeaultWL[0] = m_CurrentWL[0];
		m_DeaultWL[1] = m_CurrentWL[1];
		///----
		m_lastInput = input;
		vtkSmartPointer <vtkImageData> imageData = NULL;
		imageData = m_lastInput->getVtkData();
		vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
		ImageFlip->SetInputData(imageData);
		ImageFlip->SetFilteredAxes(0);
		ImageFlip->Update();
		imageData = ImageFlip->GetOutput();

		g_ser = m_lastInput->getSeries()->getSeriesNumber();
		g_time = m_lastInput->getSeries()->getDateAsString() + m_lastInput->getSeries()->getTimeAsString();
		g_institutionName = m_lastInput->getSeries()->getInstitutionName();
		g_dicomKVP = m_lastInput->getImage(0)->getDICOMKVP();
		g_dicomXRayTubeCurrent = m_lastInput->getImage(0)->getXRayTubeCurrent();

		for (int i = 0; i < 3; i++)
		{
			m_resliceImageViewer[i]->SetInputData(imageData);
			m_planeWidget[i]->SetInputData(imageData);
		}
		ResetViews();
		for (int i = 0; i < 3; i++)
		{
			int max = m_resliceImageViewer[i]->GetSliceMax() + 1;
			setCornerAnnotations(m_cornerAnnotations[i], max, MathTools::roundToNearestInteger(m_CurrentWL[0]), MathTools::roundToNearestInteger(m_CurrentWL[1]));
			m_resliceImageViewer[i]->Render();
		}
		//m_MPR3DvtkRenderer->ResetCamera();
		m_mpr2DView->renderWindow()->Render();
	}
}
};  // End namespace udg

