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

#include "transferfunctioneditorcommands.h"

#include <QtConcurrentMap>

namespace udg {

TransferFunctionEditor::SetTransferFunctionCommand::SetTransferFunctionCommand(TransferFunctionEditor *editor, const TransferFunction &transferFunction)
    : m_editor(editor), m_oldTransferFunction(editor->transferFunction()), m_newTransferFunction(transferFunction)
{
    setText(tr("Set transfer function %1").arg(transferFunction.name()));
}

void TransferFunctionEditor::SetTransferFunctionCommand::redo()
{
    m_editor->setTransferFunctionCommand(m_newTransferFunction);
}

void TransferFunctionEditor::SetTransferFunctionCommand::undo()
{
    m_editor->setTransferFunctionCommand(m_oldTransferFunction);
}

TransferFunctionEditor::SetNameCommand::SetNameCommand(TransferFunctionEditor *editor, const QString &name)
    : m_editor(editor), m_oldName(editor->transferFunction().name()), m_newName(name)
{
    setText(tr("Set name %1").arg(name));
}

void TransferFunctionEditor::SetNameCommand::redo()
{
    m_editor->setNameCommand(m_newName);
}

void TransferFunctionEditor::SetNameCommand::undo()
{
    m_editor->setNameCommand(m_oldName);
}

TransferFunctionEditor::SetColorTransferFunctionCommand::SetColorTransferFunctionCommand(TransferFunctionEditor *editor,
                                                                                         const ColorTransferFunction &colorTransferFunction)
    : m_editor(editor), m_oldColorTransferFunction(editor->transferFunction().colorTransferFunction()), m_newColorTransferFunction(colorTransferFunction)
{
    setText(tr("Set color transfer function %1").arg(colorTransferFunction.name()));
}

void TransferFunctionEditor::SetColorTransferFunctionCommand::redo()
{
    m_editor->setColorTransferFunctionCommand(m_newColorTransferFunction);
}

void TransferFunctionEditor::SetColorTransferFunctionCommand::undo()
{
    m_editor->setColorTransferFunctionCommand(m_oldColorTransferFunction);
}

TransferFunctionEditor::SetScalarOpacityTransferFunctionCommand
                      ::SetScalarOpacityTransferFunctionCommand(TransferFunctionEditor *editor, const OpacityTransferFunction &scalarOpacityTransferFunction)
    : m_editor(editor), m_oldScalarOpacityTransferFunction(editor->transferFunction().scalarOpacityTransferFunction()),
      m_newScalarOpacityTransferFunction(scalarOpacityTransferFunction)
{
    setText(tr("Set scalar opacity transfer function %1").arg(scalarOpacityTransferFunction.name()));
}

void TransferFunctionEditor::SetScalarOpacityTransferFunctionCommand::redo()
{
    m_editor->setScalarOpacityTransferFunctionCommand(m_newScalarOpacityTransferFunction);
}

void TransferFunctionEditor::SetScalarOpacityTransferFunctionCommand::undo()
{
    m_editor->setScalarOpacityTransferFunctionCommand(m_oldScalarOpacityTransferFunction);
}

TransferFunctionEditor::SetGradientOpacityTransferFunctionCommand
                      ::SetGradientOpacityTransferFunctionCommand(TransferFunctionEditor *editor,
                                                                  const OpacityTransferFunction &gradientOpacityTransferFunction)
    : m_editor(editor), m_oldGradientOpacityTransferFunction(editor->transferFunction().gradientOpacityTransferFunction()),
      m_newGradientOpacityTransferFunction(gradientOpacityTransferFunction)
{
    setText(tr("Set gradient opacity transfer function %1").arg(gradientOpacityTransferFunction.name()));
}

void TransferFunctionEditor::SetGradientOpacityTransferFunctionCommand::redo()
{
    m_editor->setGradientOpacityTransferFunctionCommand(m_newGradientOpacityTransferFunction);
}

void TransferFunctionEditor::SetGradientOpacityTransferFunctionCommand::undo()
{
    m_editor->setGradientOpacityTransferFunctionCommand(m_oldGradientOpacityTransferFunction);
}

TransferFunctionEditor::AddColorPointCommand::AddColorPointCommand(TransferFunctionEditor *editor, double x, const QColor &color)
    : m_editor(editor), m_x(x), m_color(color)
{
    setText(tr("Add color point at %1").arg(x));
}

void TransferFunctionEditor::AddColorPointCommand::redo()
{
    m_editor->addColorPointCommand(m_x, m_color);
}

void TransferFunctionEditor::AddColorPointCommand::undo()
{
    m_editor->removeColorPointCommand(m_x);
}

TransferFunctionEditor::RemoveColorPointCommand::RemoveColorPointCommand(TransferFunctionEditor *editor, double x)
    : m_editor(editor), m_x(x), m_color(editor->transferFunction().getColor(x))
{
    setText(tr("Remove color point at %1").arg(x));
}

void TransferFunctionEditor::RemoveColorPointCommand::redo()
{
    m_editor->removeColorPointCommand(m_x);
}

void TransferFunctionEditor::RemoveColorPointCommand::undo()
{
    m_editor->addColorPointCommand(m_x, m_color);
}

TransferFunctionEditor::AddScalarOpacityPointCommand::AddScalarOpacityPointCommand(TransferFunctionEditor *editor, double x, double opacity)
    : m_editor(editor), m_x(x), m_opacity(opacity)
{
    setText(tr("Add scalar opacity point at (%1, %2)").arg(x).arg(opacity));
}

void TransferFunctionEditor::AddScalarOpacityPointCommand::redo()
{
    m_editor->addScalarOpacityPointCommand(m_x, m_opacity);
}

void TransferFunctionEditor::AddScalarOpacityPointCommand::undo()
{
    m_editor->removeScalarOpacityPointCommand(m_x);
}

TransferFunctionEditor::RemoveScalarOpacityPointCommand::RemoveScalarOpacityPointCommand(TransferFunctionEditor *editor, double x)
    : m_editor(editor), m_x(x), m_opacity(editor->transferFunction().getScalarOpacity(x))
{
    setText(tr("Remove scalar opacity point at %1").arg(x));
}

void TransferFunctionEditor::RemoveScalarOpacityPointCommand::redo()
{
    m_editor->removeScalarOpacityPointCommand(m_x);
}

void TransferFunctionEditor::RemoveScalarOpacityPointCommand::undo()
{
    m_editor->addScalarOpacityPointCommand(m_x, m_opacity);
}

TransferFunctionEditor::AddGradientOpacityPointCommand::AddGradientOpacityPointCommand(TransferFunctionEditor *editor, double y, double opacity)
    : m_editor(editor), m_y(y), m_opacity(opacity)
{
    setText(tr("Add gradient opacity point at (%1, %2)").arg(y).arg(opacity));
}

void TransferFunctionEditor::AddGradientOpacityPointCommand::redo()
{
    m_editor->addGradientOpacityPointCommand(m_y, m_opacity);
}

void TransferFunctionEditor::AddGradientOpacityPointCommand::undo()
{
    m_editor->removeGradientOpacityPointCommand(m_y);
}

TransferFunctionEditor::RemoveGradientOpacityPointCommand::RemoveGradientOpacityPointCommand(TransferFunctionEditor *editor, double y)
    : m_editor(editor), m_y(y), m_opacity(editor->transferFunction().getGradientOpacity(y))
{
    setText(tr("Remove gradient opacity point at %1").arg(y));
}

void TransferFunctionEditor::RemoveGradientOpacityPointCommand::redo()
{
    m_editor->removeGradientOpacityPointCommand(m_y);
}

void TransferFunctionEditor::RemoveGradientOpacityPointCommand::undo()
{
    m_editor->addGradientOpacityPointCommand(m_y, m_opacity);
}

} // End namespace udg
