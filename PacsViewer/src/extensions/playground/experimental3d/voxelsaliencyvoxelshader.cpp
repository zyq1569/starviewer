#include "voxelsaliencyvoxelshader.h"

namespace udg {

VoxelSaliencyVoxelShader::VoxelSaliencyVoxelShader()
    : VoxelShader(), m_data(0), m_maxValue(0), m_opacities(0), m_maximumSaliency(1.0f), m_saliencyFactor(1.0f)
{
}

VoxelSaliencyVoxelShader::~VoxelSaliencyVoxelShader()
{
    delete[] m_opacities;
}

void VoxelSaliencyVoxelShader::setData(const unsigned short *data, unsigned short maxValue)
{
    m_data = data;
    m_maxValue = maxValue;
    delete[] m_opacities;
    m_opacities = new float[m_maxValue + 1];
}

void VoxelSaliencyVoxelShader::setTransferFunction(const TransferFunction &transferFunction)
{
    m_transferFunction = transferFunction;
    precomputeOpacities();
}

void VoxelSaliencyVoxelShader::setVoxelSaliencies(const QVector<float> &voxelSaliencies, float maximumSaliency, float saliencyFactor)
{
    m_voxelSaliencies = voxelSaliencies;
    m_maximumSaliency = maximumSaliency;
    m_saliencyFactor = saliencyFactor;
}

QString VoxelSaliencyVoxelShader::toString() const
{
    return "VoxelSaliencyVoxelShader";
}

void VoxelSaliencyVoxelShader::precomputeOpacities()
{
    if (!m_opacities)
    {
        return;
    }

    unsigned int size = m_maxValue + 1;
    for (unsigned int i = 0; i < size; i++)
    {
        m_opacities[i] = m_transferFunction.getOpacity(i);
    }
}

} // namespace udg
