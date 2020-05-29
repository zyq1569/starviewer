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

#include "obscurancevoxelshader.h"

namespace udg {

ObscuranceVoxelShader::ObscuranceVoxelShader()
 : VoxelShader(), m_data(0), m_maxValue(0), m_ambientColors(0), m_obscurance(0), m_factor(1.0), m_lowFilter(0.0), m_highFilter(1.0), m_additive(false),
 m_additiveWeight(0.0)
{
}

ObscuranceVoxelShader::~ObscuranceVoxelShader()
{
    delete[] m_ambientColors;
}

void ObscuranceVoxelShader::setData(const unsigned short *data, unsigned short maxValue)
{
    m_data = data;
    m_maxValue = maxValue;
    delete[] m_ambientColors;
    m_ambientColors = new HdrColor[m_maxValue + 1];
}

void ObscuranceVoxelShader::setTransferFunction(const TransferFunction &transferFunction)
{
    m_transferFunction = transferFunction;
    precomputeAmbientColors();
}

void ObscuranceVoxelShader::setObscurance(const Obscurance *obscurance)
{
    m_obscurance = obscurance;
}

void ObscuranceVoxelShader::setFactor(double factor)
{
    m_factor = factor;
}

void ObscuranceVoxelShader::setFilters(double low, double high)
{
    m_lowFilter = low; m_highFilter = high;
}

void ObscuranceVoxelShader::setAdditive(bool on, double weight)
{
    m_additive = on;
    m_additiveWeight = weight;
}

QString ObscuranceVoxelShader::toString() const
{
    return "ObscuranceVoxelShader";
}

void ObscuranceVoxelShader::precomputeAmbientColors()
{
    if (!m_ambientColors)
    {
        return;
    }

    unsigned int size = m_maxValue + 1;

    for (unsigned int i = 0; i < size; i++)
    {
        m_ambientColors[i] = m_transferFunction.get(i);
    }
}

}
