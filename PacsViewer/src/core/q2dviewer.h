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

#ifndef UDGQ2DVIEWER_H
#define UDGQ2DVIEWER_H

#include "qviewer.h"
#include "annotationflags.h"
#include "anatomicalplane.h"
#include "volumedisplayunit.h"

#include <QPointer>

// Fordward declarations
// Vtk
class vtkCoordinate;
class vtkImageData;
class vtkImageSlice;

namespace udg {

// Fordward declarations
class Image;
class ImageOverlay;
class Drawer;
class DrawerBitmap;
class ImagePlane;
class ImageOrientationOperationsMapper;
class VolumeReaderManager;
class QViewerCommand;
class PatientOrientation;
class BlendFilter;
class VolumeDisplayUnit;
class VolumePixelData;
class Q2DViewerAnnotationHandler;
class VolumeDisplayUnitHandlerFactory;
class GenericVolumeDisplayUnitHandler;
class TransferFunction;
class TransferFunctionModel;

/**
    Base class for 2D displays.

    The usual mode of operation will be to view a single volume.
    Usually to be able to view a volume we will do the following
    \ code
    Q2DViewer * visor = new Q2DViewer ();
    visor-> setInput (volume);

    In case we want to overlap two volumes we must indicate the overlapping volume with the setOverlayInput () method.
    When we overlap volumes we have 1 way to overlap these volumes, applying a blending,
    in this case we should use the setOverlapMethod () method indicating one of the options (currently only Blend)
    \ TODO finish the doc on overlap

    By default the display shows the first image in Axial. For the other views (Sagittal and Coronal) it would show the central image

    We can choose which textual and reference annotations will appear in the 2D view through the "AnnotationFlags" flags through the methods
    \ c enableAnnotation () or \ c removeAnnotation () which will make the specified annotation visible or invisible.
    The default flag is \ c AllAnnotation and therefore all default annotations are displayed.
*/
class Q2DViewer : public QViewer {
    Q_OBJECT
public:
    /// Types of overlapping models
    enum OverlapMethod { None, Blend };

    /// Image alignment (right, left, centered)
    enum AlignPosition { AlignCenter, AlignRight, AlignLeft };

    Q2DViewer(QWidget *parent = 0);
    ~Q2DViewer();

    /// It gives us back the view we have in those moments of the volume
    OrthogonalPlane getView() const;

    /// Return the view plane on the specified input. If i is out of range, default constructed value will be returned.
    OrthogonalPlane getViewOnInput(int i) const;

    Volume* getMainInput() const;

    /// Gets the i-th input. If i is out of range, null is returned
    Volume* getInput(int i) const;

    QList<Volume*> getInputs() const;

    int getNumberOfInputs() const;

    /// We return the drawer to be able to paint primitives
    /// @return Viewer drawer object
    Drawer* getDrawer() const;

    /// Returns the VOI LUT that is currently applied to the image in this viewer.
    virtual VoiLut getCurrentVoiLut() const;

    /// Returns the VOI LUT that is currently applied to the specified volume,
    /// or a default-constructed VOI LUT if the index is out of range.
    VoiLut getCurrentVoiLutInVolume(int index) const;

    /// Returns the current slice / phase
    int getCurrentSlice() const;
    int getCurrentPhase() const;

    /// Gets the current slice on the specified input. If i is out of range, 0 will be returned
    int getCurrentSliceOnInput(int i) const;

    /// Gets the current phase on the specified input. If i is out of range, 0 will be returned
    int getCurrentPhaseOnInput(int i) const;

    /// Calculates the coordinate of the image below the cursor in world coordinates
    /// In case the cursor is out of the image, the coordinate has no validity
    /// @param xyz [] The coordinate of the image, in world coordinate system
    /// @return True if the cursor is inside the image, false otherwise
    bool getCurrentCursorImageCoordinate(double xyz[3]);

    /// Same as getCurrentCursorImageCoordinate with the chance to specify from which input we want to get the coordinate.
    /// If i is out of range, false will be returned
    bool getCurrentCursorImageCoordinateOnInput(double xyz[3], int i);

    /// Returns current displayed image.
    /// If some orthogonal reconstruction different from original acquisition is applied, returns null
    Image* getCurrentDisplayedImage() const;

    /// Same as getCurrentDisplayedImage() but returning the corresponding image on the specified input
    Image* getCurrentDisplayedImageOnInput(int i) const;

    /// Gives us the current image plan we are viewing
    /// @param vtkReconstructionHack HACK boolean variable that makes us a little hack
    /// for cases where the "real" plan is not what we want and need a small modification
    /// WARNING: This parameter will only be given (with true value) in cases where you really know what is being done!
    /// @return The current image plan
    ImagePlane* getCurrentImagePlane(bool vtkReconstructionHack = false);

    /// Returns the laterality corresponding to the current displayed image.
    /// If image is not reconstructed, image laterality is returned, or series laterality if not present
    /// If image is reconstructed, series laterality is returned
    /// If no laterality is found, en empty character will be returned
    QChar getCurrentDisplayedImageLaterality() const;

    /// Retorna el thickness. En cas que no disposem del thickness, el valor retornat serà 0.0
    double getCurrentSliceThickness() const;

    /// Returns the patient orientation corresponding to the image currently being displayed,
    /// that is, taking into account rotations, flips, reconstructions, and so on.
    PatientOrientation getCurrentDisplayedImagePatientOrientation() const;

    /// It tells us what is the projection plane of the image that is seen at that moment
    /// Values: AXIAL, SAGITAL, CORONAL, OBLIQUE or N / A
    QString getCurrentAnatomicalPlaneLabel() const;

    /// Returns current anatomical plane as AnatomicalPlaneType
    AnatomicalPlane getCurrentAnatomicalPlane() const;

    /// Returns the space between the slices depending on the current view and whether the thickness is enabled
    double getCurrentSpacingBetweenSlices();

    /// Returns the depth (z coordinate value) of the main displayed image
    double getCurrentDisplayedImageDepth() const;

    /// Returns the depth (z coordinate value) of the displayed image from the specified input
    double getCurrentDisplayedImageDepthOnInput(int i) const;

    /// Gets the pixel data corresponding to the current rendered image
    SliceOrientedVolumePixelData getCurrentPixelData();

    /// Gets the pixel data corresponding to the current rendered image from the specified input.
    SliceOrientedVolumePixelData getCurrentPixelDataFromInput(int i);

    /// Restores the standard rendering quality in this viewer.
    void restoreRenderingQuality();

    /// Gives us the minimum / maximum slice of slices, taking into account all the images,
    /// whether there are phases or not
    /// @return minimum / maximum slice value
    int getMinimumSlice() const;
    int getMaximumSlice() const;

    /// Returns the total number of slices on the spatial dimension that has the main input on the current view
    int getNumberOfSlices() const;

    /// Returns the total number of phases that has the main input.
    /// It only applies to the original acquisition view plane. The minimum number of phases will be 1.
    int getNumberOfPhases() const;

    /// Returns the total number of phases from the specified input.
    /// It only applies to the original acquisition view plane. The minimum number of phases will be 1.
    /// If i is out of range, 1 will be returned
    int getNumberOfPhasesFromInput(int i) const;

    /// Returns true if the number of phases is greater than 1
    bool hasPhases() const;

    /// Returns true if the number of phases is greater than 1 on the specified input.
    /// If i is out of range, false will be returned.
    bool doesInputHavePhases(int i) const;

    /// Returns true if the current image of the selected input is visible, false otherwise
    bool isInputVisible(int i) const;

    /// Ens indica si s'està aplicant o no thick slab
    bool isThickSlabActive() const;

    /// Ask is thickslab is active on the i-th input. If i is out of range, false will be returned
    bool isThickSlabActiveOnInput(int i) const;

    /// Returns current slab projection mode.
    VolumeDisplayUnit::SlabProjectionMode getSlabProjectionMode() const;

    /// Returns current slab thickness in mm. If thickslab is disabled, returns 0.
    double getSlabThickness() const;

    /// Returns the maximum slab thickness that can be set.
    double getMaximumSlabThickness() const;

    /// Casts the given QViewer to a Q2DViewer object
    /// If casting is successful, casted pointer to Q2DViewer will be returned, null otherwise
    static Q2DViewer* castFromQViewer(QViewer *viewer);

    /// Sets the opacity of the image actor of the volume at the given index.
    void setVolumeOpacity(int index, double opacity);

    /// Returns the transfer function of the volume at the given index.
    const TransferFunction& getVolumeTransferFunction(int index) const;

    /// Sets the transfer function of the volume at the given index.
    void setVolumeTransferFunction(int index, const TransferFunction &transferFunction);

    /// Clears the transfer function of the volume at the given index.
    void clearVolumeTransferFunction(int index);

    /// Returns the prop that represents the image in the scene.
    vtkImageSlice* getImageProp() const;

    /// Returns true if this Q2DViewer can show a display shutter in its current state,
    /// i.e. if there is a display shutter for the current image and there isn't
    /// any restriction to show display shutters.
    bool canShowDisplayShutter() const;

    /// Given a world coordinate, we adjust it so that it falls within the limits of the current image
    /// This serves us for tools that take any point in the world, but need it to be
    /// within the limits of the image, such as an ROI. This method just adjusts the coordinate because
    /// is within the limits of the image itself
    /// @param xyz [] Coordinate we want to adjust. It will be an input / output parameter and its contents
    /// will be modified to fall within the image limits
    void putCoordinateInCurrentImageBounds(double xyz[3]);

    /// given a 3D point in DICOM reference space, it gives us the projection of this point on
    /// the current plan, transformed into VTK world coordinates
    /// @param pointToProject []
    /// @param projectedPoint []
    /// @param vtkReconstructionHack HACK boolean variable that makes us a little hack
    /// for cases where the "real" plan is not what we want and need a small modification
    void projectDICOMPointToCurrentDisplayedImage(const double pointToProject[3], double projectedPoint[3], bool vtkReconstructionHack = false);

    bool getDicomWorldCoordinates(const double xyz[3], double dicomWorldPosition[4]);

    /// Assigna/Retorna el volum solapat
    void setOverlayInput(Volume *volume);
    Volume* getOverlayInput();

    /// We indicate that the current Overlay needs to be updated
    void updateOverlay();

    /// Assign the opacity of the overlapping volume.
    /// Values can range from 0.0 to 1.0, where 0.0 is transparent and 1.0 is completely opaque.
    void setOverlayOpacity(double opacity);

    /// Returns the current transfer function of the main volume.
    const TransferFunction& getTransferFunction() const;

    /// Returns the transfer function model corresponding to the input(s) of this Q2DViewer.
    TransferFunctionModel* getTransferFunctionModel() const;

    /// Returns a unique string identifier of all inputs
    virtual QString getInputIdentifier() const;

    /// Returns true if this viewer contains the given volume.
    bool containsVolume(const Volume *volume) const;

    /// Returns the index of the given volume in this viewer. If this viewer doesn't contain the given volume, returns -1.
    int indexOfVolume(const Volume *volume) const;

    /// Returns VOI LUT data corresponding to the volume at the given index.
    VoiLutPresetsToolData* getVoiLutDataForVolume(int index) const;

    /// Returns the fusion balance as a value in the range [0, 100] representing the weight of the second input.
    int getFusionBalance() const;

    /// Moves the camera based on the absolute motion vector
    void absolutePan(double motionVector[3]);

public slots:
    virtual void setInput(Volume *volume);

    /// Specifies the input volume asynchronously.
    /// You can specify a command to run once the volume has been loaded and is about to be displayed.
    /// Useful for specifying changes to the viewer (slice change, w / l, etc.) without worrying about when the volume has been loaded.
    void setInputAsynchronously(Volume *volume, QViewerCommand *inputFinishedCommand = 0);

    void setInputAsynchronously(const QList<Volume*> &volumes, QViewerCommand *inputFinishedCommand = 0);

    void resetView(const OrthogonalPlane &view);

    void resetView(const AnatomicalPlane &anatomicalPlane);

    /// Restores the viewer to its initial state
    void restore();

    /// Deletes all viewer primitives
    void clearViewer();

    /// Change the WW of the viewer, in order to change whites to blacks, and blacks to whites
    void invertVoiLut();

    /// Change the slice we see from the current view
    void setSlice(int value);

    /// Sets the given phase index to the main volume.
    void setPhase(int value);

    /// Sets the given phase index to the volume at the given index. If there isn't a volume at the given index, it does nothing.
    void setPhaseInVolume(int index, int phase);

    /// Indicates the type of volume overlap, by default blending
    void setOverlapMethod(OverlapMethod method);

    ///Add or remove the visibility of a textual / graphic annotation
    void enableAnnotation(AnnotationFlags annotation, bool enable = true);
    void removeAnnotation(AnnotationFlags annotation);

    /// Sets the VOI LUT for this viewer.
    virtual void setVoiLut(const VoiLut &voiLut);

    /// Sets the given VOI LUT to the volume at the given index. If there isn't a volume at the given index, it does nothing.
    void setVoiLutInVolume(int index, const VoiLut &voiLut);

    /// Sets the transfer function of the main volume.
    void setTransferFunction(const TransferFunction &transferFunction);

    /// Clears the transfer function of the main volume.
    void clearTransferFunction();

    /// All it does is issue the seedPositionChanged signal, so you can call it from the seedTool
    /// EVERYTHING This method should become obsolete
    void setSeedPosition(double pos[3]);

    /// Apply a 90-degree rotation clockwise
    /// as many "times" as we tell you, by default it will always be 1 "time"
    void rotateClockWise(int times = 1);

    /// Apply a 90-degree rotation counterclockwise
    /// as many "times" as we tell you, by default it will always be 1 "time"
    void rotateCounterClockWise(int times = 1);

    ///Apply a horizontal / vertical flip to the image.
    /// The vertical flip will be done as a 180º rotation followed by a horizontal flip
    void horizontalFlip();

    void verticalFlip();

    // ALL of these methods should also be in QString version!

    /// Sets the given slab projection mode to the main volume.
    void setSlabProjectionMode(VolumeDisplayUnit::SlabProjectionMode projectionMode);

    /// Sets the given slab projection mode to the volume at the given index.
    ///  If there isn't a volume at the given index, it does nothing.
    void setSlabProjectionModeInVolume(int index, VolumeDisplayUnit::SlabProjectionMode slabProjectionMode);

    /// Sets the given slab thickness in mm to the main volume.
    void setSlabThickness(double thickness);

    /// Sets the given slab thickness in mm to the volume at the given index.
    /// If there isn't a volume at the given index, it does nothing.
    void setSlabThicknessInVolume(int index, double thickness);

    /// Disables thick slab. Acts as a shortcut for setSlabThickness(0.0).
    void disableThickSlab();

    /// Image alignment within the viewer
    void alignLeft();
    void alignRight();

    /// Set image alignment position (right, left, centered)
    void setAlignPosition(AlignPosition alignPosition);

    /// Apply the necessary 2D transformations on the current image so that it has the indicated orientation
    /// The indicated orientation must be possible to obtain by means of operations of rotation and flip. In case not
    /// if possible combinations exist, the image orientation will not be changed
    void setImageOrientation(const PatientOrientation &desiredPatientOrientation);

    /// Makes ImageOverlays visible or not
    void showImageOverlays(bool enable);

    /// Makes the shutters visible or not
    void showDisplayShutters(bool enable);

    /// Sets the fusion balance as a value in the range [0, 100] representing the weight of the second input.
    void setFusionBalance(int balance);

signals:
    /// Send the new slice we are in
    void sliceChanged(int);

    /// Send the new phase we are in
    void phaseChanged(int);

    /// Send the new view we are in
    void viewChanged(int);

    /// Emitted when a new patient orientation has been set
    void imageOrientationChanged(const PatientOrientation &orientation);

    /// Signal sent when the seed has changed
    /// EVERYTHING Try to take it out and put it in the SeedTool tool
    void seedPositionChanged(double x, double y, double z);

    /// Emitted when the slab projection mode has changed.
    void slabProjectionModeChanged(VolumeDisplayUnit::SlabProjectionMode slabProjectionMode);

    /// Issued when the thickness thickness changes
    /// @param thickness New thickness value
    void slabThicknessChanged(double thickness);

    ///Signal sent when the overlay has changed
    void overlayChanged();
    void overlayModified();

    /// Signal emitted when the viewer is restored
    void restored();

    /// Signal emitted when a set of new volumes (not dummies) have been rendered
    void newVolumesRendered();

protected:
    ///We process the resize event of the Qt window
    virtual void resizeEvent(QResizeEvent *resize);

    void getCurrentRenderedItemBounds(double bounds[6]);

    void setDefaultOrientation(const AnatomicalPlane &anatomicalPlane);

    /// Returns the current view plane.
    virtual OrthogonalPlane getCurrentViewPlane() const;

    /// Sets the current view plane.
    virtual void setCurrentViewPlane(const OrthogonalPlane &viewPlane);

private:
    /// Updates image orientation according to the preferred presentation depending on its attributes, like modality.
    /// At this moment it is only applying to mammography (MG) images
    void updatePreferredImageOrientation();

    /// Initializes camera parameters
    void initializeCamera();

    /// Initialized the dummy display unit with the proper parameters
    void initializeDummyDisplayUnit();

    /// Adds/remove the image actors to/from the scene
    void addImageActors();
    void removeImageActors();

    /// Updates the displayed images in the image slices.
    void updateImageSlices();

    /// Print some information related to the volume
    void printVolumeInformation();

    /// Update the shutter filter pipeline depending on whether it is enabled or not
    void updateShutterPipeline();

    /// Updates the mask used as display shutter if display shutters should and can be shown.
    void updateDisplayShutterMask();

    ///Resets the camera settings to the current view.
    void resetCamera();

    /// Apply the appropriate rotation factor according to the turns we indicate.
    /// It doesn’t update the camera or the scene, simply
    /// is used to set the correct values to the internal variables that control how the image is rotated.
    void rotate(int times);

    /// Sets if image should be flipped (horizontally) or not. It does not update the camera nor renders the scene.
    void setFlip(bool flip);

    /// Updates the camera, renders and emits the current image orientataion
    void applyImageOrientationChanges();

    /// Loads an asynchronous volumet
    void loadVolumeAsynchronously(Volume *volume);
    void loadVolumesAsynchronously(const QList<Volume *> &volumes);

    /// Returns a dummy volume
    Volume* getDummyVolumeFromVolume(Volume *volume);

    /// Specifies which command to run after specifying a volume as input
    void setInputFinishedCommand(QViewerCommand *command);

    /// Removes the command that should be executed after an input is specified
    void deleteInputFinishedCommand();

    ///If set, run the command set to after specifying input to the viewer
    void executeInputFinishedCommand();

    ///Updates the current image default presets values. It only applies to original acquisition plane.
    void updateCurrentImageDefaultPresetsInAllInputsOnOriginalAcquisitionPlane();

    /// Calls setNewVolumes and excutes the command while catching any exception that may be thrown.
    void setNewVolumesAndExecuteCommand(const QList<Volume*> &volumes);

    ///Delete the bitmaps created by this viewer
    void removeViewerBitmaps();

    /// Loads ImageOverlays from the last volume per parameter (as long as it's not a dummy)
    /// and adds them to the Drawer
    void loadOverlays(Volume *volume);

    /// Enum to define the different dimensions an image slice could be associated to
    enum SliceDimension { SpatialDimension, TemporalDimension };

    /// Updates the image slice to be displayed on the specified dimension
    void updateSliceToDisplay(int value, SliceDimension dimension);

    /// Updates the slice to display in the secondary volumes to the closest one in the main volume.
    void updateSecondaryVolumesSlices();

    /// Returns the VolumeDisplayUnit of the given index. Returns null if there's no display unit or index is out of range
    VolumeDisplayUnit* getDisplayUnit(int index) const;
    VolumeDisplayUnit* getMainDisplayUnit() const;

    /// Returns all the current display units. The list will be empty if we have no input.
    QList<VolumeDisplayUnit*> getDisplayUnits() const;

private slots:
    ///Update camera transformations (currently rotate and flip)
    void updateCamera();

    /// Reimplement so that it does a setInputAsynchronously
    /// TODO: At the moment this is done in a sloppy way until the loading methods are transferred
    /// asynchronous to QViewer.
    virtual void setInputAndRender(Volume *volume);
    void setInputAndRender(const QList<Volume*> &volumes);

    /// Replaces the volumes displayed by this viewer by the new ones and resets the viewer.
    /// If the second parameter is false, the volumes won't be rendered.
    void setNewVolumes(const QList<Volume*> &volumes, bool setViewerStatusToVisualizingVolume = true);

    void volumeReaderJobFinished();

protected:
    /// This is the second volume added to overlap
    Volume *m_overlayVolume;

    /// This is the blender for viewing merged images
    BlendFilter* m_blender;

    /// Opacity of the overlapping volume
    double m_overlayOpacity;

private:
    /// Name of the groups in the drawer for Overlays
    static const QString OverlaysDrawerGroup;

    /// Constant to define the object name of the "dummy" volumes
    static const QString DummyVolumeObjectName;

    /// Types of overlapping volumes in case we have more than one
    OverlapMethod m_overlapMethod;

    /// Rotation factor. Clockwise 0: 0º, 1: 90º, 2: 180º, 3: 270º.
    int m_rotateFactor;

    /// Indicates whether or not to apply a horizontal flip to the camera
    bool m_applyFlip;

    /// This variable controls whether the image is flipped from its original orientation. Useful for controlling annotations.
    bool m_isImageFlipped;

    /// Especialista en dibuixar primitives
    Drawer *m_drawer;

    ///Indicate what type of projection we apply on the slab
    VolumeDisplayUnit::SlabProjectionMode m_slabProjectionMode;

    /// It contains the mapping of operations to be done when I want to move from one orientation to another
    ImageOrientationOperationsMapper *m_imageOrientationOperationsMapper;

    ///Position where the image should be aligned (right, left, or centered)
    AlignPosition m_alignPosition;

    /// Manager of the reading of volumes
    VolumeReaderManager *m_volumeReaderManager;

    QViewerCommand *m_inputFinishedCommand;

    /// List of overlays
    QList<DrawerBitmap*> m_viewerBitmaps;

    /// Controls whether overlays are enabled or not
    bool m_overlaysAreEnabled;

    /// If true, display shutters are visible when they are available and it's possible to show them.
    bool m_showDisplayShutters;

    /// Factory to create the proper display units handler
    VolumeDisplayUnitHandlerFactory *m_displayUnitsFactory;

    /// Unit to be used when no input is present or an out of index unit have been requested.
    /// This way we can always return safe default values.
    VolumeDisplayUnit *m_dummyDisplayUnit;

    /// VolumeDisplayUnits handler. Handles all the display units of the viewer.
    QSharedPointer<GenericVolumeDisplayUnitHandler> m_displayUnitsHandler;

    /// Handles the textual annotations of the viewer
    Q2DViewerAnnotationHandler *m_annotationsHandler;

    /// Fusion balance stored as a value in the range [0, 100] representing the weight of the second input.
    int m_fusionBalance;

};

};  //  End namespace udg

#endif
