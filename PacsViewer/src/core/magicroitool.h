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

#ifndef UDGMAGICROITOOL_H
#define UDGMAGICROITOOL_H

#include "roitool.h"

#include <QVector>

namespace udg {

class Q2DViewer;
class Volume;
class DrawerText;
class DrawerPolygon;
class VolumePixelData;
class VoxelIndex;

/**
   Tool used to edit the superimposed volume in a 2D viewer
*/
class MagicROITool : public ROITool {
    Q_OBJECT
public:
    // Creixement
    enum { LeftDown, Down, RightDown, Right, RightUp, Up, LeftUp, Left };
    // Moviments
    enum { MoveRight, MoveLeft, MoveUp, MoveDown };

    MagicROITool(QViewer *viewer, QObject *parent = 0);
    ~MagicROITool();

    void handleEvent(unsigned long eventID);

protected:
    virtual void setTextPosition(DrawerText *text);

private:
    /// Returns the current pixel data from the selected input.
    SliceOrientedVolumePixelData getPixelData();

    /// It calls for the generation of the magical region
    void generateRegion();

    /// Gets the index of the input where the magic ROI has to be drawn
    int getROIInputIndex() const;
    
    /// Returns the slice oriented voxel index in the selected
    ///  pixel data corresponding to the current picked position.
    VoxelIndex getPickedPositionVoxelIndex();
    
    /// Calculates the range of valid intensity values from \ sa #m_magicSize and \ see #m_magicFactor
    void computeLevelRange();

    /// Iterative version of the Growing region
    void computeRegionMask();

    /// Make a movement from an index to a direction
    /// @param a, @param b index of the volume of the mask we are looking at in each call
    /// @param movement direction in movement
    void doMovement(int &a, int &b, int movement);

    /// Undo a dses movement from an index to a direction
    /// @param a, @param b index of the volume of the mask we are looking at in each call
    /// @param movement direction in movement
    void undoMovement(int &a, int &b, int movement);

    ///Generate the polygon from the mask
    void computePolygon();

    /// Auxiliary methods for the generation of the polygon
    void getNextIndex(int direction, int x, int y, int &nextX, int &nextY);
    int getNextDirection(int direction);
    int getInverseDirection(int direction);
    void addPoint(int direction, int x, int y);
    bool isLoopReached();

    /// Returns the standard deviation within the region marked by magicSize
    double getStandardDeviation();

    /// The generation of the magical region begins
    void startRegion();

    /// Calculate the final region and display the measurements on the screen
    void endRegion();

    ///Modify the Magic Factor #m_magicFactor according to the movement of the mouse
    void modifyRegionByFactor();

    /// Calculate the bounds of the mask
    void computeMaskBounds();

    ///Returns the value of the voxel at the given slice oriented voxel index.
    /// Only the first component is considered.
    double getVoxelValue(const VoxelIndex &index);

    /// Removes the temporary representation from the tool
    void deleteTemporalRepresentation();

    /// Returns the mask index corresponding to the given x and y image indices.
    int getMaskVectorIndex(int x, int y) const;

    /// Returns the mask value at the given x and y image indices.
    ///  If the indices are out of bounds, returns false.
    bool getMaskValue(int x, int y) const;

private slots:
    /// Initialize the tool
    void initialize();

    ///It restarts the region, invalidating the previous one that was in progress if it existed
    void restartRegion();

private:
    /// Possible states of the tool.
    enum State { Ready, Drawing };

    /// mid Adela tool
    static const int MagicSize;
    static const double InitialMagicFactor;

    double m_magicFactor;

    ///Mask of the region that will form the polygon
    QVector<bool> m_mask;

    /// Bounds of the mask
    int m_minX, m_maxX, m_minY, m_maxY;
    
    ///Range of values to be taken into account for the growing region
    double m_lowerLevel;
    double m_upperLevel;

    /// Coordinates of the world where the initial click was made
    double m_pickedPosition[3];

    /// Full polygon that will be displayed during the ROI edition.
    QPointer<DrawerPolygon> m_filledRoiPolygon;

    /// Screen coordinates where the initial click was made
    QPoint m_pickedPositionInDisplayCoordinates;

    ///Index of the input to draw the magic ROI on
    int m_inputIndex;

    /// Current state of the tool.
    State m_state;
};

}

#endif
