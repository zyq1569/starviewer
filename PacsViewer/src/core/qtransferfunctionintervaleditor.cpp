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

#include "qtransferfunctionintervaleditor.h"

#include <QColorDialog>

namespace udg {

QTransferFunctionIntervalEditor::QTransferFunctionIntervalEditor(QWidget *parent)
 : QWidget(parent), m_minimum(0), m_maximum(255)
{
    setupUi(this);

    m_isFirst = m_isLast = false;

    m_intervalEndSpinBox->setVisible(m_isIntervalCheckBox->isChecked());

    connect(m_isIntervalCheckBox, SIGNAL(toggled(bool)), SLOT(isIntervalToggled(bool)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewStart(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewEnd(int)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SIGNAL(startChanged(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SIGNAL(endChanged(int)));
    connect(m_selectColorPushButton, SIGNAL(clicked()), SLOT(selectColor()));
    connect(m_colorSpinBox, SIGNAL(colorChanged(const QColor&)), SIGNAL(colorChanged(const QColor&)));
}

QTransferFunctionIntervalEditor::QTransferFunctionIntervalEditor(int maximum, QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    m_maximum = maximum;
    m_isFirst = m_isLast = false;

    m_intervalStartSpinBox->setMaximum(m_maximum);
    m_intervalEndSpinBox->setMaximum(m_maximum);

    m_intervalEndSpinBox->setVisible(m_isIntervalCheckBox->isChecked());

    connect(m_isIntervalCheckBox, SIGNAL(toggled(bool)), SLOT(isIntervalToggled(bool)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewStart(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewEnd(int)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SIGNAL(startChanged(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SIGNAL(endChanged(int)));
    connect(m_selectColorPushButton, SIGNAL(clicked()), SLOT(selectColor()));
    connect(m_colorSpinBox, SIGNAL(colorChanged(const QColor&)), SIGNAL(colorChanged(const QColor&)));
}

QTransferFunctionIntervalEditor::~QTransferFunctionIntervalEditor()
{
}

int QTransferFunctionIntervalEditor::minimum() const
{
    return m_minimum;
}

void QTransferFunctionIntervalEditor::setMinimum(int minimum)
{
    m_minimum = minimum;
    m_intervalStartSpinBox->setMinimum(m_minimum);
    m_intervalEndSpinBox->setMinimum(m_minimum);

    if (m_isFirst)
    {
        this->setStart(m_minimum);
    }
}

int QTransferFunctionIntervalEditor::maximum() const
{
    return m_maximum;
}

void QTransferFunctionIntervalEditor::setMaximum(int maximum)
{
    m_maximum = maximum;
    m_intervalStartSpinBox->setMaximum(m_maximum);
    m_intervalEndSpinBox->setMaximum(m_maximum);

    if (m_isLast)
    {
        if (this->isInterval())
        {
            this->setEnd(m_maximum);
        }
        else
        {
            this->setStart(m_maximum);
        }
    }
}

void QTransferFunctionIntervalEditor::setIsFirst(bool isFirst)
{
    m_isFirst = isFirst;
    if (m_isFirst && m_isLast)
    {
        firstAndLast();
    }
    else
    {
        m_isIntervalCheckBox->setEnabled(true);
        if (m_isFirst)
        {
            this->setStart(m_minimum);
        }
        m_intervalStartSpinBox->setReadOnly(m_isFirst);
    }
}

bool QTransferFunctionIntervalEditor::isFirst() const
{
    return m_isFirst;
}

void QTransferFunctionIntervalEditor::setIsLast(bool isLast)
{
    m_isLast = isLast;
    if (m_isFirst && m_isLast)
    {
        firstAndLast();
    }
    else
    {
        m_isIntervalCheckBox->setEnabled(true);
        if (m_isLast)
        {
            this->setEnd(m_maximum);
        }
        m_intervalEndSpinBox->setReadOnly(m_isLast);
        if (!m_isIntervalCheckBox->isChecked())
        {
            if (m_isLast)
            {
                this->setStart(m_maximum);
            }
            m_intervalStartSpinBox->setReadOnly(m_isLast);
        }
    }
}

bool QTransferFunctionIntervalEditor::isLast() const
{
    return m_isLast;
}

bool QTransferFunctionIntervalEditor::isInterval() const
{
    return m_isIntervalCheckBox->isChecked();
}

void QTransferFunctionIntervalEditor::setStart(int start)
{
    m_intervalStartSpinBox->setValue(start);
    emit startChanged(this->start());
}

int QTransferFunctionIntervalEditor::start() const
{
    return m_intervalStartSpinBox->value();
}

void QTransferFunctionIntervalEditor::setEnd(int end)
{
    m_intervalEndSpinBox->setValue(end);
    emit endChanged(this->end());
}

int QTransferFunctionIntervalEditor::end() const
{
    return m_intervalEndSpinBox->value();
}

void QTransferFunctionIntervalEditor::setColor(QColor color)
{
    m_colorSpinBox->setColor(color);
}

const QColor& QTransferFunctionIntervalEditor::color() const
{
    return m_colorSpinBox->getColor();
}

void QTransferFunctionIntervalEditor::setIsInterval(bool isInterval)
{
    m_isIntervalCheckBox->setChecked(isInterval);
}

void QTransferFunctionIntervalEditor::setPreviousEnd(int previousEnd)
{
    if (previousEnd >= this->start())
    {
        this->setStart(previousEnd + 1);
    }
}

void QTransferFunctionIntervalEditor::setNextStart(int nextStart)
{
    if (nextStart <= this->end())
    {
        this->setEnd(nextStart - 1);
    }
}

void QTransferFunctionIntervalEditor::firstAndLast()
{
    m_isIntervalCheckBox->setChecked(true);
    m_isIntervalCheckBox->setDisabled(true);
    this->setStart(m_minimum);
    m_intervalStartSpinBox->setReadOnly(true);
    this->setEnd(m_maximum);
    m_intervalEndSpinBox->setReadOnly(true);
}

void QTransferFunctionIntervalEditor::isIntervalToggled(bool checked)
{
    if (!checked)
    {
        this->setEnd(this->start());
    }
    if (m_isLast)
    {
        if (!checked)
        {
            this->setStart(m_maximum);
        }
        m_intervalStartSpinBox->setReadOnly(!checked);
    }
}

void QTransferFunctionIntervalEditor::adjustWithNewStart(int start)
{
    if (!m_isIntervalCheckBox->isChecked() || start > this->end())
    {
        this->setEnd(start);
    }
}

void QTransferFunctionIntervalEditor::adjustWithNewEnd(int end)
{
    if (end < this->start())
    {
        this->setStart(end);
    }
}

void QTransferFunctionIntervalEditor::selectColor()
{
    QColor color = this->color();
    bool ok;
    color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &ok, this));
    if (ok)
    {
        this->setColor(color);
    }
}

}
