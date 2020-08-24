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

#ifndef UDGQVIEWER_H
#define UDGQVIEWER_H

#include "orthogonalplane.h"
#include "anatomicalplane.h"

#include <QWidget>
// Llista de captures de pantalla
#include <QList>
#include <vtkImageData.h>

// Fordward declarations
class QStackedLayout;
class QVTKWidget;
class vtkCamera;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkWindowToImageFilter;
class vtkEventQtSlotConnect;

namespace udg {

// Fordward declarations
class Volume;
class Series;
class Image;
class ToolProxy;
class VoiLutPresetsToolData;
class PatientBrowserMenu;
class QViewerWorkInProgressWidget;
class VoiLut;

/**
     Base class for all display windows
  */
class QViewer : public QWidget {
    Q_OBJECT
public:
    QViewer(QWidget *parent = 0);
    ~QViewer();

    /// Tipus de fitxer que pot desar
    enum FileType { PNG, JPEG, TIFF, DICOM, PNM, META, BMP };

    /// Estat del viewer
    enum ViewerStatus { NoVolumeInput, DownloadingVolume, DownloadingError, LoadingVolume, LoadingError, VisualizingVolume, VisualizingError };

    /// Retorna l'interactor renderer
    virtual vtkRenderWindowInteractor* getInteractor() const;

    /// Retorna el renderer
    virtual vtkRenderer* getRenderer() const;

    /// Returns the main input
    virtual Volume* getMainInput() const;

    /// Returns the total number of inputs of the viewer
    virtual int getNumberOfInputs() const;
    
    /// Returns true if viewer has some input set, false otherwise
    bool hasInput() const;
    
    /// Retorna el vtkRenderWindow
    vtkRenderWindow* getRenderWindow() const;

    /// Returns the size in screen coordinates (pixels?) Of the entire viewer viewport
    /// hide the vtkRenderWindow call :: GetSize ()
    QSize getRenderWindowSize() const;

    /// Methods for obtaining the current and / or previous cursor coordinate
    /// of the mouse during the last event
    /// The returned coordinates are in Display's coordinate system
    QPoint getEventPosition() const;
    QPoint getLastEventPosition() const;

    QPoint getWheelAngleDelta() const;
    
    /// Tells us if the viewer is active at that time
    /// @return True if active, false otherwise
    bool isActive() const;

    /// Ens retorna el ToolProxy del viewer
    ToolProxy* getToolProxy() const;

    ///Pass display coordinates to world coordinates and vice versa
    void computeDisplayToWorld(double x, double y, double z, double worldPoint[3]);
    void computeWorldToDisplay(double x, double y, double z, double displayPoint[3]);

    ///It gives us the world coordinate of the last (or previous) event captured
    void getEventWorldCoordinate(double worldCoordinate[3]);
    void getLastEventWorldCoordinate(double worldCoordinate[3]);

    /// It captures the current view and saves it in an internal structure
    void grabCurrentView();

    /// Save the list of captures in a file of various types with the
    /// filename base \ c baseName and in the format specified by \ c extension.
    /// Returns TRUE if there were images to save, FALSE otherwise
    bool saveGrabbedViews(const QString &baseName, FileType extension);

    /// Retorna el nombre de vistes capturades que estan desades
    int grabbedViewsCount()
    {
        return m_grabList.size();
    }

    /// Zoom in on the scene with the given factor
    /// @param factor Zoom factor we want to apply to the camera
    void zoom(double factor);

    /// Absolute zoom to the scene based on the factor value
    void absoluteZoom(double factor);

    /// Move the camera according to the motion vector we pass to it
    /// @param motionVector [] Motion vector that determines where and how far the camera moves
    void pan(double motionVector[3]);

    /// Zoom in on the box defined by the topCorner and bottomCorner parameters in world coordinates
    /// to fit the size of the window.
    /// The determination of which coordinate is considered "upper" or "lower" is relative depending on the point of view,
    /// the really important thing is that these coordinates are two opposite vertices within the box you want to define
    /// @param topCorner Top coordinate
    /// @param bottomCorner Lower coordinate opposite the upper
    /// @param marginRate value as a percentage of the margin you want to leave
    /// @return True if scale could be performed, false otherwise
    bool scaleToFit3D(double topCorner[3], double bottomCorner[3], double marginRate = 0.0);

    /// Returns the VOI LUT presets data for this viewer.
    VoiLutPresetsToolData* getVoiLutData() const;
    /// Sets the VOI LUT presets data for this viewer.
    void setVoiLutData(VoiLutPresetsToolData *voiLutData);

    /// Enables / disables renderings to take effect
    /// Useful in cases where we need to do several operations of
    /// visualization but we do not want these to become effective until we indicate it
    void enableRendering(bool enable);

    ///It returns the patient menu with which the input is chosen
    PatientBrowserMenu* getPatientBrowserMenu() const;

    /// Specifies whether selecting an input from the patient menu will automatically load it
    /// or, conversely, nothing will be done. By default it is enabled and the input will be loaded in the viewfinder.
    void setAutomaticallyLoadPatientBrowserMenuSelectedInput(bool load);

    /// Returns viewer status. Useful for whether the viewer is viewing data
    /// or loading ...
    ViewerStatus getViewerStatus() const;
    ViewerStatus getPreviousViewerStatus() const;

    ///Change viewer status
    void setViewerStatus(ViewerStatus status);

    /// Returns a unique string identifier of all inputs
    virtual QString getInputIdentifier() const;

    /// Returns the current Zoom Factor
    double getCurrentZoomFactor();

    /// Returns the current focal point of the active camera
    bool getCurrentFocalPoint(double focalPoint[3]);

    /// Returns the VOI LUT that is currently applied to the image in this viewer.
    /// The default implementation returns a default VoiLut.
    virtual VoiLut getCurrentVoiLut() const;

public slots:
    /// We indicate the input data
    virtual void setInput(Volume *volume) = 0;

    /// Manage the events you receive from the window
    void eventHandler(vtkObject *object, unsigned long vtkEvent, void *clientData, void *callData, vtkCommand *command);

    /// Force display execution
    void render();

    /// Assign whether this viewer is active, that is, what you are interacting with
    /// @param active
    void setActive(bool active);

    ///Delete all screenshots
    void clearGrabbedViews();

    /// Resets the view to a determined orthogonal plane
    virtual void resetView(const OrthogonalPlane &view);
    
    /// Resets the view to the specified anatomical plane
    void resetView(const AnatomicalPlane &anatomicalPlane);
    virtual void resetViewToAxial();
    virtual void resetViewToSagital();
    virtual void resetViewToCoronal();
    
    /// Resets the view to the original acquisition plane of the input volume
    void resetViewToAcquisitionPlane();

    /// Activate or deactivate the context menu
    void enableContextMenu();
    void disableContextMenu();

    /// Sets the VOI LUT for this viewer. The default implementation does nothing.
    virtual void setVoiLut(const VoiLut &voiLut);

    /// Fits the current rendered item into the viewport size
    void fitRenderingIntoViewport();

signals:
    /// We report the event received.
    /// \ TODO now we send the code in vtkCommand,
    /// but it could (or should) change for a mapping of ours
    void eventReceived(unsigned long eventID);

    /// Signal that is emitted when another series is chosen for the input
    void volumeChanged(Volume *volume);

    /// It is broadcast when the camera settings have changed
    void cameraChanged();

    /// Reports that the viewer has received an event that is considered to have been selected
    void selected(void);

    /// Reports that the zoom has changed
    void zoomFactorChanged(double factor);

    /// Reports that the image has been moved
    void panChanged(double *translation);

    /// Indicates that the viewfinder status has changed
    void viewerStatusChanged();

    /// Signal emitted when the anatomical view has changed
    void anatomicalViewChanged(const AnatomicalPlane &anatomicalPlane);

    /// Emitted when this viewer receives a double click event.
    void doubleClicked();

protected:
    /// Gets the bounds of the rendered item
    virtual void getCurrentRenderedItemBounds(double bounds[6]) = 0;

    virtual void contextMenuEvent(QContextMenuEvent *menuEvent);

    void contextMenuRelease();

    /// Updates the VOI LUT data.
    void updateVoiLutData();

    /// We set the camera orientation of the main renderer
    /// If the given parameter is not one of the valid listed values, it will do nothing
    /// @param orientation Orientation
    void setCameraOrientation(const OrthogonalPlane &orientation);

    /// Sets the orientation of the given camera
    void setCameraViewPlane(const OrthogonalPlane &viewPlane);

    /// Adjusts camera scale factor. Returns false if no scale could be performed, true otherwise.
    /// To be used by zoom(), scaleToFit3D() methods and the so
    bool adjustCameraScaleFactor(double factor);
    
    /// Sets the default rendered item orientation for the given anatomical plane
    virtual void setDefaultOrientation(const AnatomicalPlane &anatomicalPlane);
    
    ///It returns us the active camera by the main renderer, if any, NUL otherwise.
    vtkCamera* getActiveCamera();

    /// Gives us the world coordinate of the last (or previous) event captured
    /// @param worldCoordinate Variable where the coordinate will be returned
    /// @param current If true, it gives us the coordinate of the most recent event,
    /// if false, gives us the previous coordinate to the most recent event
    void getRecentEventWorldCoordinate(double worldCoordinate[3], bool current);

    virtual void setupInteraction();

    /// Returns the current view plane.
    virtual OrthogonalPlane getCurrentViewPlane() const;

    /// Sets the current view plane.
    virtual void setCurrentViewPlane(const OrthogonalPlane &viewPlane);

    /// Handles errors produced by lack of memory space for visualization.
    void handleNotEnoughMemoryForVisualizationError();

private slots:
    /// Slot used when a series has been selected with the PatientBrowserMenu
    /// Method that specifies an input followed by a call to the render () method
    /// TODO: Converted to virtual in order to be reimplemented by Q2DViewer by asynchronous upload
    virtual void setInputAndRender(Volume *volume);

private:
    /// Updates the current widget displayed on the screen from the viewer status
    void setCurrentWidgetByViewerStatus(ViewerStatus status);

    ///Initializes the QWorkInProgress widget from status
    void initializeWorkInProgressByViewerStatus(ViewerStatus status);

    /// Creates and configures the render window with the desired features.
    void setupRenderWindow();

protected:
    /// The volume to display
    Volume *m_mainVolume;

    /// The widget to display a vtk window with qt
    QVTKWidget *m_vtkWidget;

    /// La llista de captures de pantalla
    QList<vtkImageData*> m_grabList;

    ///Filter to connect what is displayed by the renderer in a pipeline,
    /// epr save the images in a file, for example
    vtkWindowToImageFilter *m_windowToImageFilter;

    /// Connector d'events vtk i slots qt
    vtkEventQtSlotConnect *m_vtkQtConnections;

    /// Renderer per defecte
    vtkRenderer *m_renderer;

    /// Indicates whether the context menu is active or not. By default it is always active
    bool m_contextMenuActive;

    /// ToolProxy
    ToolProxy *m_toolProxy;

    /// It will help us to control if the mouse has been moved between events
    bool m_mouseHasMoved;

    /// VOI LUT presets data for this viewer.
    VoiLutPresetsToolData *m_voiLutData;

    /// Indicates whether we should perform the render action or not
    bool m_isRenderingEnabled;

    /// Patient menu through which we can choose the input of the viewer
    PatientBrowserMenu *m_patientBrowserMenu;

    /// Widget that is displayed when an asynchronous job is being performed
    QViewerWorkInProgressWidget *m_workInProgressWidget;

    /// The default margin for fit into viewport. Should be between 0..1.
    double m_defaultFitIntoViewportMarginRate;

private:
    /// Current view plane: plane that is perpendicular to the camera pointing direction.
    OrthogonalPlane m_currentViewPlane;

    /// Indicates whether the viewer is active or not
    bool m_isActive;
    
    /// Last wheel scroll event angle delta.
    QPoint m_lastAngleDelta;

    ///Current and previous states of the current viewer
    ViewerStatus m_viewerStatus;
    ViewerStatus m_previousViewerStatus;

    /// Layout that allows us to create different widgets for different viewer states.
    QStackedLayout *m_stackedLayout;
};

};  // End namespace udg

#endif
