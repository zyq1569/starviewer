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

#ifndef UDGVIEWERSLAYOUT_H
#define UDGVIEWERSLAYOUT_H

#include <QWidget>

#include <QMap>
#include <QSet>

namespace udg {

class AnatomicalPlane;
class Q2DViewerWidget;
class RelativeGeometryLayout;
class Volume;

/ **
    Class that allows to distribute on a widget a series Q2DViewerWidgets
    with different layouts and geometries in a versatile way.
  * /
class ViewersLayout: public QWidget {
Q_OBJECT
public:
    ViewersLayout (QWidget * parent = 0);
    ~ ViewersLayout ();

    /// Get the selected viewer
    Q2DViewerWidget * getSelectedViewer () const;

    /// Returns us in total number of viewers contained in the layout manager,
    /// regardless of whether they are visible or not.
    int getNumberOfViewers () const;

    /// Returns the viewer with "number" index. If number is out of range,
    /// will return a null pointer.
    Q2DViewerWidget * getViewerWidget (int number) const;

    /// Makes a regular layout with the rows and columns indicated
    void setGrid (int rows, int columns);
    void setGridInArea (int rows, int columns, const QRectF & geometry);

    /// Adds a new viewer with the indicated geometry
    Q2DViewerWidget * addViewer (const QRectF & geometry);

    /// Clean the layout, removing all viewers and geometries,
    /// leaving it in the initial state, as if we had just created the object
    void cleanUp ();
    void cleanUp (const QRectF & geometry);

    QRectF convertGeometry (const QRectF & viewerGeometry, const QRectF & newGeometry);

    /// Maps all viewers inside \ a oldGeometry to occupy the corresponding space inside \ a newGeometry.
    void mapViewersToNewGeometry (const QRectF & oldGeometry, const QRectF & newGeometry);

    /// Returns the relative geometry of the given viewer in its normal (i.e. not maximized) state.
    /// If the given viewer is not in this ViewersLayout, returns a null QRectF.
    QRectF getGeometryOfViewer (Q2DViewerWidget * viewer) const;

    /// Return viewers located inside the given geometry.
    QList <Q2DViewerWidget *> getViewersInsideGeometry (const QRectF & geometry);

public slots:
    /// We mark as selected the viewer passed by parameter
    void setSelectedViewer (Q2DViewerWidget * viewer);

    /// Resizes the given viewer to change between normal (demaximized) and maximized states.
    void toggleMaximization (Q2DViewerWidget * viewer);

signals:
    /// Signal emitted when a viewer is added
    void viewerAdded (Q2DViewerWidget * viewer);

    /// Signal to be emitted just before the viewer widget is going to be deleted
    void viewerRemoved (Q2DViewerWidget * viewer);

    /// Signal emitted when a viewer is hidden, ie
    // is no longer visible but still exists
    void viewerHidden (Q2DViewerWidget * viewer);

    /// Emitted when a previously hid viewer is shown
    void viewerShown (Q2DViewerWidget * viewer);

    /// Signal that is output when the selected viewer changes
    void selectedViewerChanged (Q2DViewerWidget * viewer);

    void manualSynchronizationStateChanged (bool enable);

    /// Propagation of the matching signal in Q2DViewerWidget.
    void fusionLayout2x1FirstRequested (const QList <Volume *> & volumes, const AnatomicalPlane & anatomicalPlane);
    /// Propagation of the matching signal in Q2DViewerWidget.
    void fusionLayout2x1SecondRequested (const QList <Volume *> & volumes, const AnatomicalPlane & anatomicalPlane);
    /// Propagation of the matching signal in Q2DViewerWidget.
    void fusionLayout3x1Requested (const QList <Volume *> & volumes, const AnatomicalPlane & anatomicalPlane);
    /// Propagation of the matching signal in Q2DViewerWidget.
    void fusionLayout2x3FirstRequested (const QList <Volume *> & volumes);
    /// Propagation of the matching signal in Q2DViewerWidget.
    void fusionLayout2x3SecondRequested (const QList <Volume *> & volumes);
    /// Propagation of the matching signal in Q2DViewerWidget.
    void fusionLayout3x3Requested (const QList <Volume *> & volumes);

private:
    /// Creates and returns a new viewer configured properly
    Q2DViewerWidget * getNewQ2DViewerWidget ();

    /// Auxiliary method for a problem that can prove to us that we run out of memory due to an unresolved bug
    void deleteQ2DViewerWidget (Q2DViewerWidget * viewer);

    /// Place the given viewer in the position and sizes provided
    /// @param viewer Viewer we want to position and adjust within the layout
    /// @param geometry with the positions and realistic sizes corresponding to the viewer
    void setViewerGeometry (Q2DViewerWidget * viewer, const QRectF & geometry);

    /// Take the necessary actions to hide the indicated viewer from the current layout
    void hideViewer (Q2DViewerWidget * viewer);

    /// Performs the needed actions to show the given viewer of the current layout
    void showViewer (Q2DViewerWidget * viewer);

    /// Demaximized maximized viewers that overlap with the given geometry.
    void demaximizeViewersIntersectingGeometry (const QRectF & geometry);

private:
    RelativeGeometryLayout * m_layout;

    /// Viewer selected. We will always have one.
    Q2DViewerWidget * m_selectedViewer;

    /// This struct holds the information needed to demaximize a maximized viewer.
    struct MaximizationData
    {
        QRectF normalGeometry;
        QSet<Q2DViewerWidget*> occludedViewers;
    };

    /// Maps each maximized viewer to its maximization data.
    QMap<Q2DViewerWidget*, MaximizationData> m_maximizedViewers;

};

}

#endif
