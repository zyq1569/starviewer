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

#include "qviewer.h"
#include "volume.h"
#include "series.h"
#include "image.h"
#include "toolproxy.h"
#include "patientbrowsermenu.h"
//To be able to add and modify the presets we display
#include "voilutpresetstooldata.h"
#include "qviewerworkinprogresswidget.h"
#include "voiluthelper.h"
#include "logging.h"
#include "mathtools.h"
#include "starviewerapplication.h"
#include "coresettings.h"

// TODO:  EVERYTHING: Ouch! SuperGuarrada (tm). To be able to bring out
// the menu and have access to the Main Patient. Must be fixed when removing dependencies from
// interface, pacs, etc.etc. !!
#include "../interface/qapplicationmainwindow.h"

// Qt
#include <QStackedLayout>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QDir>
#include <QScreen>

// Include's vtk
#include <QVTKOpenGLNativeWidget.h>
#include <QVTKInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkBMPWriter.h>
#include <vtkPNGWriter.h>
#include <vtkPNMWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkWindowToImageFilter.h>
#include <vtkEventQtSlotConnect.h>
//Required for zooming
#include <vtkCamera.h>

namespace udg {
	static Volume *m_selectVolume = NULL;
class QeventMouse :public QObject
{
public:
	QeventMouse(QViewer *Viewer) { qViewer = Viewer; };
	~QeventMouse() { };
	bool eventFilter(QObject *object, QEvent *event)
	{
		if (event->type() == QEvent::MouseButtonDblClick)
		{
			QMouseEvent *button = (QMouseEvent * )event;
			if (button->buttons() & Qt::LeftButton)
			{
				qViewer->mouseButtonDblClick();
			}
		}
		else if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *key = (QKeyEvent *)event;
			if (key->key() == Qt::Key_P)
			{
				qViewer->keyPress(key->key());
			}
		}
		return 0;
	}
public:
	QViewer *qViewer;
private:
};

void QViewer::mouseButtonDblClick()
{
	emit doubleClicked();
}

void QViewer::keyPress(int key)
{
	emit keyEventPress(key);
}
QViewer::QViewer(QWidget *parent)
    :QWidget(parent),/* m_mainVolume(0),*/ m_contextMenuActive(true),
      m_mouseHasMoved(false), m_voiLutData(0),m_isRenderingEnabled(true), m_isActive(false)
{
    m_lastAngleDelta = QPoint();
    m_defaultFitIntoViewportMarginRate = 0.0;
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    m_vtkWidget->setFocusPolicy(Qt::WheelFocus);
    m_renderer = vtkRenderer::New();

    m_windowToImageFilter = vtkWindowToImageFilter::New();

    setupRenderWindow();

    this->setCurrentViewPlane(OrthogonalPlane::XYPlane);

    // We connect the events :
    setupInteraction();

    m_toolProxy = new ToolProxy(this);
    connect(this, SIGNAL(eventReceived(unsigned long)), m_toolProxy, SLOT(forwardEvent(unsigned long)));

    // We initialize the window level data
    setVoiLutData(new VoiLutPresetsToolData(this));

    m_workInProgressWidget = new QViewerWorkInProgressWidget(this);

    // We add the layout
    m_stackedLayout = new QStackedLayout(this);
    m_stackedLayout->setSpacing(0);
    m_stackedLayout->setMargin(0);
    m_stackedLayout->addWidget(m_vtkWidget);
    m_stackedLayout->addWidget(m_workInProgressWidget);

    // We initialize the viewer status
    m_previousViewerStatus = m_viewerStatus = NoVolumeInput;
    this->setCurrentWidgetByViewerStatus(m_viewerStatus);
    this->initializeWorkInProgressByViewerStatus(m_viewerStatus);

    this->setMouseTracking(false);
	m_patientBrowserMenu = getStaticBrowserMenu();// new PatientBrowserMenu(0);
    //Right now the default behavior will be that once a
    this->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    //volume is selected we immediately assign it as input
    this->setAutomaticallyLoadPatientBrowserMenuSelectedInput(true);

	//20241219
	m_qeventMouse = new QeventMouse(this);
	installEventFilter(m_qeventMouse);
}

QViewer::~QViewer()
{
    // The removal of the vtkWidget must be at the end as the others
    // objects that we remove can be used during their destruction
    delete m_toolProxy;
    //m_patientBrowserMenu->deleteLater();
    m_windowToImageFilter->Delete();
    delete m_vtkWidget;
    m_vtkQtConnections->Delete();
    m_renderer->Delete();
	delete m_qeventMouse;
}

vtkRenderWindowInteractor* QViewer::getInteractor() const
{
//#ifdef VTK_90
	return m_vtkWidget->interactor();
//#else
//	return m_vtkWidget->GetInteractor();
//#endif // DEBUG

}

vtkRenderer* QViewer::getRenderer() const
{
    return m_renderer;
}
/*
Volume* QViewer::getMainInput() const
{
	return NULL m_mainVolume;
}
*/


int QViewer::getNumberOfInputs() const
{
    if (hasInput())
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

bool QViewer::hasInput() const
{
    if (getMainInput())
    {
        return true;
    }
    else
    {
        return false;
    }
}

vtkRenderWindow* QViewer::getRenderWindow() const
{
//#ifdef VTK_90
    return m_vtkWidget->renderWindow();
//#else
//	return m_vtkWidget->GetRenderWindow();
//#endif // VTK_90


}

QSize QViewer::getRenderWindowSize() const
{
    return QSize(this->getRenderWindow()->GetSize()[0], this->getRenderWindow()->GetSize()[1]);
}

QPoint QViewer::getEventPosition() const
{
    QPoint point(this->getInteractor()->GetEventPosition()[0], this->getInteractor()->GetEventPosition()[1]);
    point *= this->devicePixelRatioF();
    return point;

}

QPoint QViewer::getLastEventPosition() const
{
    QPoint point(this->getInteractor()->GetLastEventPosition()[0], this->getInteractor()->GetLastEventPosition()[1]);
    point *= this->devicePixelRatioF();
    return point;
}

QPoint QViewer::getWheelAngleDelta() const
{
    return m_lastAngleDelta;
}

bool QViewer::isActive() const
{
    return m_isActive;
}

ToolProxy* QViewer::getToolProxy() const
{
    return m_toolProxy;
}

void QViewer::eventHandler(vtkObject *object, unsigned long vtkEvent, void *clientData, void *callData, vtkCommand *command)
{
    Q_UNUSED(object);
    Q_UNUSED(clientData);
    Q_UNUSED(callData);
    Q_UNUSED(command);

#ifdef Q_OS_OSX
    // MouseWheel doesn't work as expected when using a trackpad on Mac because a MouseWheel event is emitted for both vertical and horizontal
    // orientation movements. Only vertical events with a delta different to 0 are captured.
    switch (vtkEvent)
    {
    case vtkCommand::MouseWheelForwardEvent:
    case vtkCommand::MouseWheelBackwardEvent:
    {
        QWheelEvent *e = (QWheelEvent*)callData; //WARNING: I don't like that casting here, may become dangerous.
        if (e)
        {
            if (e->delta() == 0 || e->orientation() == Qt::Horizontal)
            {
                return;
            }
        }
    }
    }
#endif

    if (vtkEvent == vtkCommand::MouseWheelForwardEvent || vtkEvent == vtkCommand::MouseWheelBackwardEvent)
    {
        QWheelEvent *event = (QWheelEvent*)callData; //WARNING: I don't like that casting here, may become dangerous.
        if (event)
        {
            m_lastAngleDelta = event->angleDelta();
        }
    }

    /// When the window is "selected" a signal will be issued indicating this.
    /// We understand selected when the wheel has been clicked or moved over the viewfinder.
    /// TODO It is now inefficient because once selected it would
    /// not be necessary to re-send this signal. The system needs to be improved
    switch (vtkEvent)
    {
	case vtkCommand::LeftButtonPressEvent:
		{
			 if (Volume* volume = getMainInput())
			 {
				 m_selectVolume = volume;
			 }
		}
    case QVTKInteractor::vtkCustomEvents::ContextMenuEvent:
    case vtkCommand::RightButtonPressEvent:
    case vtkCommand::MiddleButtonPressEvent:
    case vtkCommand::MouseWheelForwardEvent:
    case vtkCommand::MouseWheelBackwardEvent:
        m_mouseHasMoved = false;
        setActive(true);
        //if (vtkEvent == vtkCommand::LeftButtonPressEvent && getInteractor()->GetRepeatCount() == 1)
        //{
        //    emit doubleClicked();
		//	if (getToolProxy()->isToolActive("ZoomTool"))
		//	{
		//		return; // avoid accidental pan when doing a double click (#2854)
		//	}
        //}
        break;

    case vtkCommand::MouseMoveEvent:
        m_mouseHasMoved = true;
        break;

    case vtkCommand::RightButtonReleaseEvent:
        if (!m_mouseHasMoved)
        {
            contextMenuRelease();
        }
        break;
    }
    emit eventReceived(vtkEvent);
}

void QViewer::setActive(bool active)
{
    if (!m_isActive && active)
    {
        m_isActive = true;		
        emit selected();
    }
    else
    {
        m_isActive = active;
    }
}

void QViewer::computeDisplayToWorld(double x, double y, double z, double worldPoint[3])
{
    vtkRenderer *renderer = this->getRenderer();
    if (renderer)
    {
        double homogeneousWorldPoint[4];
        renderer->SetDisplayPoint(x, y, z);
        renderer->DisplayToWorld();
        renderer->GetWorldPoint(homogeneousWorldPoint);
        
        double divisor = 1.0;
        if (homogeneousWorldPoint[3])
        {
            divisor = homogeneousWorldPoint[3];
        }
        worldPoint[0] = homogeneousWorldPoint[0] / divisor;
        worldPoint[1] = homogeneousWorldPoint[1] / divisor;
        worldPoint[2] = homogeneousWorldPoint[2] / divisor;
    }
}

void QViewer::computeWorldToDisplay(double x, double y, double z, double displayPoint[3])
{
    vtkRenderer *renderer = this->getRenderer();
    if (renderer)
    {
        renderer->SetWorldPoint(x, y, z, 1.0);
        renderer->WorldToDisplay();
        renderer->GetDisplayPoint(displayPoint);
    }
}

void QViewer::getEventWorldCoordinate(double worldCoordinate[3])
{
    getRecentEventWorldCoordinate(worldCoordinate, true);
}

void QViewer::getLastEventWorldCoordinate(double worldCoordinate[3])
{
    getRecentEventWorldCoordinate(worldCoordinate, false);
}

void QViewer::getRecentEventWorldCoordinate(double worldCoordinate[3], bool current)
{
    QPoint position;

    if (current)
    {
        position = this->getEventPosition();
    }
    else
    {
        position = this->getLastEventPosition();
    }

    this->computeDisplayToWorld(position.x(), position.y(), 0, worldCoordinate);
}

void QViewer::setupInteraction()
{
    Q_ASSERT(m_renderer);

    /// TODO Do this here? or do it in the tool manager?
    this->getInteractor()->RemoveObservers(vtkCommand::LeftButtonPressEvent);
    this->getInteractor()->RemoveObservers(vtkCommand::RightButtonPressEvent);
    this->getInteractor()->RemoveObservers(vtkCommand::MouseWheelForwardEvent);
    this->getInteractor()->RemoveObservers(vtkCommand::MouseWheelBackwardEvent);
    this->getInteractor()->RemoveObservers(vtkCommand::MiddleButtonPressEvent);
    this->getInteractor()->RemoveObservers(vtkCommand::CharEvent);

    m_vtkQtConnections = vtkEventQtSlotConnect::New();
    /// Dispatch any event-> tools
    m_vtkQtConnections->Connect(this->getInteractor(), vtkCommand::AnyEvent, this, SLOT(eventHandler(vtkObject*, unsigned long, void*, void*, vtkCommand*)));
}

vtkCamera* QViewer::getActiveCamera()
{
    return (this->getRenderer() ? this->getRenderer()->GetActiveCamera() : NULL);
}

bool QViewer::saveGrabbedViews(const QString &baseName, FileType extension)
{
    if (!m_grabList.empty())
    {
        vtkImageWriter *writer;
        QString fileExtension;
        switch (extension)
        {
        case PNG:
            writer = vtkPNGWriter::New();
            fileExtension = "png";
            break;

        case JPEG:
            writer = vtkJPEGWriter::New();
            fileExtension = "jpg";
            break;

        case TIFF:
            writer = vtkTIFFWriter::New();
            fileExtension = "tiff";
            break;

        case PNM:
            writer = vtkPNMWriter::New();
            fileExtension = "pnm";
            break;

        case BMP:
            writer = vtkBMPWriter::New();
            fileExtension = "bmp";
            break;

        case DICOM:
		{	
			if (Volume *volume = m_selectVolume)
			{	
				fileExtension = "dcm";
				WARN_LOG("DICOM format file for saving images");
				QList<Image*> images = volume->getImages();
				int count = images.size();
				if (count == 1)
				{
					QFile file(images.at(0)->getPath());
					file.copy(QString("%1.%2").arg(baseName).arg(fileExtension));
				}
				else 
				{
					int i = 0;
					int padding = QString::number(count).size();
					foreach(Image* image, images)
					{
						QFile file(image->getPath());
						file.copy((QString("%1-%2.%3").arg(baseName).arg(i, padding, 10, QChar('0')).arg(fileExtension)));
						i++;
					}
				}		
			}
			clearGrabbedViews();
			return true;
		}
        case META:
            // TODO A suportar
            DEBUG_LOG("The META format is not yet supported for saving images");
			clearGrabbedViews();
            return false;
        }
        int count = m_grabList.count();
        if (count == 1)
        {
            //We only record a single image
            writer->SetInputData(m_grabList.at(0));
            writer->SetFileName(qPrintable(QString("%1.%2").arg(baseName).arg(fileExtension)));
            writer->Write();
        }
        else if (count > 1)
        {
            // We have more than one image, so we add it
            // an additional index for each image automatically
            int i = 0;
            int padding = QString::number(count).size();
            foreach (vtkImageData *image, m_grabList)
            {
                writer->SetInputData(image);
                writer->SetFileName(qPrintable(QString("%1-%2.%3").arg(baseName).arg(i, padding, 10, QChar('0')).arg(fileExtension)));
                writer->Write();
                i++;
            }
        }
        writer->Delete();
        // Let's empty the list
        clearGrabbedViews();
        return true;
    }
    else
    {
        return false;
    }
}

void QViewer::clearGrabbedViews()
{
    foreach (vtkImageData *image, m_grabList)
    {
        image->Delete();
    }
    m_grabList.clear();
}

void QViewer::render()
{
    // ATTENTION It is important that it is only rendered when we are in VisualizingVolume state
    // because otherwise it can cause rendering problems in some cases
    // when the appropriate openGL rendering context is not obtained
    if (m_isRenderingEnabled && getViewerStatus() == VisualizingVolume)
    {
        try
        {
            this->getRenderWindow()->Render();
        }
        catch (const std::bad_alloc &ba)
        {
            WARN_LOG(QString("bad_alloc when trying to render: ").arg(ba.what()));
            handleNotEnoughMemoryForVisualizationError();
        }
    }
}

void QViewer::absoluteZoom(double factor, QPoint zoomCenter)
{
    double currentFactor = getCurrentZoomFactor();

    zoom(currentFactor / (factor * getRenderWindowSize().height()), zoomCenter);
}

double QViewer::getCurrentZoomFactor()
{
    double zoomFactor;

    if (getActiveCamera()->GetParallelProjection())
    {
        zoomFactor = getActiveCamera()->GetParallelScale();
    }
    else
    {
        zoomFactor = getActiveCamera()->GetViewAngle();
    }

    return zoomFactor;
}

void QViewer::zoom(double factor, QPoint zoomCenter)
{
    double worldCenter1[3];
    computeDisplayToWorld(zoomCenter.x(), zoomCenter.y(), 0, worldCenter1);

    if (adjustCameraScaleFactor(factor))
    {
        double worldCenter2[3];
        computeDisplayToWorld(zoomCenter.x(), zoomCenter.y(), 0, worldCenter2);
        Vector3 difference = Vector3(worldCenter1) - Vector3(worldCenter2);
        Vector3 position(getActiveCamera()->GetPosition());
        Vector3 focalPoint(getActiveCamera()->GetFocalPoint());
        position += difference;
        focalPoint += difference;
        getActiveCamera()->SetPosition(position.x, position.y, position.z);
        getActiveCamera()->SetFocalPoint(focalPoint.x, focalPoint.y, focalPoint.z);

        double zoomFactor = getCurrentZoomFactor();

        emit cameraChanged();
        emit zoomChanged(zoomFactor / getRenderWindowSize().height(), zoomCenter);
        this->render();
    }
}

void QViewer::pan(double motionVector[3])
{
    if (!this->hasInput())
    {
        return;
    }

    vtkCamera *camera = getActiveCamera();
    if (!camera)
    {
        DEBUG_LOG("There is no camera");
        return;
    }

    double viewFocus[4], viewPoint[3];
    camera->GetFocalPoint(viewFocus);
    camera->GetPosition(viewPoint);
    camera->SetFocalPoint(motionVector[0] + viewFocus[0], motionVector[1] + viewFocus[1], motionVector[2] + viewFocus[2]);
    camera->SetPosition(motionVector[0] + viewPoint[0], motionVector[1] + viewPoint[1], motionVector[2] + viewPoint[2]);

    //We in principle do not make use of this feature
    if (this->getInteractor()->GetLightFollowCamera())
    {
        vtkRenderer *renderer = getRenderer();
        Q_ASSERT(renderer);
        renderer->UpdateLightsGeometryToFollowCamera();
    }

    double xyz[3];
    getCurrentFocalPoint(xyz);

    emit cameraChanged();
    emit panChanged(xyz);
    this->render();
}

bool QViewer::getCurrentFocalPoint(double focalPoint[3])
{
    vtkCamera *camera = getActiveCamera();
    if (!camera)
    {
        DEBUG_LOG("There is no camera");
        return false;
    }

    camera->GetFocalPoint(focalPoint);

    return true;
}

VoiLut QViewer::getCurrentVoiLut() const
{
    return VoiLut();
}

Vector3 QViewer::getViewPlaneNormal()
{
    Vector3 normal;
    getActiveCamera()->GetViewPlaneNormal(normal.x, normal.y, normal.z);
    return normal;
}

bool QViewer::scaleToFit3D(double topCorner[3], double bottomCorner[3], double marginRate)
{
    if (!hasInput())
    {
        return false;
    }

    // Calcular la width i height en coordenades de display
    double displayTopLeft[3], displayBottomRight[3];
    this->computeWorldToDisplay(topCorner[0], topCorner[1], topCorner[2], displayTopLeft);
    this->computeWorldToDisplay(bottomCorner[0], bottomCorner[1], bottomCorner[2], displayBottomRight);

    //We recalculate taking into account the display
    double width, height;
    width = fabs(displayTopLeft[0] - displayBottomRight[0]);
    height = fabs(displayTopLeft[1] - displayBottomRight[1]);

    // We adjust the image according to whether the window is narrower in width or height. If we want to see the whole region I chose, we will adjust for what
    // be narrower, if we adjusted for the widest we would lose image on the other side
    QSize size = this->getRenderWindowSize();
    double ratio = qMin(size.width() / width, size.height() / height);
    double factor = ratio * (1.0 - marginRate);
    
    return adjustCameraScaleFactor(factor);
}

void QViewer::fitRenderingIntoViewport()
{
    // First we get the bounds of the current rendered item in world coordinates
    double bounds[6];
    getCurrentRenderedItemBounds(bounds);
    
    double topCorner[3];
    double bottomCorner[3];
    for (int i = 0; i < 3; ++i)
    {
        topCorner[i] = bounds[i * 2];
        bottomCorner[i] = bounds[i * 2 + 1];
    }

    // Scaling the viewport to fit the current item bounds
    if (scaleToFit3D(topCorner, bottomCorner, m_defaultFitIntoViewportMarginRate))
    {
        render();
    }
}

VoiLutPresetsToolData* QViewer::getVoiLutData() const
{
    return m_voiLutData;
}

void QViewer::setVoiLutData(VoiLutPresetsToolData *voiLutData)
{
    if (m_voiLutData)
    {
        disconnect(m_voiLutData, 0, this, 0);
        delete m_voiLutData;
    }

    m_voiLutData = voiLutData;
    connect(m_voiLutData, SIGNAL(currentPresetChanged(VoiLut)), SLOT(setVoiLut(VoiLut)));
    connect(m_voiLutData, SIGNAL(presetSelected(VoiLut)), SLOT(setVoiLut(VoiLut)));
}

void QViewer::grabCurrentView()
{
    m_windowToImageFilter->Update();
    m_windowToImageFilter->Modified();

    vtkImageData *image = vtkImageData::New();
    image->ShallowCopy(m_windowToImageFilter->GetOutput());
    m_grabList << image;
}

void QViewer::resetView(const OrthogonalPlane &view)
{
    setCameraOrientation(view);
}

void QViewer::resetView(const AnatomicalPlane &desiredAnatomicalPlane)
{
    if (!hasInput())
    {
        return;
    }

    // HACK Disable rendering temporarily to enhance performance and avoid flickering
    enableRendering(false);

    // First we reset the view to the corresponding orthogonal plane
    const OrthogonalPlane &orthogonalPlane = getMainInput()->getCorrespondingOrthogonalPlane(desiredAnatomicalPlane);
    resetView(orthogonalPlane);
    
    // Then we apply the standard orientation for the desired projection
    setDefaultOrientation(desiredAnatomicalPlane);

    // HACK End of performance hack
    enableRendering(true);
    fitRenderingIntoViewport();

    emit anatomicalViewChanged(desiredAnatomicalPlane);
}

void QViewer::resetViewToAxial()
{
    resetView(AnatomicalPlane::Axial);
}

void QViewer::resetViewToCoronal()
{
    resetView(AnatomicalPlane::Coronal);
}

void QViewer::resetViewToSagital()
{
    resetView(AnatomicalPlane::Sagittal);
}

void QViewer::resetViewToAcquisitionPlane()
{
    resetView(OrthogonalPlane::XYPlane);
    fitRenderingIntoViewport();
}

void QViewer::enableContextMenu()
{
    m_contextMenuActive = true;
}

void QViewer::disableContextMenu()
{
    m_contextMenuActive = false;
}

void QViewer::contextMenuRelease()
{
    // Extret dels exemples de vtkEventQtSlotConnect

    // We get the position of the event
    QPoint point = this->getEventPosition();
    point /= this->devicePixelRatioF(); // Vtk pixels are real pixels, Qt wants logical pixels.

    // Remember to flip y
    QSize size = this->getRenderWindowSize();
    size /= this->devicePixelRatioF(); // Vtk pixels are real pixels, Qt wants logical pixels.
    point.setY(size.height() - point.y());



    // Map to global
    QPoint globalPoint = this->mapToGlobal(point);
    QContextMenuEvent contextMenuEvent(QContextMenuEvent::Mouse, point, globalPoint);
    this->contextMenuEvent(&contextMenuEvent);
}

void QViewer::updateVoiLutData()
{
    if (!hasInput())
    {
        return;
    }

    VoiLutHelper().initializeVoiLutData(m_voiLutData, getMainInput());
}

void QViewer::setCameraOrientation(const OrthogonalPlane &orientation)
{
    vtkCamera *camera = getActiveCamera();
    if (camera)
    {
        setCameraViewPlane(orientation);
        this->getRenderer()->ResetCamera();
        this->render();
    }
}

void QViewer::setCameraViewPlane(const OrthogonalPlane &viewPlane)
{
    vtkCamera *camera = getActiveCamera();
    if (!camera)
    {
        return;
    }

    this->setCurrentViewPlane(viewPlane);

    //We adjust the camera settings according to the view
    camera->SetFocalPoint(0.0, 0.0, 0.0);
    switch (this->getCurrentViewPlane())
    {
    case OrthogonalPlane::XYPlane:
        camera->SetViewUp(0.0, -1.0, 0.0);
        camera->SetPosition(0.0, 0.0, -1.0);
        break;

    case OrthogonalPlane::YZPlane:
        camera->SetViewUp(0.0, 0.0, 1.0);
        camera->SetPosition(1.0, 0.0, 0.0);
        break;

    case OrthogonalPlane::XZPlane:
        camera->SetViewUp(0.0, 0.0, 1.0);
        camera->SetPosition(0.0, -1.0, 0.0);
        break;
    }
}

bool QViewer::adjustCameraScaleFactor(double factor)
{
    if (MathTools::isNaN(factor))
    {
        DEBUG_LOG("Scale factor is NaN. No scale factor will be applied.");
        return false;
    }

    vtkRenderer *renderer = getRenderer();
    if (!renderer)
    {
        DEBUG_LOG("Renderer is NULL. No scale factor will be applied.");
        return false;
    }
    
    getActiveCamera()->Zoom(factor);
    
    if (this->getInteractor()->GetLightFollowCamera())
    {
        renderer->UpdateLightsGeometryToFollowCamera();
    }

    return true;
}

void QViewer::setDefaultOrientation(const AnatomicalPlane &anatomicalPlane)
{
    Q_UNUSED(anatomicalPlane);
}

QString QViewer::getInputIdentifier() const
{
    QString selectedItem;
    if (hasInput())
    {
        selectedItem = QString::number(getMainInput()->getIdentifier().getValue());
    }

    return selectedItem;
}

void QViewer::contextMenuEvent(QContextMenuEvent *menuEvent)
{
    if (m_contextMenuActive)
    {
        // It is possible that in some moments (when the patient is loaded and other dialogs leave)
        // no window is active or this is not a QApplicationMainWindow and it is a dialog,
        // so it can return us to NULL and sometimes it made us crack the app. This is how we heal ourselves in health
        // EVERYTHING It would be nice to check
        QApplicationMainWindow *mainWindow = QApplicationMainWindow::getActiveApplicationMainWindow();
        if (!mainWindow)
        {
            return;
        }

        // We update the input to show current studies
        m_patientBrowserMenu->setPatient(mainWindow->getCurrentPatient());

        QString selectedItem = getInputIdentifier();

        m_patientBrowserMenu->popup(menuEvent->globalPos(), selectedItem); //->globalPos() ?
        //zyq20240416
		//QPoint point = QCursor::pos();
        //menuEvent->globalPos() -> error???????!!! (windows 10:now 2020-09-09)
        //m_patientBrowserMenu->popup(QCursor::pos(), selectedItem);
    }
}

void QViewer::enableRendering(bool enable)
{
    m_isRenderingEnabled = enable;
}

PatientBrowserMenu* QViewer::getPatientBrowserMenu() const
{
    return m_patientBrowserMenu;
}

void QViewer::setAutomaticallyLoadPatientBrowserMenuSelectedInput(bool load)
{
	return;
    if (load)
    {
        connect(m_patientBrowserMenu, SIGNAL(selectedVolume(Volume*)), this, SLOT(setInputAndRender(Volume*)));
    }
    else
    {
        disconnect(m_patientBrowserMenu, SIGNAL(selectedVolume(Volume*)), this, SLOT(setInputAndRender(Volume*)));
    }
}

QViewer::ViewerStatus QViewer::getViewerStatus() const
{
    return m_viewerStatus;
}

QViewer::ViewerStatus QViewer::getPreviousViewerStatus() const
{
    return m_previousViewerStatus;
}

void QViewer::setViewerStatus(ViewerStatus status)
{
    if (m_viewerStatus != status)
    {
        m_previousViewerStatus = m_viewerStatus;
        m_viewerStatus = status;
        this->setCurrentWidgetByViewerStatus(status);
        this->initializeWorkInProgressByViewerStatus(status);
        emit viewerStatusChanged();
    }
}

void QViewer::setCurrentWidgetByViewerStatus(ViewerStatus status)
{
    if (status == NoVolumeInput || status == VisualizingVolume)
    {
        m_stackedLayout->setCurrentWidget(m_vtkWidget);
    }
    else
    {
        m_stackedLayout->setCurrentWidget(m_workInProgressWidget);
    }
}

void QViewer::initializeWorkInProgressByViewerStatus(ViewerStatus status)
{
    m_workInProgressWidget->reset();
    switch (status)
    {
    case NoVolumeInput:
    case VisualizingVolume:
        // Do nothing
        break;
        
    case DownloadingVolume:
        m_workInProgressWidget->setTitle(tr("Downloading related study..."));
        break;
        
    case LoadingVolume:
        m_workInProgressWidget->setTitle(tr("Loading data..."));
        break;
        
    case DownloadingError:
        m_workInProgressWidget->setTitle(tr("Error downloading related study"));
        m_workInProgressWidget->showError(QString());
        break;
        
    case LoadingError:
        m_workInProgressWidget->setTitle(tr("Error loading data"));
        break;

    case VisualizingError:
        m_workInProgressWidget->setTitle(tr("Error visualizing data"));
        break;
    }
}

void QViewer::setInputAndRender(Volume *volume)
{
    this->setInput(volume);
    this->render();
}

void QViewer::setVoiLut(const VoiLut &voiLut)
{
    Q_UNUSED(voiLut)
}

OrthogonalPlane QViewer::getCurrentViewPlane() const
{
    return m_currentViewPlane;
}

void QViewer::setCurrentViewPlane(const OrthogonalPlane &viewPlane)
{
    m_currentViewPlane = viewPlane;
}

void QViewer::handleNotEnoughMemoryForVisualizationError()
{
    setViewerStatus(VisualizingError);
    m_workInProgressWidget->showError(tr("There's not enough memory for the rendering process. Try to close all the open %1 windows, restart the application "
                                         "and try again. If the problem persists, adding more RAM memory or switching to a 64-bit operating system may solve the problem.")
                                      .arg(ApplicationNameString));
    // The cursor may have been changed by a tool that hasn't finished its operation and won't receive a mouse button release event,
    // thus the cursor is reset to its default form here
    // TODO Tools should be able to handle this situation by themselves
    unsetCursor();
    // In case of error during rendering the render window is left unusable, so we must create a new one
    setupRenderWindow();
}

void QViewer::setupRenderWindow()
{
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    // TODO getInteractor() forces m_vtkWiget to create a render window the first time just to return the interactor,
    // that render window is unused afterwards.
    //      Could this be improved?
    renderWindow->SetInteractor(getInteractor());
    renderWindow->AddRenderer(getRenderer());
    renderWindow->DoubleBufferOn();
    renderWindow->LineSmoothingOn();
    renderWindow->SetDPI(QGuiApplication::primaryScreen()->logicalDotsPerInch());

    if (!Settings().getValue(CoreSettings::DontForceMultiSampling).toBool())
    {
        // This is the default of VTK except on Mac due to some alleged problems in some models, and is needed to get smooth lines
        // The setting will allow to avoid those problems if they arise, at the cost of getting aliased lines
        renderWindow->SetMultiSamples(8);
    }

    // TODO This is needed for the rendering process to work correctly if coming from handleNotEnoughMemoryForVisualizationError().
    //      Alternatively the rendering process also works correctly after a Q2DViewer::restore().
    //      Why?
    getRenderWindow()->RemoveRenderer(getRenderer());

    m_vtkWidget->setRenderWindow(renderWindow);
//#ifdef VTK_90
//	m_vtkWidget->setRenderWindow(renderWindow);
//#else
//	m_vtkWidget->SetRenderWindow(renderWindow);
//#endif // VTK_90
    m_windowToImageFilter->SetInput(renderWindow);
}

Volume* QViewer::selectVolume(Volume* volume)
{
	/// The volume to select
	//static Volume *m_selectVolume = NULL;
	if (volume && m_selectVolume != volume)
	{
		m_selectVolume = volume;
	}
	return m_selectVolume;
}

PatientBrowserMenu * QViewer::getStaticBrowserMenu()
{
	static PatientBrowserMenu BrowserMenu(NULL);
	return &BrowserMenu;
}


};  // End namespace udg
