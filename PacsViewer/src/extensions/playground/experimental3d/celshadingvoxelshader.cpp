#include "celshadingvoxelshader.h"

#include "mathtools.h"

namespace udg {

CelShadingVoxelShader::CelShadingVoxelShader()
    : VoxelShader(), m_quantums(2)
{
}

CelShadingVoxelShader::~CelShadingVoxelShader()
{
}

void CelShadingVoxelShader::setQuantums(int quantums)
{
//    m_quantums = quantums;

    int maxQuantumSize = static_cast<int>(ceil(quantums / 2.0));
    int textureSize = maxQuantumSize * (maxQuantumSize + 1);
    if (MathTools::isOdd(quantums))
    {
        textureSize -= 1;
    }
    m_texture.resize(textureSize);

    int i = 0;
    int quantumSize = maxQuantumSize;
    while (i < textureSize)
    {
        int limit = i + quantumSize;
        float p = 0;

        if (i < textureSize / 2)
        {
            p = i / (textureSize - 1.0);
        }
        else
        {
            p = (limit - 1) / (textureSize - 1.0);
        }

        while (i < limit)
        {
            m_texture[i] = p;
            i++;
        }

        if (i > textureSize / 2)
        {
            quantumSize++;
        }
        else if (quantumSize > 1)
        {
            quantumSize--;
        }
    }

    m_quantums = textureSize;
}

QString CelShadingVoxelShader::toString() const
{
    return "CelShadingVoxelShader";
}

} // namespace udg
