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

#ifndef TRANSFERFUNCTIONEDITORCOMMANDS_H
#define TRANSFERFUNCTIONEDITORCOMMANDS_H

/**
    \file Aquí es defineixen tots els commands de TransferFunctionEditor, per no omplir massa transferfunctioneditor.h.
    No els documentarem individualment, però tots tenen el constructor, un redo() i un undo().
 */

#include <QUndoCommand>

#include "transferfunctioneditor.h"

#include <QColor>

namespace udg {

class TransferFunctionEditor::SetTransferFunctionCommand : public QUndoCommand {
public:
    SetTransferFunctionCommand(TransferFunctionEditor *editor, const TransferFunction &transferFunction);
    virtual void redo();
    virtual void undo();
private:
    TransferFunctionEditor *m_editor;
    TransferFunction m_oldTransferFunction;
    TransferFunction m_newTransferFunction;
};

class TransferFunctionEditor::SetNameCommand : public QUndoCommand {
public:
    SetNameCommand(TransferFunctionEditor *editor, const QString &name);
    virtual void redo();
    virtual void undo();
private:
    TransferFunctionEditor *m_editor;
    QString m_oldName;
    QString m_newName;
};

class TransferFunctionEditor::SetColorTransferFunctionCommand : public QUndoCommand {
public:
    SetColorTransferFunctionCommand(TransferFunctionEditor *editor, const ColorTransferFunction &colorTransferFunction);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    ColorTransferFunction m_oldColorTransferFunction;
    ColorTransferFunction m_newColorTransferFunction;
};

class TransferFunctionEditor::SetScalarOpacityTransferFunctionCommand : public QUndoCommand {
public:
    SetScalarOpacityTransferFunctionCommand(TransferFunctionEditor *editor, const OpacityTransferFunction &scalarOpacityTransferFunction);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    OpacityTransferFunction m_oldScalarOpacityTransferFunction;
    OpacityTransferFunction m_newScalarOpacityTransferFunction;
};

class TransferFunctionEditor::SetGradientOpacityTransferFunctionCommand : public QUndoCommand {
public:
    SetGradientOpacityTransferFunctionCommand(TransferFunctionEditor *editor, const OpacityTransferFunction &gradientOpacityTransferFunction);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    OpacityTransferFunction m_oldGradientOpacityTransferFunction;
    OpacityTransferFunction m_newGradientOpacityTransferFunction;
};

class TransferFunctionEditor::AddColorPointCommand : public QUndoCommand {
public:
    AddColorPointCommand(TransferFunctionEditor *editor, double x, const QColor &color);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    double m_x;
    QColor m_color;
};

class TransferFunctionEditor::RemoveColorPointCommand : public QUndoCommand {
public:
    RemoveColorPointCommand(TransferFunctionEditor *editor, double x);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    double m_x;
    QColor m_color;
};

class TransferFunctionEditor::AddScalarOpacityPointCommand : public QUndoCommand {
public:
    AddScalarOpacityPointCommand(TransferFunctionEditor *editor, double x, double opacity);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    double m_x;
    double m_opacity;
};

class TransferFunctionEditor::RemoveScalarOpacityPointCommand : public QUndoCommand {
public:
    RemoveScalarOpacityPointCommand(TransferFunctionEditor *editor, double x);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    double m_x;
    double m_opacity;
};

class TransferFunctionEditor::AddGradientOpacityPointCommand : public QUndoCommand {
public:
    AddGradientOpacityPointCommand(TransferFunctionEditor *editor, double y, double opacity);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    double m_y;
    double m_opacity;
};

class TransferFunctionEditor::RemoveGradientOpacityPointCommand : public QUndoCommand {
public:
    RemoveGradientOpacityPointCommand(TransferFunctionEditor *editor, double y);
    virtual void undo();
    virtual void redo();
private:
    TransferFunctionEditor *m_editor;
    double m_y;
    double m_opacity;
};

} // End namespace udg

#endif // TRANSFERFUNCTIONEDITORCOMMANDS_H
