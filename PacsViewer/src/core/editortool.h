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

#ifndef UDGEDITORTOOL_H
#define UDGEDITORTOOL_H

#include "tool.h"

class vtkActor;

namespace udg {

class Q2DViewer;
class Volume;
class EditorToolData;

/**
    Tool used to edit the superimposed volume in a 2D viewer
  */
class EditorTool : public Tool {
    Q_OBJECT
public:
    ///Tool states
    enum { NoEditor, Paint, Erase, EraseSlice, EraseRegion };

    EditorTool(QViewer *viewer, QObject *parent = 0);
    ~EditorTool();

    void handleEvent(unsigned long eventID);

    /// Returns tool data
    ToolData* getToolData() const;

private:
    /// Paint the green square if necessary (Paint and Erase states) at the cursor position
    /// if the left button is pressed the action of painting is also done
    void setPaintCursor();

    /// Change the status to Erase
    void setErase();

    ///Change the status to Paint
    void setPaint();

    ///Change the status to Erase Slice
    void setEraseSlice();

    ///Change the status to Erase Region
    void setEraseRegion();

    ///It calls for action as appropriate according to the state
    void setEditorPoint();

    ///Delete a square portion of the m_editorSize size mask
    void eraseMask();

    /// Paint a square portion of the m_editorSize size mask
    void paintMask();

    ///Erase a slice of the mask
    void eraseSliceMask();

    ///Delete a connected portion of the mask (2D)
    void eraseRegionMask();

    /// Recursive call of the function eraseRegionMask
    /// @param a, @param b, @param c index of the volume of the mask we are looking at in each call
    void eraseRegionMaskRecursive(int a, int b, int c);

    /// Decreases a state of the tool. Order: Paint, Erase, EraseRegion, EraseSlice
    void decreaseState();

    ///Increases the status of the tool. Order: Paint, Erase, EraseRegion, EraseSlice
    void increaseState();

    ///Increases the size of the editor
    void increaseEditorSize();

    /// Decreases the size of the editor
    void decreaseEditorSize();

private slots:
    /// Initialize several data required by the tool: Values of m_insideValue, m_outsideValue and m_volumeCont
    void initialize();

private:
    ///We save this pointer to be more comfortable
    Q2DViewer *m_2DViewer;

    /// The way we edit the overlay.
    /// At the moment you can have 4 types: write, delete, delete region and delete slice, by default write
    int m_editorState;

    /// mid Adela tool
    int m_editorSize;

    vtkActor *m_squareActor;

    //\ TODO: This variable should not be needed, as it should only be in ToolData
    int m_volumeCont;
    EditorToolData* m_myData;
    int m_insideValue;
    int m_outsideValue;
    bool m_isLeftButtonPressed;

};

}

#endif
