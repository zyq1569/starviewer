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

#include "genericvolumedisplayunithandler.h"

#include "mathtools.h"
#include "logging.h"
#include "volumedisplayunit.h"
#include "imagepipeline.h"
#include "transferfunctionmodel.h"
#include "secondaryvolumedisplayunit.h"
#include "volume.h"

#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageStack.h>

namespace udg {

GenericVolumeDisplayUnitHandler::GenericVolumeDisplayUnitHandler()
{
    m_transferFunctionModel = new TransferFunctionModel();
    m_imageStack = vtkImageStack::New();
}

GenericVolumeDisplayUnitHandler::~GenericVolumeDisplayUnitHandler()
{
    removeDisplayUnits();
    delete m_transferFunctionModel;
    m_imageStack->Delete();
}

void GenericVolumeDisplayUnitHandler::setInput(Volume *input)
{
    removeDisplayUnits();
    addDisplayUnit(input);
    setupDisplayUnits();
}

void GenericVolumeDisplayUnitHandler::setInputs(QList<Volume*> inputs)
{
    removeDisplayUnits();

    int supportedNumberOfInputs = qBound(0, inputs.size(), getMaximumNumberOfInputs());
    
    if (supportedNumberOfInputs < inputs.size())
    {
        DEBUG_LOG(QString("The number of the given inputs (%1) exceeds the maximum of the handler (%2). There will be only %2 handled.")
            .arg(inputs.size()).arg(getMaximumNumberOfInputs()));
    }
    
    for (int i = 0; i < supportedNumberOfInputs; ++i)
    {
        addDisplayUnit(inputs.at(i));
    }
    setupDisplayUnits();
}

bool GenericVolumeDisplayUnitHandler::hasInput() const
{
    return getNumberOfInputs() > 0;
}

int GenericVolumeDisplayUnitHandler::getNumberOfInputs() const
{
    return m_displayUnits.count();
}

VolumeDisplayUnit* GenericVolumeDisplayUnitHandler::getMainVolumeDisplayUnit() const
{
    return getVolumeDisplayUnit(0);
}

VolumeDisplayUnit* GenericVolumeDisplayUnitHandler::getVolumeDisplayUnit(int i) const
{
    if (MathTools::isInsideRange(i, 0, m_displayUnits.count() - 1))
    {
        return m_displayUnits.at(i);
    }
    else
    {
        return 0;
    }
}

QList<VolumeDisplayUnit*> GenericVolumeDisplayUnitHandler::getVolumeDisplayUnitList() const
{
    return m_displayUnits;
}

int GenericVolumeDisplayUnitHandler::getMaximumNumberOfInputs() const
{
    return std::numeric_limits<int>::max();
}

vtkImageSlice* GenericVolumeDisplayUnitHandler::getImageProp() const
{
    return m_imageStack;
}

TransferFunctionModel* GenericVolumeDisplayUnitHandler::getTransferFunctionModel() const
{
    return m_transferFunctionModel;
}

void GenericVolumeDisplayUnitHandler::removeDisplayUnits()
{
    foreach (VolumeDisplayUnit *unit, m_displayUnits)
    {
        m_imageStack->RemoveImage(unit->getImageSlice());
        delete unit;
    }
    m_displayUnits.clear();
}

void GenericVolumeDisplayUnitHandler::addDisplayUnit(Volume *input)
{
    if (!input)
    {
        return;
    }

    const int LargeDimension = 1000;
    int *dimensions = input->getDimensions();
    VolumeDisplayUnit *displayUnit;

    // Use VolumeDisplayUnit for the primary volume if it has large dimensions, and SecondaryVolumeDisplayUnit otherwise
    if (m_displayUnits.isEmpty() && (dimensions[0] > LargeDimension || dimensions[1] > LargeDimension || dimensions[2] > LargeDimension))
    {
        displayUnit = new VolumeDisplayUnit();
    }
    else
    {
        displayUnit = new SecondaryVolumeDisplayUnit();
    }

    // Add the vdu to the list before setting the volume to avoid a memory leak if setVolume throws a bad_alloc
    m_displayUnits << displayUnit;
    displayUnit->setVolume(input);
    m_imageStack->AddImage(displayUnit->getImageSlice());
}

void GenericVolumeDisplayUnitHandler::setupDisplayUnits()
{
    updateMainDisplayUnitIndex();
    updateLayerNumbers();
    setupDefaultOpacities();
    initializeTransferFunctions();
}

void GenericVolumeDisplayUnitHandler::updateLayerNumbers()
{
    for (int i = 0; i < m_displayUnits.size(); i++)
    {
        m_displayUnits[i]->getImageSlice()->GetProperty()->SetLayerNumber(i);
    }
}

void GenericVolumeDisplayUnitHandler::setupDefaultOpacities()
{
    // The default opacities will be 1 for the main volume and 0.5 for the others
    for (int i = 0; i < getNumberOfInputs(); i++)
    {
        vtkImageSlice *imageSlice = m_displayUnits.at(i)->getImageSlice();
        imageSlice->GetProperty()->SetOpacity(1.0);
    }
}

void GenericVolumeDisplayUnitHandler::initializeTransferFunctions()
{
    // By default, all volumes have no transfer function
    foreach (VolumeDisplayUnit *unit, m_displayUnits)
    {
        unit->clearTransferFunction();
    }

    setupDefaultTransferFunctions();
}

} // End namespace udg
