#ifndef UDGDIRECTILLUMINATIONVOXELSHADER2_H
#define UDGDIRECTILLUMINATIONVOXELSHADER2_H

#include "ambientvoxelshader2.h"

namespace udg {

/**
    És un voxel shader que pinta el volum amb il·luminació directa (ambient, difusa, especular).
    TODO Això és una marranada per interpolar sobre el volum original en l'optimització de funcions de transferència 2D. Cal pensar com fer-ho ben fet.
 */
class DirectIlluminationVoxelShader2 : public AmbientVoxelShader2 {

public:

    DirectIlluminationVoxelShader2();
    virtual ~DirectIlluminationVoxelShader2();

    /// Assigna la taula de normals codificades.
    void setEncodedNormals(const unsigned short *encodedNormals);
    /// Assigna les taules d'il·luminació difusa (indexades per les normals codificades).
    void setDiffuseShadingTables(const float *red, const float *green, const float *blue);
    /// Assigna les taules d'il·luminació especular (indexades per les normals codificades).
    void setSpecularShadingTables(const float *red, const float *green, const float *blue);

    /// Retorna el color corresponent al vòxel a la posició offset.
    virtual HdrColor shade(const Vector3 &position, int offset, const Vector3 &direction, float remainingOpacity, const HdrColor &baseColor = HdrColor());
    /// Retorna el color corresponent al vòxel a la posició position, fent servir valors interpolats.
    virtual HdrColor shade(const Vector3 &position, const Vector3 &direction, const TrilinearInterpolator *interpolator, float remainingOpacity,
                           const HdrColor &baseColor = HdrColor());
    /// Retorna el color corresponent al vòxel a la posició offset.
    HdrColor nvShade(const Vector3 &position, int offset, const Vector3 &direction, float remainingOpacity, const HdrColor &baseColor = HdrColor());
    /// Retorna el color corresponent al vòxel a la posició position, fent servir valors interpolats.
    HdrColor nvShade(const Vector3 &position, const Vector3 &direction, const TrilinearInterpolator *interpolator, float remainingOpacity,
                     const HdrColor &baseColor = HdrColor());
    /// Retorna un string representatiu del voxel shader.
    virtual QString toString() const;

protected:

    const unsigned short *m_encodedNormals;
    const float *m_redDiffuseShadingTable, *m_greenDiffuseShadingTable, *m_blueDiffuseShadingTable;
    const float *m_redSpecularShadingTable, *m_greenSpecularShadingTable, *m_blueSpecularShadingTable;

};

inline HdrColor DirectIlluminationVoxelShader2::shade(const Vector3 &position, int offset, const Vector3 &direction, float remainingOpacity,
                                                      const HdrColor &baseColor)
{
    return nvShade(position, offset, direction, remainingOpacity, baseColor);
}

inline HdrColor DirectIlluminationVoxelShader2::shade(const Vector3 &position, const Vector3 &direction, const TrilinearInterpolator *interpolator,
                                                      float remainingOpacity, const HdrColor &baseColor)
{
    return nvShade(position, direction, interpolator, remainingOpacity, baseColor);
}

inline HdrColor DirectIlluminationVoxelShader2::nvShade(const Vector3 &position, int offset, const Vector3 &direction, float remainingOpacity,
                                                        const HdrColor &baseColor)
{
    Q_ASSERT(m_data);
    Q_ASSERT(m_encodedNormals);
    Q_ASSERT(m_redDiffuseShadingTable); Q_ASSERT(m_greenDiffuseShadingTable); Q_ASSERT(m_blueDiffuseShadingTable);
    Q_ASSERT(m_redSpecularShadingTable); Q_ASSERT(m_greenSpecularShadingTable); Q_ASSERT(m_blueSpecularShadingTable);

    HdrColor color = AmbientVoxelShader2::nvShade(position, offset, direction, remainingOpacity, baseColor);

    if (color.isTransparent())
    {
        return color;
    }

    unsigned short normal = m_encodedNormals[offset];
    color.red = color.red * m_redDiffuseShadingTable[normal] + m_redSpecularShadingTable[normal];
    color.green = color.green * m_greenDiffuseShadingTable[normal] + m_greenSpecularShadingTable[normal];
    color.blue = color.blue * m_blueDiffuseShadingTable[normal] + m_blueSpecularShadingTable[normal];
    return color;
}

inline HdrColor DirectIlluminationVoxelShader2::nvShade(const Vector3 &position, const Vector3 &direction, const TrilinearInterpolator *interpolator,
                                                        float remainingOpacity, const HdrColor &baseColor)
{
    Q_UNUSED(direction);
    Q_UNUSED(remainingOpacity);
    Q_UNUSED(baseColor);

    Q_ASSERT(interpolator);
    Q_ASSERT(m_data);
    Q_ASSERT(m_encodedNormals);
    Q_ASSERT(m_redDiffuseShadingTable); Q_ASSERT(m_greenDiffuseShadingTable); Q_ASSERT(m_blueDiffuseShadingTable);
    Q_ASSERT(m_redSpecularShadingTable); Q_ASSERT(m_greenSpecularShadingTable); Q_ASSERT(m_blueSpecularShadingTable);

    int offsets[8];
    double weights[8];
    interpolator->getOffsetsAndWeights(position, offsets, weights);

    double value;
    if (m_alternativeData)
    {
        double intensity = TrilinearInterpolator::interpolate<double>(m_alternativeData, offsets, weights);
        double gradient = TrilinearInterpolator::interpolate<double>(m_gradientMagnitudes, offsets, weights);
        value = m_extension->cluster2DIndex(intensity, gradient);
    }
    else
    {
        value = TrilinearInterpolator::interpolate<double>(m_data, offsets, weights);
    }

    HdrColor color = m_ambientColors[static_cast<int>(value)];

    if (color.isTransparent())
    {
        return color;
    }

    int normals[8];
    for (int i = 0; i < 8; i++)
    {
        normals[i] = m_encodedNormals[offsets[i]];
    }

    double diffuseRed = TrilinearInterpolator::interpolate<double>(m_redDiffuseShadingTable, normals, weights);
    double diffuseGreen = TrilinearInterpolator::interpolate<double>(m_greenDiffuseShadingTable, normals, weights);
    double diffuseBlue = TrilinearInterpolator::interpolate<double>(m_blueDiffuseShadingTable, normals, weights);
    double specularRed = TrilinearInterpolator::interpolate<double>(m_redSpecularShadingTable, normals, weights);
    double specularGreen = TrilinearInterpolator::interpolate<double>(m_greenSpecularShadingTable, normals, weights);
    double specularBlue = TrilinearInterpolator::interpolate<double>(m_blueSpecularShadingTable, normals, weights);
    color.red = color.red * diffuseRed + specularRed;
    color.green = color.green * diffuseGreen + specularGreen;
    color.blue = color.blue * diffuseBlue + specularBlue;
    return color;
}

} // namespace udg

#endif // UDGDIRECTILLUMINATIONVOXELSHADER2_H
