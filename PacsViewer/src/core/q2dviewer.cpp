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

#include "q2dviewer.h"
#include "drawer.h"
#include "volume.h"
#include "logging.h"
#include "image.h"
#include "series.h"
#include "imageplane.h"
#include "mathtools.h"
#include "imageorientationoperationsmapper.h"
#include "transferfunction.h"
#include "voilutpresetstooldata.h"
#include "coresettings.h"
#include "qviewerworkinprogresswidget.h"
#include "patientorientation.h"
#include "imageoverlay.h"
#include "drawerbitmap.h"
#include "filteroutput.h"
#include "blendfilter.h"
#include "imagepipeline.h"
#include "volumereadermanager.h"
#include "qviewercommand.h"
#include "renderqviewercommand.h"
#include "mammographyimagehelper.h"
#include "slicelocator.h"
#include "q2dviewerannotationhandler.h"
#include "volumedisplayunithandlerfactory.h"
#include "genericvolumedisplayunithandler.h"
#include "patientbrowsermenu.h"
#include "voiluthelper.h"
#include "sliceorientedvolumepixeldata.h"

// Qt
#include <QResizeEvent>
// Include basic vtk
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkPropPicker.h>
#include <QVTKWidget.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkMatrix4x4.h>

namespace udg {

const QString Q2DViewer::OverlaysDrawerGroup("Overlays");
const QString Q2DViewer::DummyVolumeObjectName("Dummy Volume");

Q2DViewer::Q2DViewer(QWidget *parent)
    : QViewer(parent), m_overlayVolume(0), m_blender(0), m_overlapMethod(Q2DViewer::Blend), m_rotateFactor(0), m_applyFlip(false),
      m_isImageFlipped(false), m_slabProjectionMode(VolumeDisplayUnit::Max), m_fusionBalance(50)
{
    m_displayUnitsFactory = new VolumeDisplayUnitHandlerFactory;
    initializeDummyDisplayUnit();
    m_volumeReaderManager = new VolumeReaderManager(this);
    m_inputFinishedCommand = NULL;

    connect(m_volumeReaderManager, SIGNAL(readingFinished()), SLOT(volumeReaderJobFinished()));
    connect(m_volumeReaderManager, SIGNAL(progress(int)), m_workInProgressWidget, SLOT(updateProgress(int)));
    //connect(m_patientBrowserMenu, SIGNAL(selectedVolumes(QList<Volume*>)), this, SLOT(setInputAndRender(QList<Volume*>)));

    // We create annotations and actors
    m_annotationsHandler = new Q2DViewerAnnotationHandler(this);
    initializeCamera();

    // We create the drawer, passing the this object as a viewfinder
    m_drawer = new Drawer(this);

    m_imageOrientationOperationsMapper = new ImageOrientationOperationsMapper();

    m_alignPosition = Q2DViewer::AlignCenter;

    //We initialize the shutter filter
    m_showDisplayShutters = true;
    m_overlaysAreEnabled = true;
}

Q2DViewer::~Q2DViewer()
{
    delete m_displayUnitsFactory;
    delete m_dummyDisplayUnit;
    // We delete other vtk objects in case they had to be created
    delete m_blender;

    removeViewerBitmaps();
    
    // TODO we had to first remove the drawer ahead of other objects
    // to fix ticket # 539, but this indicates that there is a problem with
    // design that makes it not robust enough. The order in which objects are deleted
    // shouldn't affect us
    // HACK We force the drawer to be deleted first
    delete m_drawer;
    delete m_imageOrientationOperationsMapper;

    deleteInputFinishedCommand();
}

void Q2DViewer::rotateClockWise(int times)
{
    // Must be at least 1 (+ 90º)
    if (times <= 0)
    {
        return;
    }

    rotate(times);

    applyImageOrientationChanges();
}

void Q2DViewer::rotateCounterClockWise(int times)
{
    //Must be at least 1 (-90º)
    if (times <= 0)
    {
        return;
    }

    rotate(-times);

    applyImageOrientationChanges();
}

void Q2DViewer::horizontalFlip()
{
    setFlip(true);

    applyImageOrientationChanges();
}

void Q2DViewer::verticalFlip()
{
    rotate(2);
    setFlip(true);

    applyImageOrientationChanges();
}

PatientOrientation Q2DViewer::getCurrentDisplayedImagePatientOrientation() const
{
    if (!getMainInput())
    {
        return PatientOrientation();
    }
	//zyq20240408
	//QViewer::selectVolume(getMainInput());
    // If we are not in the axial view (original acquisition) we will get
    // orientation through the first image
    int index = (getCurrentViewPlane() == OrthogonalPlane::XYPlane) ? getCurrentSlice() : 0;

    PatientOrientation originalOrientation;
    Image *image = getMainInput()->getImage(index);
    if (image)
    {
        originalOrientation = image->getPatientOrientation();
        if (originalOrientation.getDICOMFormattedPatientOrientation().isEmpty())
        {
            return PatientOrientation();
        }
    }
    else
    {
        DEBUG_LOG("The current image index is incorrect or there are no images in the volume. This should not happen in this method.");
        return PatientOrientation();
    }
    
    // We choose the labels we have taken as a reference according to the current view
    QString baseRowLabel;
    QString baseColumnLabel;
    switch (getCurrentViewPlane())
    {
    case OrthogonalPlane::XYPlane:
        baseRowLabel = originalOrientation.getRowDirectionLabel();
        baseColumnLabel = originalOrientation.getColumnDirectionLabel();
        break;

    case OrthogonalPlane::YZPlane:
        baseRowLabel = originalOrientation.getColumnDirectionLabel();
        // TODO We have the normal “upside down”, it should actually be the opposite
        baseColumnLabel = PatientOrientation::getOppositeOrientationLabel(originalOrientation.getNormalDirectionLabel());
        break;

    case OrthogonalPlane::XZPlane:
        baseRowLabel = originalOrientation.getRowDirectionLabel();
        // TODOWe have the normal “upside down”, it should actually be the opposite
        baseColumnLabel = PatientOrientation::getOppositeOrientationLabel(originalOrientation.getNormalDirectionLabel());
        break;
    }

    //You will now need to choose the appropriate labels based on the rotations and flips
    QString rowLabel;
    QString columnLabel;
    int absoluteRotateFactor = (4 + m_rotateFactor) % 4;
    if (getCurrentViewPlane() == OrthogonalPlane::YZPlane || getCurrentViewPlane() == OrthogonalPlane::XZPlane)
    {
        // HACK FLIP At the moment we need to do this trick. During refactoring it would be necessary
        // see if it can be done in a better way
        if (m_isImageFlipped)
        {
            absoluteRotateFactor = (absoluteRotateFactor + 2) % 4;
        }
    }
    switch (absoluteRotateFactor)
    {
    case 0:
        rowLabel = baseRowLabel;
        columnLabel = baseColumnLabel;
        break;

    case 1:
        rowLabel = PatientOrientation::getOppositeOrientationLabel(baseColumnLabel);
        columnLabel = baseRowLabel;
        break;

    case 2:
        rowLabel = PatientOrientation::getOppositeOrientationLabel(baseRowLabel);
        columnLabel = PatientOrientation::getOppositeOrientationLabel(baseColumnLabel);
        break;

    case 3:
        rowLabel = baseColumnLabel;
        columnLabel = PatientOrientation::getOppositeOrientationLabel(baseRowLabel);
        break;
    }

    if (m_isImageFlipped)
    {
        rowLabel = PatientOrientation::getOppositeOrientationLabel(rowLabel);
    }
    
    PatientOrientation patientOrientation;
    patientOrientation.setLabels(rowLabel, columnLabel);

    return patientOrientation;
}

QString Q2DViewer::getCurrentAnatomicalPlaneLabel() const
{
    return AnatomicalPlane::getLabelFromPatientOrientation(getCurrentDisplayedImagePatientOrientation());
}

AnatomicalPlane Q2DViewer::getCurrentAnatomicalPlane() const
{
    return AnatomicalPlane::getPlaneFromPatientOrientation(getCurrentDisplayedImagePatientOrientation());
}

void Q2DViewer::setDefaultOrientation(const AnatomicalPlane &anatomicalPlane)
{
    if (!hasInput())
    {
        return;
    }

    /// We apply the standard orientation for the desired projection
    /// unless original acquisition is axial and is the same as the desired one
    /// because when the patient is acquired in prono position
    /// we don't want to change the acquisition orientation and thus respect the acquired one
    AnatomicalPlane acquisitionPlane = getMainInput()->getAcquisitionPlane();
    if (acquisitionPlane != AnatomicalPlane::Axial || acquisitionPlane != anatomicalPlane)
    {
        PatientOrientation desiredOrientation = anatomicalPlane.getDefaultRadiologicalOrienation();
        setImageOrientation(desiredOrientation);
    }
}

void Q2DViewer::updatePreferredImageOrientation()
{
    Image *image = getCurrentDisplayedImage();
    
    if (!image)
    {
        return;
    }
    
    /// There are studies that are of the MG modality that should not be targeted.
    /// Some exceptions have been added to be able to control them.
    MammographyImageHelper mammographyImageHelper;
    if (mammographyImageHelper.isStandardMammographyImage(image))
    {
        PatientOrientation desiredOrientation = mammographyImageHelper.getImageOrientationPresentation(image);

        //We apply the orientation we want
        setImageOrientation(desiredOrientation);
    }
}

double Q2DViewer::getCurrentSliceThickness() const
{
    return getMainDisplayUnit()->getSliceThickness();
}

int Q2DViewer::getMinimumSlice() const
{
    return getMainDisplayUnit()->getMinimumSlice();
}

int Q2DViewer::getMaximumSlice() const
{
    return getMainDisplayUnit()->getMaximumSlice();
}

int Q2DViewer::getNumberOfSlices() const
{
    return getMainDisplayUnit()->getNumberOfSlices();
}

int Q2DViewer::getNumberOfPhases() const
{
    Volume *mainVolume = getMainInput();
    if (mainVolume)
    {
        return mainVolume->getNumberOfPhases();
    }
    else
    {
        return 1;
    }
}

int Q2DViewer::getNumberOfPhasesFromInput(int i) const
{
    Volume *input = getInput(i);

    if (input)
    {
        return input->getNumberOfPhases();
    }
    else
    {
        return 1;
    }
}

bool Q2DViewer::hasPhases() const
{
    return getNumberOfPhases() > 1;
}

bool Q2DViewer::doesInputHavePhases(int i) const
{
    return getNumberOfPhasesFromInput(i) > 1;
}

bool Q2DViewer::isInputVisible(int i) const
{
    return getDisplayUnit(i)->isVisible();
}

void Q2DViewer::initializeCamera()
{
    vtkCamera *camera = getActiveCamera();
    Q_ASSERT(camera);
    camera->ParallelProjectionOn();
}

void Q2DViewer::initializeDummyDisplayUnit()
{
    m_dummyDisplayUnit = new VolumeDisplayUnit;
    m_dummyDisplayUnit->setVisible(false);
}

void Q2DViewer::addImageActors()
{
    if (m_displayUnitsHandler)
    {
        getRenderer()->AddViewProp(m_displayUnitsHandler->getImageProp());
    }
}

void Q2DViewer::removeImageActors()
{
    if (m_displayUnitsHandler)
    {
        getRenderer()->RemoveViewProp(m_displayUnitsHandler->getImageProp());
    }
}

void Q2DViewer::setInput(Volume *volume)
{
    if (!volume)
    {
        return;
    }

    m_volumeReaderManager->cancelReading();
    deleteInputFinishedCommand();

    setNewVolumes(QList<Volume*>() << volume);
}

void Q2DViewer::setInputAsynchronously(Volume *volume, QViewerCommand *inputFinishedCommand)
{
    if (!volume)
    {
        return;
    }
    DEBUG_LOG(QString("Q2DViewer::setInputAsynchronously to Volume %1").arg(volume->getIdentifier().getValue()));

    setInputAsynchronously(QList<Volume*>() << volume, inputFinishedCommand);
}

void Q2DViewer::setInputAsynchronously(const QList<Volume *> &volumes, QViewerCommand *inputFinishedCommand)
{
    m_volumeReaderManager->cancelReading();
    setInputFinishedCommand(inputFinishedCommand);

    bool allowAsynchronousVolumeLoading = Settings().getValue(CoreSettings::AllowAsynchronousVolumeLoading).toBool();
    bool thereAreVolumesNotLoaded = false;
    int i = 0;
    while (i < volumes.size() && !thereAreVolumesNotLoaded)
    {
        thereAreVolumesNotLoaded = !volumes.at(i)->isPixelDataLoaded();
        i++;
    }
    if (thereAreVolumesNotLoaded && allowAsynchronousVolumeLoading)
    {
        loadVolumesAsynchronously(volumes);
    }
    else
    {
        setNewVolumesAndExecuteCommand(volumes);
    }
}

void Q2DViewer::setInputAndRender(Volume *volume)
{
    setInputAsynchronously(QList<Volume*>() << volume);
}

void Q2DViewer::setInputAndRender(const QList<Volume*> &volumes)
{
    RenderQViewerCommand *command = new RenderQViewerCommand(this);
    setInputAsynchronously(volumes, command);
}

void Q2DViewer::executeInputFinishedCommand()
{
    if (m_inputFinishedCommand)
    {
        m_inputFinishedCommand->execute();
    }
    else
    {
        // If no command is defined, we render by default
        render();
    }
}

void Q2DViewer::setInputFinishedCommand(QViewerCommand *command)
{
    /// We make sure that the new command they pass us is not the same as we currently have
    if (command != m_inputFinishedCommand)
    {
        deleteInputFinishedCommand();
        m_inputFinishedCommand = command;
    }
}

void Q2DViewer::deleteInputFinishedCommand()
{
    delete m_inputFinishedCommand;
    m_inputFinishedCommand = NULL;
}

void Q2DViewer::loadVolumeAsynchronously(Volume *volume)
{
    loadVolumesAsynchronously(QList<Volume*>() << volume);
}

void Q2DViewer::loadVolumesAsynchronously(const QList<Volume*> &volumes)
{
    setViewerStatus(LoadingVolume);

    m_volumeReaderManager->readVolumes(volumes);

    /// TODO:At the moment we have no choice but to specify a fake volume.
    /// The rest of the viewer (and those that depend on it) are expected
    /// have a volume loaded after calling setInput.
    /// We also have the problem that in order for it to
    /// appear in the right-click menu as selected, you have to put the same id.
    QList<Volume*> dummies;
    foreach (Volume *volume, volumes) {
        Volume *dummyVolume = getDummyVolumeFromVolume(volume);
        dummyVolume->setIdentifier(volume->getIdentifier());
        dummies << dummyVolume;
    }

    setNewVolumes(dummies, false);
}

void Q2DViewer::volumeReaderJobFinished()
{
    if (m_volumeReaderManager->readingSuccess())
    {
        setNewVolumesAndExecuteCommand(m_volumeReaderManager->getVolumes());
    }
    else
    {
        setViewerStatus(LoadingError);
        m_workInProgressWidget->showError(m_volumeReaderManager->getLastErrorMessageToUser());
    }
}

void Q2DViewer::setNewVolumesAndExecuteCommand(const QList<Volume*> &volumes)
{
    try
    {
        setNewVolumes(volumes);
        emit newVolumesRendered();
    }
    catch (...)
    {
        handleNotEnoughMemoryForVisualizationError();

        /// Use volumes.first() instead of getMainVolume() because the main volume has probably been deleted
        Volume *dummyVolume = getDummyVolumeFromVolume(volumes.first());
        dummyVolume->setIdentifier(volumes.first()->getIdentifier());
        setNewVolumes(QList<Volume*>() << dummyVolume, false);
    }
}

Volume* Q2DViewer::getDummyVolumeFromVolume(Volume *volume)
{
    /// TODO:We are losing memory for the life of 2dviewer, the dummy from before would have to be deleted
    Volume *newVolume = new Volume(this);
    newVolume->setObjectName(DummyVolumeObjectName);
    newVolume->setImages(volume->getImages());
    newVolume->convertToNeutralVolume();

    return newVolume;
}

void Q2DViewer::setNewVolumes(const QList<Volume*> &volumes, bool setViewerStatusToVisualizingVolume)
{
    if (volumes.isEmpty())
    {
        DEBUG_LOG("Received an empty volume list -> doing nothing");
        WARN_LOG("Received an empty volume list -> doing nothing");
        return;
    }

    // We must first set the status in VisualizingVolume so that the QVTKWidget can get the size it deserves
    // if not, we may find that it has not yet been displayed and has undefined sizes making the image not show full # 1434
    // EVERYTHING: fitRenderingIntoViewport () should be independent of whether or not the QVTKWidget is being displayed
    if (setViewerStatusToVisualizingVolume)
    {
        setViewerStatus(VisualizingVolume);
    }

    // HACK
    // Disable rendering to improve the efficiency of setInput as it is otherwise rendered multiple times
    enableRendering(false);
    
    // Check if there was input before and take the appropiate actions
    if (hasInput())
    {
        //When making a new input, the distances saved by the drawer do not make sense, so they are deleted
        m_drawer->removeAllPrimitives();

        Volume *mainInput = getMainInput();

        if (mainInput != volumes.first())
        {
            // By changing the volume, we eliminate overlays that we may have had before
            removeViewerBitmaps();
        }

        // If the previous volume was a dummy, delete it
        // Warning: assuming that we don't assign the same dummy again
        if (mainInput->objectName() == DummyVolumeObjectName)
        {
            delete mainInput;
            getMainDisplayUnit()->setVolume(0);
        }
    }

    // Init new input
    removeImageActors();
    m_displayUnitsHandler = m_displayUnitsFactory->createVolumeDisplayUnitHandler(volumes);

    getDisplayUnit(0)->setVoiLutData(getVoiLutData());
    for (int i = 1; i < getNumberOfInputs(); i++)
    {
        getDisplayUnit(i)->setVoiLutData(new VoiLutPresetsToolData(this));
    }

    addImageActors();

    setCurrentViewPlane(OrthogonalPlane::XYPlane);
    m_alignPosition = Q2DViewer::AlignCenter;

    // This is done to destroy the blender in case there is already some input and you want to change it
    delete m_blender;
    m_blender = 0;

    updateDisplayShutterMask();

    printVolumeInformation();

    m_annotationsHandler->updateAnnotations(MainInformationAnnotation | AdditionalInformationAnnotation);

    loadOverlays(volumes.first());

    // Reset the view to the acquisition plane
    resetViewToAcquisitionPlane();

    // HACK
    // Rendering is activated again so that the scene is rendered
    enableRendering(true);

    m_slabProjectionMode = VolumeDisplayUnit::Max;

    executeInputFinishedCommand();
    //We indicate the change in volume
    emit volumeChanged(getMainInput());
}

void Q2DViewer::removeViewerBitmaps()
{
    // We deleted the bitmaps we had so far
    foreach (DrawerBitmap *bitmap, m_viewerBitmaps)
    {
        bitmap->decreaseReferenceCount();
        delete bitmap;
    }
    m_viewerBitmaps.clear();
}

void Q2DViewer::loadOverlays(Volume *volume)
{
    if (!volume)
    {
        return;
    }

    if (volume->objectName() == DummyVolumeObjectName)
    {
        return;
    }

    double volumeSpacing[3];
    volume->getSpacing(volumeSpacing);
    double volumeOrigin[3];
    volume->getOrigin(volumeOrigin);
    
    int numberOfSlices = volume->getNumberOfSlicesPerPhase();
    int numberOfPhases = volume->getNumberOfPhases();
    for (int sliceIndex = 0; sliceIndex < numberOfSlices; ++sliceIndex)
    {
        for (int phaseIndex = 0; phaseIndex < numberOfPhases; ++ phaseIndex)
        {
            Image *image = volume->getImage(sliceIndex, phaseIndex);
            if (!image)
            {
                ERROR_LOG(QString("Unexpected error trying to access image with indexes: %1 (slice), %2 (phase) of current volume")
                          .arg(sliceIndex).arg(phaseIndex));
                DEBUG_LOG(QString("Unexpected error trying to access image with indexes: %1 (slice), %2 (phase) of current volume")
                          .arg(sliceIndex).arg(phaseIndex));
            }
            else
            {
                if (image->hasOverlays())
                {
                    // Calculem l'origen del bitmap corresponent a aquesta imatge
                    double imageOrigin[3];
                    imageOrigin[0] = volumeOrigin[0];
                    imageOrigin[1] = volumeOrigin[1];
                    imageOrigin[2] = volumeOrigin[2] + sliceIndex * volumeSpacing[2];
                    // Creem els bitmaps
                    foreach(const ImageOverlay &overlay, image->getOverlaysSplit())
                    {
                        DrawerBitmap *overlayBitmap = overlay.getAsDrawerBitmap(imageOrigin, volumeSpacing);
                        //Initially it will not be, according to the slice we are in the Drawer will decide on its visibility
                        overlayBitmap->setVisibility(false);
                        // The primitive cannot be deleted with the tools
                        overlayBitmap->setErasable(false);
                        overlayBitmap->increaseReferenceCount();
                        getDrawer()->draw(overlayBitmap, OrthogonalPlane::XYPlane, sliceIndex);
                        getDrawer()->addToGroup(overlayBitmap, OverlaysDrawerGroup);
                        m_viewerBitmaps << overlayBitmap;
                    }
                }
            }
        }
    }

    if (!m_overlaysAreEnabled)
    {
        showImageOverlays(m_overlaysAreEnabled);
    }
}

void Q2DViewer::setOverlayInput(Volume *volume)
{
    m_overlayVolume = volume;
    if (m_overlapMethod == Blend)
    {
        if (!m_blender)
        {
            m_blender = new BlendFilter();
            m_blender->setBase(getMainInput()->getVtkData());
        }
        m_blender->setOverlay(m_overlayVolume->getVtkData());
        m_blender->setOverlayOpacity(1.0 - m_overlayOpacity);
    }
    updateOverlay();
    emit overlayChanged();
}

Volume* Q2DViewer::getOverlayInput()
{
    return m_overlayVolume;
}

void Q2DViewer::updateOverlay()
{
    if (!hasInput())
    {
        return;
    }
    
    switch (m_overlapMethod)
    {
    case None:
        // We update the pipeline
        getMainDisplayUnit()->getImagePipeline()->setInput(getMainInput()->getVtkData());
        // ALL of this procedure may be insufficient,
        // the pipeline should be unified in the same method
        break;

    case Blend:
        // TODO Review how to give a blending input to the viewer
        // This procedure may be insufficient in terms of how the pipeline is built
        m_blender->update();
        getMainDisplayUnit()->getImagePipeline()->setInput(m_blender->getOutput());
        break;
    }

    emit overlayModified();
}

void Q2DViewer::setOverlayOpacity(double opacity)
{
    m_overlayOpacity = opacity;
}

Volume* Q2DViewer::getMainInput() const
{
    return getMainDisplayUnit()->getVolume();
}

Volume* Q2DViewer::getInput(int i) const
{
    return getDisplayUnit(i)->getVolume();
}

int Q2DViewer::getNumberOfInputs() const
{
    if (m_displayUnitsHandler.isNull())
    {
        return 0;
    }
    else
    {
        return m_displayUnitsHandler->getNumberOfInputs();
    }
}

void Q2DViewer::resetView(const OrthogonalPlane &view)
{
    double slabThickness = getSlabThickness();

    setCurrentViewPlane(view);
    m_annotationsHandler->updateAnnotations(VoiLutAnnotation);
    
    // We reset the camera to defaults
    m_rotateFactor = 0;
    setFlip(false);
    m_isImageFlipped = false;
    m_alignPosition = Q2DViewer::AlignCenter;
    
    resetCamera();
    
    if (hasInput())
    {
        // Adapt the camera to the new view plane in order to make actors visible
        double bounds[6];
        getCurrentRenderedItemBounds(bounds);
        getRenderer()->ResetCamera(bounds);

        // We calculate the slice to be shown according to the chosen view
        int initialSliceIndex = this->getMinimumSlice();
        if (getCurrentViewPlane() == OrthogonalPlane::YZPlane || getCurrentViewPlane() == OrthogonalPlane::XZPlane)
        {
            initialSliceIndex = (getMinimumSlice() + getMaximumSlice()) / 2;
        }
        setSlice(initialSliceIndex);

        // Set appropriate zoom level
        fitRenderingIntoViewport();

        // Set slab thickness again to ensure that annotations are updated and signals emitted if necessary
        setSlabThickness(slabThickness);
    }

    emit viewChanged(getCurrentViewPlane());
}

// This method needs to be redefined in Q2DViewer because the
// OrthogonalPlane version is redefined and it would hide this version otherwise
// (see https://isocpp.org/wiki/faq/strange-inheritance#hiding-rule)
void Q2DViewer::resetView(const AnatomicalPlane &anatomicalPlane)
{
    QViewer::resetView(anatomicalPlane);
}

void Q2DViewer::updateCamera()
{
    if (hasInput())
    {
        vtkCamera *camera = getActiveCamera();
        Q_ASSERT(camera);

        double roll = 0.0;
        switch (getCurrentViewPlane())
        {
        case OrthogonalPlane::XYPlane:
            if (m_isImageFlipped)
            {
                roll = m_rotateFactor * 90. + 180.;
            }
            else
            {
                roll = -m_rotateFactor * 90. + 180.;
            }
            break;

        case OrthogonalPlane::YZPlane:
            if (m_isImageFlipped)
            {
                roll = m_rotateFactor * 90. - 90.;
            }
            else
            {
                roll = -m_rotateFactor * 90. - 90.;
            }
            break;

        case OrthogonalPlane::XZPlane:
            if (m_isImageFlipped)
            {
                roll = m_rotateFactor * 90.;
            }
            else
            {
                roll = -m_rotateFactor * 90.;
            }
            break;
        }
        camera->SetRoll(roll);

        if (m_applyFlip)
        {
            // Alternative 1)
            // EVERYTHING This is how we move the camera, but the image would not move
            // perhaps implementing the Azimuth method in our own way and taking it as a center
            // the center of the image. Another possibility is to counteract the displacement of the
            // camera on the axis on which it occurred
            camera->Azimuth(180);
            switch (getCurrentViewPlane())
            {
            // HACK This hack is related to getCurrentDisplayedImageOrientationLabels ()
            // is a little trick for the image to be oriented correctly. It would be necessary
            // see if in refactoring we can do it in a better way
            case OrthogonalPlane::YZPlane:
            case OrthogonalPlane::XZPlane:
                rotate(-2);
                break;

            default:
                break;
            }

            getRenderer()->ResetCameraClippingRange();
            setFlip(false);
            m_isImageFlipped = !m_isImageFlipped;
        }
        emit cameraChanged();
        m_annotationsHandler->updateAnnotations();
    }
    else
    {
        DEBUG_LOG("Trying to update camera rotation without giving an input before...");
    }
}

void Q2DViewer::resetCamera()
{
    setCameraViewPlane(getCurrentViewPlane());
}

void Q2DViewer::setSlice(int value)
{
    updateSliceToDisplay(value, SpatialDimension);
}

void Q2DViewer::setPhase(int value)
{
    updateSliceToDisplay(value, TemporalDimension);
}

void Q2DViewer::setPhaseInVolume(int index, int phase)
{
    VolumeDisplayUnit *unit = this->getDisplayUnit(index);

    if (unit == m_dummyDisplayUnit)
    {
        DEBUG_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        WARN_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        return;
    }

    unit->setPhase(phase);
    // This also resets the camera clipping range (needed)
    this->updateImageSlices();
    this->render();
}

void Q2DViewer::updateSliceToDisplay(int value, SliceDimension dimension)
{
    if (hasInput())
    {
        int oldSlice = getCurrentSlice();
        int oldPhase = getCurrentPhase();

        // First update the index of the corresponding dimension
        switch (dimension)
        {
        case SpatialDimension:
            getMainDisplayUnit()->setSlice(value);
            updateSecondaryVolumesSlices();
            break;

        case TemporalDimension:
            getMainDisplayUnit()->setPhase(value);
            break;
        }

        // Then update display (image and associated annotations)
        updateImageSlices();

        // The display shutter must be updated before the default presets
        if (dimension == SpatialDimension)
        {
            m_annotationsHandler->updateAnnotations();

            if (isThickSlabActive())
            {
                // When thickslab enabled, erase all drawn primitives as we cannot associate slabs with them
                // TODO Maybe it should be also applied on TemporalDimension
                getDrawer()->removeAllPrimitives();
            }

            updateDisplayShutterMask();
        }

        updateCurrentImageDefaultPresetsInAllInputsOnOriginalAcquisitionPlane();
        m_annotationsHandler->updateAnnotations(MainInformationAnnotation | AdditionalInformationAnnotation | SliceAnnotation);
        updatePreferredImageOrientation();

        // Finally we emit the signal of the changed value and render the scene
        switch (dimension)
        {
        case SpatialDimension:
            if (getCurrentSlice() != oldSlice)
            {
                emit sliceChanged(getCurrentSlice());
            }
            break;

        case TemporalDimension:
            if (getCurrentPhase() != oldPhase)
            {
                emit phaseChanged(getCurrentPhase());
            }
            break;
        }
        
        render();
    }
}

void Q2DViewer::updateSecondaryVolumesSlices()
{
    if (getNumberOfInputs() <= 1)
    {
        return;
    }

    SliceLocator sliceLocator;
    sliceLocator.setPlane(getCurrentViewPlane());

    for (int i = 1; i < getNumberOfInputs(); i++)
    {
        sliceLocator.setVolume(getDisplayUnit(i)->getVolume());
        int nearestSlice = sliceLocator.getNearestSlice(getCurrentImagePlane());

        if (nearestSlice >= 0)
        {
            getDisplayUnit(i)->setVisible(true);
            getDisplayUnit(i)->setSlice(nearestSlice);
        }
        else
        {
            getDisplayUnit(i)->setVisible(false);
            getDisplayUnit(i)->setSlice(0);
        }
    }
}

void Q2DViewer::setOverlapMethod(OverlapMethod method)
{
    m_overlapMethod = method;
    
    if (m_overlapMethod == Q2DViewer::None)
    {
        getMainDisplayUnit()->getImagePipeline()->setInput(getMainInput()->getVtkData());
    }
}

void Q2DViewer::resizeEvent(QResizeEvent *resize)
{
    Q_UNUSED(resize);
    if (hasInput())
    {
        enableRendering(false);
        fitRenderingIntoViewport();
        switch (m_alignPosition)
        {
        case AlignRight:
            alignRight();
            break;

        case AlignLeft:
            alignLeft();
            break;

        default:
            break;
        }
        enableRendering(true);
        this->render();
    }
}

const TransferFunction& Q2DViewer::getTransferFunction() const
{
    return getMainDisplayUnit()->getTransferFunction();
}

void Q2DViewer::setTransferFunction(const TransferFunction &transferFunction)
{
    getMainDisplayUnit()->setTransferFunction(transferFunction);
}

void Q2DViewer::clearTransferFunction()
{
    getMainDisplayUnit()->clearTransferFunction();
}

VoiLut Q2DViewer::getCurrentVoiLut() const
{
    return getVoiLutData()->getCurrentPreset();
}

VoiLut Q2DViewer::getCurrentVoiLutInVolume(int index) const
{
    VolumeDisplayUnit *unit = this->getDisplayUnit(index);

    if (unit == m_dummyDisplayUnit)
    {
        WARN_LOG(QString("No volume at index %1. Returning default-constructed VOI LUT.").arg(index));
        return VoiLut();
    }
    else
    {
        return unit->getVoiLutData()->getCurrentPreset();
    }
}

int Q2DViewer::getCurrentSlice() const
{
    return getMainDisplayUnit()->getSlice();
}

int Q2DViewer::getCurrentPhase() const
{
    return getCurrentPhaseOnInput(0);
}

int Q2DViewer::getCurrentSliceOnInput(int i) const
{
    return getDisplayUnit(i)->getSlice();
}

int Q2DViewer::getCurrentPhaseOnInput(int i) const
{
    return getDisplayUnit(i)->getPhase();
}

Image* Q2DViewer::getCurrentDisplayedImage() const
{
    return getMainDisplayUnit()->getCurrentDisplayedImage();
}

Image* Q2DViewer::getCurrentDisplayedImageOnInput(int i) const
{
    return getDisplayUnit(i)->getCurrentDisplayedImage();
}

ImagePlane* Q2DViewer::getCurrentImagePlane(bool vtkReconstructionHack)
{
    if (!hasInput())
    {
        return 0;
    }

    return getMainInput()->getImagePlane(getCurrentSlice(), getCurrentViewPlane(), vtkReconstructionHack);
}

bool Q2DViewer::getDicomWorldCoordinates(const double xyz[3], double dicomWorldPosition[4])
{
    int index[3];
    ImagePlane *currentPlane = NULL;
    bool result = false;

    // 2.- Find the voxel index in the DICOM
    this->getMainInput()->computeCoordinateIndex(xyz, index);

    // 3.- We need the image to which the point belongs in order to find the dicom image that contains the plan information.
    double *spacing = this->getMainInput()->getSpacing();

    currentPlane = getCurrentImagePlane();

    if (currentPlane)
    {
        // 3.- We construct the mat to map the DICOM pixel index to a point in the real world
        std::array<double, 3> currentPlaneRowVector = Vector3(currentPlane->getImageOrientation().getRowVector());
        std::array<double, 3> currentPlaneColumnVector = Vector3(currentPlane->getImageOrientation().getColumnVector());
        std::array<double, 3> currentPlaneOrigin = currentPlane->getOrigin();

        int xIndex, yIndex, zIndex;
        getCurrentViewPlane().getXYZIndexes(xIndex, yIndex, zIndex);

        vtkMatrix4x4 *projectionMatrix = vtkMatrix4x4::New();
        projectionMatrix->Identity();
        for (int row = 0; row < 3; row++)
        {
            projectionMatrix->SetElement(row, xIndex, (currentPlaneRowVector[row]) * spacing[xIndex]);
            projectionMatrix->SetElement(row, yIndex, (currentPlaneColumnVector[row]) * spacing[yIndex]);
            projectionMatrix->SetElement(row, zIndex, 0.0);
            projectionMatrix->SetElement(row, 3, currentPlaneOrigin[row]);
        }

        // 3.- Real-world pixel index mapping
        dicomWorldPosition[0] = (double)index[0];
        dicomWorldPosition[1] = (double)index[1];
        dicomWorldPosition[2] = (double)index[2];
        dicomWorldPosition[3] = 1.0;
        // Matriu * punt
        projectionMatrix->MultiplyPoint(dicomWorldPosition, dicomWorldPosition);
        projectionMatrix->Delete();

        result = true;
    }
    else
    {
        DEBUG_LOG("The requested plane is not available, maybe due to poor multiframe support.");
        INFO_LOG("The position of the 3D cursor cannot be updated because we cannot get the corresponding plane.");
    }

    delete currentPlane;

    return result;
}

void Q2DViewer::projectDICOMPointToCurrentDisplayedImage(const double pointToProject[3], double projectedPoint[3], bool vtkReconstructionHack)
{
    // HERE WE ADD THE SOURCE AS IT IS + THE VTK DISPLACEMENT ERROR
    //
    // The projection is done as follows:
    // First a conventional projection of the point on the current plan (DICOM) is made
    // Since VTK coordinate mapping goes to its ball, we need to correct the displacement
    // introduced by VTK with respect to the "real" coordinates of DICOM
    // This shift will consist of re-adding the source of the foreground of the volume
    // in principle, doing this with the m_mainVolume source would also be correct

    ImagePlane *currentPlane = getCurrentImagePlane(vtkReconstructionHack);
    if (currentPlane)
    {
        // First we project the point on the current image plane
        std::array<double, 3> planeProjectedPoint = currentPlane->projectPoint(pointToProject,
                                                                               getCurrentViewPlane() == OrthogonalPlane::YZPlane && vtkReconstructionHack);

        //
        // VTK CORRECTION!
        //
        // From here it is necessary to correct the error introduced by the mapping made by the vtk
        // add the source of the first image, or what would be the same, the source of m_mainVolume
        //
        // TODO test if with the source of m_mainVolume also works fine
        Image *firstImage = getMainInput()->getImage(0);
        const double *ori = firstImage->getImagePositionPatient();
        int xIndex, yIndex, zIndex;
        getCurrentViewPlane().getXYZIndexes(xIndex, yIndex, zIndex);
        
        projectedPoint[xIndex] = planeProjectedPoint[0] + ori[xIndex];
        projectedPoint[yIndex] = planeProjectedPoint[1] + ori[yIndex];
        projectedPoint[zIndex] = planeProjectedPoint[2] + ori[zIndex];
    }
    else
    {
        DEBUG_LOG("There are no valid plans");
    }
}

void Q2DViewer::absolutePan(double motionVector[3])
{
    double projectedPoint[3];
    this->projectDICOMPointToCurrentDisplayedImage(motionVector, projectedPoint);

    double relativeMotionVector[3];
    double currentPosition[3];

    vtkCamera *camera = getActiveCamera();
    if (!camera)
    {
        DEBUG_LOG("There is no camera");
        return;
    }
    camera->GetFocalPoint(currentPosition);

    int x, y, z;
    getCurrentViewPlane().getXYZIndexes(x, y, z);

    relativeMotionVector[x] = projectedPoint[x] - currentPosition[x];
    relativeMotionVector[y] = projectedPoint[y] - currentPosition[y];
    relativeMotionVector[z] = 0.0;

    pan(relativeMotionVector);
}

Drawer* Q2DViewer::getDrawer() const
{
    return m_drawer;
}

bool Q2DViewer::getCurrentCursorImageCoordinate(double xyz[3])
{
    return getCurrentCursorImageCoordinateOnInput(xyz, 0);
}

bool Q2DViewer::getCurrentCursorImageCoordinateOnInput(double xyz[3], int i)
{
    VolumeDisplayUnit *displayUnit = getDisplayUnit(i);
    
    if (!displayUnit)
    {
        return false;
    }

    if (!displayUnit->getVolume())
    {
        return false;
    }
    
    bool inside = false;
    vtkPropPicker *picker = displayUnit->getImagePointPicker();
    getInteractor()->SetPicker(picker);

    QPoint position = getEventPosition();
    if (picker->Pick(position.x(), position.y(), 0.0, getRenderer()))
    {
        inside = true;
        // We calculate the pixel found
        picker->GetPickPosition(xyz);
        // We calculate the correct depth. It should be borne in mind that in the event that we have phases
        // vtk is not aware of it (each phase moves in depth z as if it were one more image)
        // and if we didn't do this calculation, we would be giving an incorrect Z coordinate
        int zIndex = displayUnit->getViewPlane().getZIndex();
        xyz[zIndex] =  displayUnit->getCurrentDisplayedImageDepth();
    }
    return inside;
}

OrthogonalPlane Q2DViewer::getView() const
{
    return getCurrentViewPlane();
}

OrthogonalPlane Q2DViewer::getViewOnInput(int i) const
{
    return getDisplayUnit(i)->getViewPlane();
}

QList<Volume*> Q2DViewer::getInputs() const
{
    QList<Volume*> volumes;
    foreach (VolumeDisplayUnit *unit, getDisplayUnits())
    {
        volumes << unit->getVolume();
    }
    
    return volumes;
}

QString Q2DViewer::getInputIdentifier() const
{
    QStringList identifiers;
    foreach(Volume *volume, getInputs())
    {
        identifiers << QString::number(volume->getIdentifier().getValue());
    }

    return identifiers.join("+");
}

void Q2DViewer::setSeedPosition(double pos[3])
{
    emit seedPositionChanged(pos[0], pos[1], pos[2]);
}

QChar Q2DViewer::getCurrentDisplayedImageLaterality() const
{
    QChar laterality;
    bool searchSeriesLateralityOnly = false;
    Image *currentImage = getCurrentDisplayedImage();
    if (!currentImage)
    {
        currentImage = getMainInput()->getImage(0);
        searchSeriesLateralityOnly = true;
    }
    
    if (currentImage)
    {
        if (!searchSeriesLateralityOnly)
        {
            laterality = currentImage->getImageLaterality();
        }

        if (laterality.isNull() || laterality.isSpace())
        {
            laterality = currentImage->getParentSeries()->getLaterality();
        }
    }
    
    return laterality;
}

void Q2DViewer::updateImageSlices()
{
    // We make sure we have valid data
    if (!getMainInput()->isPixelDataLoaded())
    {
        return;
    }

    // We only need to update the main vdu, since it will modify the camera and thus the others will also be updated
    getMainDisplayUnit()->updateImageSlice(m_renderer->GetActiveCamera());

    // TODO If we separate the renderers maybe we should apply it to each renderer?
    getRenderer()->ResetCameraClippingRange();
}

void Q2DViewer::enableAnnotation(AnnotationFlags annotation, bool enable)
{
    if (enable)
    {
        m_annotationsHandler->enableAnnotations(annotation);
    }
    else
    {
        m_annotationsHandler->disableAnnotations(annotation);
    }

    if (hasInput())
    {
        render();
    }
}

void Q2DViewer::removeAnnotation(AnnotationFlags annotation)
{
    enableAnnotation(annotation, false);
}

void Q2DViewer::setVoiLut(const VoiLut &voiLut)
{
    if (!hasInput())
    {
        return;
    }

    // If the new VOI LUT is not the currently selected one, it means that someone is calling this method to set a custom VOI LUT
    if (voiLut != getCurrentVoiLut())
    {
        // This will cause another call to this method that will follow the other branch
        getVoiLutData()->setCustomVoiLut(voiLut);
    }
    // Otherwise, it means that it has been called from the signal/slot connection from the VOI LUT data
    else
    {
        getMainDisplayUnit()->setVoiLut(voiLut);
        m_annotationsHandler->updateAnnotations(VoiLutAnnotation);
        render();
    }
}

void Q2DViewer::setVoiLutInVolume(int index, const VoiLut &voiLut)
{
    VolumeDisplayUnit *unit = this->getDisplayUnit(index);

    if (unit == m_dummyDisplayUnit)
    {
        DEBUG_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        WARN_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        return;
    }

    unit->setCurrentVoiLutPreset(voiLut);

    // An explicit render is only needed when it isn't the main unit one because if it's the main unit then
    // setVoiLut() is called eventually through signal-slot connection when setCurrentVoiLutPreset() is called.
    if (unit != getMainDisplayUnit())
    {
        this->render();
    }
}

void Q2DViewer::printVolumeInformation()
{
    double range[2];
    getMainInput()->getScalarRange(range);
    DEBUG_LOG("*** Grayscale Transform Pipeline Begin ***");
    DEBUG_LOG(QString("Image Information: Bits Allocated: %1, Bits Stored: %2, Pixel Range %3 to %4, SIGNED?Pixel Representation: %5, Photometric interpretation: %6")
              .arg(getMainInput()->getImage(0)->getBitsAllocated()).arg(getMainInput()->getImage(0)->getBitsStored()).arg(range[0]).arg(range[1])
            .arg(getMainInput()->getImage(0)->getPixelRepresentation()).arg(getMainInput()->getImage(0)->getPhotometricInterpretation().getAsQString()));
    // Until we implement Presentation states this will
    // be the case that will always run 100% of the cases
}

void Q2DViewer::setSlabProjectionMode(VolumeDisplayUnit::SlabProjectionMode projectionMode)
{
    if (m_slabProjectionMode != projectionMode)
    {
        m_slabProjectionMode = projectionMode;

        if (hasInput())
        {
            // As a measure of precaution we also remove all drawings when projection mode has changed
            // This could be a potential problem with measures that depend on the underlying data, such as ROIs,
            // as the data measured could be incoherent with the underlying data when changing the projection mode
            getDrawer()->removeAllPrimitives();
            getMainDisplayUnit()->setSlabProjectionMode(m_slabProjectionMode);
            updateImageSlices();
            render();
        }

        emit slabProjectionModeChanged(projectionMode);
    }
}

void Q2DViewer::setSlabProjectionModeInVolume(int index, VolumeDisplayUnit::SlabProjectionMode slabProjectionMode)
{
    VolumeDisplayUnit *unit = this->getDisplayUnit(index);

    if (unit == m_dummyDisplayUnit)
    {
        DEBUG_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        WARN_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        return;
    }

    unit->setSlabProjectionMode(slabProjectionMode);
    this->updateImageSlices();
    this->render();
}

VolumeDisplayUnit::SlabProjectionMode Q2DViewer::getSlabProjectionMode() const
{
    return m_slabProjectionMode;
}

void Q2DViewer::setSlabThickness(double thickness)
{
    if (!hasInput())
    {
        return;
    }
    
    VolumeDisplayUnit *mainDisplayUnit = getMainDisplayUnit();
    
    if (thickness != mainDisplayUnit->getSlabThickness())
    {
        double oldThickness = mainDisplayUnit->getSlabThickness();
        int oldSlice = getCurrentSlice();

        // First approach to avoid error primitive data: when enabling or disabling slabthickness, we delete primitives
        getDrawer()->removeAllPrimitives();

        mainDisplayUnit->setSlabThickness(thickness);
        updateImageSlices();

        updateCurrentImageDefaultPresetsInAllInputsOnOriginalAcquisitionPlane();
        m_annotationsHandler->updateAnnotations(MainInformationAnnotation | AdditionalInformationAnnotation | SliceAnnotation);
        render();

        if (mainDisplayUnit->getSlabThickness() != oldThickness)
        {
            emit slabThicknessChanged(mainDisplayUnit->getSlabThickness());
        }
        if (getCurrentSlice() != oldSlice)
        {
            emit sliceChanged(getCurrentSlice());
        }
    }
}

void Q2DViewer::setSlabThicknessInVolume(int index, double thickness)
{
    VolumeDisplayUnit *unit = this->getDisplayUnit(index);

    if (unit == m_dummyDisplayUnit)
    {
        DEBUG_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        WARN_LOG(QString("No volume at index %1. Doing nothing.").arg(index));
        return;
    }

    unit->setSlabThickness(thickness);
    this->updateImageSlices();
    this->render();
}

double Q2DViewer::getSlabThickness() const
{
    return getMainDisplayUnit()->getSlabThickness();
}

double Q2DViewer::getMaximumSlabThickness() const
{
    return getMainDisplayUnit()->getMaximumSlabThickness();
}

void Q2DViewer::disableThickSlab()
{
    setSlabThickness(0.0);
}

bool Q2DViewer::isThickSlabActive() const
{
    return isThickSlabActiveOnInput(0);
}

bool Q2DViewer::isThickSlabActiveOnInput(int i) const
{
    return getDisplayUnit(i)->isThickSlabActive();
}

void Q2DViewer::putCoordinateInCurrentImageBounds(double xyz[3])
{
    double bounds[6];
    getCurrentRenderedItemBounds(bounds);

    int xIndex = getCurrentViewPlane().getXIndex();
    int yIndex = getCurrentViewPlane().getYIndex();

    // Make each coordinate index to be in its corresponding 2D bounds
    xyz[xIndex] = qBound(bounds[xIndex * 2], xyz[xIndex], bounds[xIndex * 2 + 1]);
    xyz[yIndex] = qBound(bounds[yIndex * 2], xyz[yIndex], bounds[yIndex * 2 + 1]);
}

SliceOrientedVolumePixelData Q2DViewer::getCurrentPixelData()
{
    return getMainDisplayUnit()->getCurrentPixelData();
}

SliceOrientedVolumePixelData Q2DViewer::getCurrentPixelDataFromInput(int i)
{
    return getDisplayUnit(i)->getCurrentPixelData();
}

void Q2DViewer::restoreRenderingQuality()
{
    if (m_displayUnitsHandler)
    {
        foreach (auto *volumeDisplayUnit, m_displayUnitsHandler->getVolumeDisplayUnitList())
        {
            volumeDisplayUnit->restoreRenderingQuality();
        }
    }
}

void Q2DViewer::restore()
{
    if (!hasInput())
    {
        return;
    }

    // If there is a loading volume we do not restore
    if (m_volumeReaderManager->isReading())
    {
        return;
    }

    // HACK
    // Disable rendering to improve the efficiency of running the pipeline again,
    // since otherwise it renders multiple times and causes unwanted effects like flickering
    enableRendering(false);

    // Esborrem les anotacions
    if (hasInput())
    {
        m_drawer->removeAllPrimitives();
    }

    // TODO The view is reseted to the acquisition plane but could be more appropiate having a
    // defined command to place the image properly by default depending on the input if no one is defined
    // Take into account this call disables thickslab
    resetViewToAcquisitionPlane();
    VoiLutHelper::selectDefaultPreset(getVoiLutData(), getMainInput());

    // HACK We will restore the rendering
    enableRendering(true);
    // We apply the command
    executeInputFinishedCommand();

    emit restored();
}

void Q2DViewer::clearViewer()
{
    m_drawer->clearViewer();
}

void Q2DViewer::invertVoiLut()
{
    setVoiLut(getCurrentVoiLut().inverse());
}

void Q2DViewer::alignLeft()
{
    setAlignPosition(AlignLeft);
}

void Q2DViewer::alignRight()
{
    setAlignPosition(AlignRight);
}

void Q2DViewer::setAlignPosition(AlignPosition alignPosition)
{
    m_alignPosition = alignPosition;

    if (alignPosition == AlignCenter)
    {
        // Nothing more needs to be done
        return;
    }

    // Whether it’s AlignRight or AlignLeft
    double bounds[6];
    getCurrentRenderedItemBounds(bounds);
    double motionVector[4] = { 0.0, 0.0, 0.0, 0.0 };

    double alignmentPoint[3];
    if (alignPosition == AlignLeft)
    {
        computeDisplayToWorld(0.0, 0.0, 0.0, alignmentPoint);
    }
    else if (alignPosition == AlignRight)
    {
        computeDisplayToWorld((double)getRenderer()->GetSize()[0], 0.0, 0.0, alignmentPoint);
    }
    
    // Calculation of the displacement
    int boundIndex = 0;
    switch (getCurrentViewPlane())
    {
    case OrthogonalPlane::XYPlane:
        // If it is the case that it is either rotated 180º or it is turned over, it is necessary to take the other end
        // The operation performed is a XOR (! =)
        if (m_isImageFlipped != (qAbs(m_rotateFactor) == 2))
        {
            if (alignPosition == AlignLeft)
            {
                boundIndex = 1;
            }
        }
        else
        {
            if (alignPosition == AlignRight)
            {
                boundIndex = 1;
            }
        }

        motionVector[0] = bounds[boundIndex] - alignmentPoint[0];

        break;

    case OrthogonalPlane::YZPlane:
        if (alignPosition == AlignLeft)
        {
            boundIndex = 2;
        }
        else if (alignPosition == AlignRight)
        {
            boundIndex = 3;
        }

        motionVector[1] = bounds[boundIndex] - alignmentPoint[1];

        break;

    case OrthogonalPlane::XZPlane:
        if (alignPosition == AlignRight)
        {
            boundIndex = 1;
        }

        motionVector[0] = bounds[boundIndex] - alignmentPoint[0];

        break;
    }

    pan(motionVector);
}

void Q2DViewer::setImageOrientation(const PatientOrientation &desiredPatientOrientation)
{
    m_imageOrientationOperationsMapper->setInitialOrientation(getCurrentDisplayedImagePatientOrientation());
    m_imageOrientationOperationsMapper->setDesiredOrientation(desiredPatientOrientation);

    int turns = m_imageOrientationOperationsMapper->getNumberOfClockwiseTurnsToApply();
    bool flip = m_imageOrientationOperationsMapper->requiresHorizontalFlip();

    // Update the obtained orientation parameters
    rotate(turns);
    setFlip(flip);

    // Then, only update the camera and apply render if there have been changes on the orientation parameters
    if (flip || turns > 0)
    {
        applyImageOrientationChanges();
    }
}

void Q2DViewer::showImageOverlays(bool enable)
{
    if (enable)
    {
        getDrawer()->enableGroup(OverlaysDrawerGroup);
    }
    else
    {
        getDrawer()->disableGroup(OverlaysDrawerGroup);
    }

    m_overlaysAreEnabled = enable;
}

void Q2DViewer::showDisplayShutters(bool enable)
{
    m_showDisplayShutters = enable;
    updateDisplayShutterMask();
    render();
}

void Q2DViewer::rotate(int times)
{
    // If it is zero nothing needs to be done
    if (times == 0)
    {
        return;
    }

    // If the image is inverted by mirror effect the direction of the rotations will be the opposite
    if (m_isImageFlipped)
    {
        times = -times;
    }

    m_rotateFactor = (m_rotateFactor + times) % 4;
}

void Q2DViewer::setFlip(bool flip)
{
    m_applyFlip = flip;
}

void Q2DViewer::applyImageOrientationChanges()
{
    updateCamera();
    render();

    emit imageOrientationChanged(getCurrentDisplayedImagePatientOrientation());
}

void Q2DViewer::getCurrentRenderedItemBounds(double bounds[6])
{
    getMainDisplayUnit()->getImageSlice()->GetBounds(bounds);
}

void Q2DViewer::updateCurrentImageDefaultPresetsInAllInputsOnOriginalAcquisitionPlane()
{
    if (getCurrentViewPlane() == OrthogonalPlane::XYPlane)
    {
        foreach (VolumeDisplayUnit *unit, getDisplayUnits())
        {
            unit->updateCurrentImageDefaultPresets();
        }
    }
}

double Q2DViewer::getCurrentSpacingBetweenSlices()
{
    return getMainDisplayUnit()->getCurrentSpacingBetweenSlices();
}

double Q2DViewer::getCurrentDisplayedImageDepth() const
{
    return getCurrentDisplayedImageDepthOnInput(0);
}

double Q2DViewer::getCurrentDisplayedImageDepthOnInput(int i) const
{
    return getDisplayUnit(i)->getCurrentDisplayedImageDepth();
}

vtkImageSlice* Q2DViewer::getImageProp() const
{
    if (m_displayUnitsHandler)
    {
        return m_displayUnitsHandler->getImageProp();
    }
    else
    {
        return 0;
    }
}

Q2DViewer* Q2DViewer::castFromQViewer(QViewer *viewer)
{
    if (!viewer)
    {
        DEBUG_LOG(QString("Cannot cast a null pointer"));
        return 0;
    }

    Q2DViewer *viewer2D = qobject_cast<Q2DViewer*>(viewer);
    if (!viewer2D)
    {
        DEBUG_LOG(QString("Q2DViewer casting failed! Provided viewer may not be a Q2DViewer object: %1").arg(viewer->metaObject()->className()));
    }

    return viewer2D;
}

bool Q2DViewer::canShowDisplayShutter() const
{
    return getMainInput()
            && getMainInput()->objectName() != DummyVolumeObjectName
            && !isThickSlabActive()
            && getCurrentViewPlane() == OrthogonalPlane::XYPlane
            && getCurrentDisplayedImage()
            && getCurrentDisplayedImage()->getDisplayShutterForDisplayAsVtkImageData();
}

void Q2DViewer::updateDisplayShutterMask()
{
    vtkImageData *shutterData = NULL;
    if (m_showDisplayShutters && canShowDisplayShutter())
    {
        Image *image = getCurrentDisplayedImage();

        if (image)
        {
            shutterData = image->getDisplayShutterForDisplayAsVtkImageData();
        }
    }

    getMainDisplayUnit()->setShutterData(shutterData);
}

OrthogonalPlane Q2DViewer::getCurrentViewPlane() const
{
    return getViewOnInput(0);
}

void Q2DViewer::setCurrentViewPlane(const OrthogonalPlane &viewPlane)
{
    QViewer::setCurrentViewPlane(viewPlane);

    foreach (VolumeDisplayUnit *volumeDisplayUnit, getDisplayUnits())
    {
        volumeDisplayUnit->setViewPlane(viewPlane);
    }
}

void Q2DViewer::setVolumeOpacity(int index, double opacity)
{
    getDisplayUnit(index)->setOpacity(opacity);
}

const TransferFunction& Q2DViewer::getVolumeTransferFunction(int index) const
{
    return getDisplayUnit(index)->getTransferFunction();
}

void Q2DViewer::setVolumeTransferFunction(int index, const TransferFunction &transferFunction)
{
    getDisplayUnit(index)->setTransferFunction(transferFunction);
}

void Q2DViewer::clearVolumeTransferFunction(int index)
{
    getDisplayUnit(index)->clearTransferFunction();
}

VolumeDisplayUnit* Q2DViewer::getDisplayUnit(int index) const
{
    if (m_displayUnitsHandler.isNull() || !MathTools::isInsideRange(index, 0, getNumberOfInputs()))
    {
        return m_dummyDisplayUnit;
    }

    return m_displayUnitsHandler->getVolumeDisplayUnit(index);
}

VolumeDisplayUnit* Q2DViewer::getMainDisplayUnit() const
{
    return getDisplayUnit(0);
}

QList<VolumeDisplayUnit*> Q2DViewer::getDisplayUnits() const
{
    if (m_displayUnitsHandler.isNull())
    {
        return QList<VolumeDisplayUnit*>();
    }
    else
    {
        return m_displayUnitsHandler->getVolumeDisplayUnitList();
    }
}

TransferFunctionModel* Q2DViewer::getTransferFunctionModel() const
{
    if (!m_displayUnitsHandler)
    {
        return 0;
    }

    return m_displayUnitsHandler->getTransferFunctionModel();
}

bool Q2DViewer::containsVolume(const Volume *volume) const
{
    return this->indexOfVolume(volume) >= 0;
}

int Q2DViewer::indexOfVolume(const Volume *volume) const
{
    for (int i = 0; i < this->getNumberOfInputs(); i++)
    {
        if (this->getDisplayUnit(i)->getVolume() == volume)
        {
            return i;
        }
    }

    return -1;
}

VoiLutPresetsToolData* Q2DViewer::getVoiLutDataForVolume(int index) const
{
    return getDisplayUnit(index)->getVoiLutData();
}

int Q2DViewer::getFusionBalance() const
{
    return m_fusionBalance;
}

void Q2DViewer::setFusionBalance(int balance)
{
    if (this->getNumberOfInputs() != 2)
    {
        return;
    }

    Q_ASSERT(balance >= 0 && balance <= 100);
    m_fusionBalance = balance;

    if (balance < 50)
    {
        this->setVolumeOpacity(0, 1.0);
        this->setVolumeOpacity(1, balance / 50.0);
    }
    else
    {
        this->setVolumeOpacity(0, (100 - balance) / 50.0);
        this->setVolumeOpacity(1, 1.0);
    }

    m_annotationsHandler->updateAnnotations(MainInformationAnnotation | AdditionalInformationAnnotation);
    this->render();
}

};  // End namespace udg
