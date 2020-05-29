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

#include "qtransferfunctioneditor2.h"

#include "qtransferfunctiongraphicalview.h"
#include "transferfunctioneditor.h"

namespace udg {

QTransferFunctionEditor2::QTransferFunctionEditor2(QWidget *parent)
 : QWidget(parent)
{
    init(Everything, Graphical);
}

QTransferFunctionEditor2::QTransferFunctionEditor2(Features features, View view, QWidget *parent)
 : QWidget(parent)
{
    init(features, view);
}

const TransferFunction& QTransferFunctionEditor2::transferFunction() const
{
    return m_editor->transferFunction();
}

void QTransferFunctionEditor2::setTransferFunction(const TransferFunction &transferFunction)
{
    m_editor->setTransferFunction(transferFunction);
}

void QTransferFunctionEditor2::init(Features features, View view)
{
    setupUi(this);

    // Els farem servir en el futur, quan ja estiguin implementades les coses bàsiques
    Q_UNUSED(features);
    Q_UNUSED(view);

    m_editor = new TransferFunctionEditor(this);
    m_view = new QTransferFunctionGraphicalView(m_editor, this);
    m_viewLayout->addWidget(m_view);
    m_undoView->setStack(m_editor->undoStack());

    makeConnections();
}

void QTransferFunctionEditor2::makeConnections()
{
    connect(m_editor, SIGNAL(transferFunctionChanged(TransferFunction)), SLOT(emitTransferFunctionChanged()));
    connect(m_editor, SIGNAL(nameChanged(QString)), SLOT(emitTransferFunctionChanged()));
    // Afegir connexions per tots els signals de l'editor
}

void QTransferFunctionEditor2::emitTransferFunctionChanged()
{
    emit transferFunctionChanged(m_editor->transferFunction());
}

} // End namespace udg
